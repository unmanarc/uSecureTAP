#include "tls_callbacks.h"
#include "app_options.h"

#include "pkt_dissector.h"
#include "tls_connection.h"

#include <mdz_hlp_functions/appexec.h>

#include <mdz_hlp_functions/random.h>
#include <mdz_net_sockets/streams_cryptochallenge.h>

#include <mdz_mem_vars/a_ipv4.h>

using namespace Mantids;
using namespace Mantids::Memory;
using namespace Mantids::Application;
using namespace Mantids::Network::Sockets;

TLS_Callbacks::TLS_Callbacks()
{

}

std::string genCombinedKey(sAppOptions * appOptions, const sPeerDefinition & peerDefinition)
{
    // Establish the shared key as: (server key) + (client key)

    if ( appOptions->listenMode )
        return appOptions->localTapOptions.key + peerDefinition.key;
    else
        return peerDefinition.key + appOptions->localTapOptions.key;
}

bool TLS_Callbacks::onConnect(void *obj, Network::Sockets::Socket_StreamBase *sock, const char * remoteAddr, bool)
{
    bool ok;
    sAppOptions * appOptions = (sAppOptions *)obj;
    std::string tlsCN = appOptions->notls? "" : ((Network::Sockets::Socket_TLS *)sock)->getTLSPeerCN();
    appOptions->log->log0(__func__,Logs::LEVEL_INFO, "Connection established to HOST='%s', IP='%s', CN='%s'",remoteAddr, sock->getRemotePairStr().c_str(), tlsCN.c_str());

    // Exchange remote VPN IP
    sock->writeU<uint32_t>(appOptions->localTapOptions.aIpAddr.s_addr);

    uint32_t uRemoteVPNIP = sock->readU<uint32_t>(&ok);
    if (!ok) return false;

    sPeerDefinition peerDefinition;
    peerDefinition.key = Mantids::Helpers::Random::createRandomString(128);

    // Establish a remote expected host key only when exist (otherwise will be a random value)
    // So, if the host/client is not defined, should be "impossible" to guess the password...
    if ( appOptions->peersDefinition.find(uRemoteVPNIP) != appOptions->peersDefinition.end())
        peerDefinition = appOptions->peersDefinition[uRemoteVPNIP];

    // Exchange keys (mutual auth):
    Mantids::Network::Sockets::NetStreams::CryptoChallenge cstreams(sock);
    if (cstreams.mutualChallengeResponseSHA256Auth( genCombinedKey(appOptions,peerDefinition) ,appOptions->listenMode) == std::make_pair(true,true))
    {
        // Peer not found:
        if ( appOptions->peersDefinition.find(uRemoteVPNIP) == appOptions->peersDefinition.end())
        {
            // Even if the client guess the random password from unexistant host, fail here:
            appOptions->log->log1(__func__,Abstract::IPV4::_toString(uRemoteVPNIP), Logs::LEVEL_ERR, "Weird error, peer not found from '%s', CN='%s'", remoteAddr, tlsCN.c_str());
            return false;
        }
        char data[65536];
        uint16_t datalen=65535;

        appOptions->log->log1(__func__,Abstract::IPV4::_toString(uRemoteVPNIP),Logs::LEVEL_INFO, "Connected and Authenticated to '%s', CN='%s'", remoteAddr, tlsCN.c_str());

        // Call the UP Script after the authentication...
        if (!appOptions->upScript.empty())
        {
            appOptions->log->log1(__func__,Abstract::IPV4::_toString(uRemoteVPNIP),Logs::LEVEL_INFO, "Executing '%s'", appOptions->upScript.c_str() );
            Mantids::Helpers::AppSpawn appUp;
            appUp.setExec(appOptions->upScript);
            appUp.addEnvironment("REMOTEIP=" + Abstract::IPV4::_toString(uRemoteVPNIP));
            appUp.spawnProcess();
            appUp.waitUntilProcessEnds();
        }

        appOptions->connectedPeers.addElement( peerDefinition.macAddrHash, new TLS_Connection(sock,peerDefinition) );
        for (;sock->readBlockEx<uint16_t>(data,&datalen);)
        {
            // PING:
            if (!datalen)
                continue;

            sDissectedPacketHeaders headers;
            if (!dissectNetworkPacket(data,datalen,&headers))
            {
                appOptions->log->log1(__func__,Abstract::IPV4::_toString(uRemoteVPNIP),Logs::LEVEL_ERR, "Malformed data packet. (VPN_IP=%s)");
            }

            if (headers.isIPv4)
            {
                // Rewrite MAC Address using the peer definition...
                Abstract::MACADDR::_fromHASH( peerDefinition.macAddrHash, headers.ethh->h_source );
                // Everthing is for me...
                Abstract::MACADDR::_fromHASH( appOptions->tapHwAddrHash, headers.ethh->h_dest );

                // Write to tap interface...
                ssize_t x;
                if ((x=appOptions->tapIface.writePacket(headers.ethh, headers.fullPacketLen))!=headers.fullPacketLen)
                    appOptions->log->log1(__func__, Abstract::IPV4::_toString(uRemoteVPNIP),Logs::LEVEL_WARN, "Unable to write packet to TAP interface. (packet size: %d)", x);
            }
        }

        // De-register connection...
        appOptions->connectedPeers.destroyElement( peerDefinition.macAddrHash );
        appOptions->log->log1(__func__,Abstract::IPV4::_toString(uRemoteVPNIP),Logs::LEVEL_WARN, "Disconnected from %s (%s)", remoteAddr, sock->getLastError().c_str());
        if (sock->isSecure())
        {
            for ( auto & i : ((Mantids::Network::Sockets::Socket_TLS *)sock)->getTLSErrorsAndClear() )
            {
                appOptions->log->log1(__func__,Abstract::IPV4::_toString(uRemoteVPNIP),Logs::LEVEL_WARN, "Disconnected from %s (TLS ERROR: %s)", remoteAddr, i.c_str());
            }
        }

        // Call the DOWN Script after the disconnection...
        if (!appOptions->downScript.empty())
        {
            appOptions->log->log1(__func__,Abstract::IPV4::_toString(uRemoteVPNIP),Logs::LEVEL_INFO, "Executing '%s'", appOptions->downScript.c_str() );
            Mantids::Helpers::AppSpawn appDown;
            appDown.setExec(appOptions->downScript);
            appDown.addEnvironment("REMOTEIP=" + Abstract::IPV4::_toString(uRemoteVPNIP));
            appDown.spawnProcess();
            appDown.waitUntilProcessEnds();
        }

        return true;

    }
    else
    {
        appOptions->log->log0(__func__,Logs::LEVEL_ERR, "Disconnected, Invalid Key exchange with HOST='%s', CN='%s'", remoteAddr, tlsCN.c_str());
        return false;
    }

}

bool TLS_Callbacks::onInitFailed(void *, Network::Sockets::Socket_StreamBase *s, const char *, bool)
{
    return true;
}

void TLS_Callbacks::onTimeOut(void *, Network::Sockets::Socket_StreamBase *s, const char *, bool)
{
    /*   s->writeString("HTTP/1.1 503 Service Temporarily Unavailable\r\n");
    s->writeString("Content-Type: text/html; charset=UTF-8\r\n");
    s->writeString("Connection: close\r\n");
    s->writeString("\r\n");
    s->writeString("<center><h1>503 Service Temporarily Unavailable</h1></center><hr>\r\n");*/
}
