#include "virtiface_reader.h"
#include "tls_connection.h"
#include "pkt_dissector.h"
#include <cx2_mem_vars/a_macaddr.h>

using namespace CX2;
using namespace CX2::Memory;
using namespace CX2::Application;

void virtIfaceReader(sAppOptions *appOptions)
{
    ssize_t len;
    char packet[65536];
    memset(packet,0,sizeof (packet));
    while (  (len = appOptions->tapIface.readPacket(packet, 65535)) >0 )
    {
        //
        sDissectedPacketHeaders headers;
        if (!dissectNetworkPacket(packet,len,&headers))
        {
            appOptions->log->log0(__func__,Logs::LEVEL_DEBUG, "Non-compatible packet from TAP. (NON-IPv4 Packet)");
            // Non compatible packet
        }
        else
        {
            if (headers.isIPv4)
            {
                // Write the packet from TAP into the TLS connection
                auto hEthDst = Abstract::MACADDR::_toHash( headers.ethh->h_dest);
                TLS_Connection * connection = (TLS_Connection *)appOptions->connectedPeers.openElement( hEthDst );
                if (connection)
                {
                    connection->writeLock.lock();
                    connection->sock->writeBlockEx<uint16_t>(packet,len);
                    connection->writeLock.unlock();

                    appOptions->connectedPeers.closeElement( hEthDst );
                }
                else
                    appOptions->log->log0(__func__,Logs::LEVEL_DEBUG, "IPv4 Packet: Endpoint %s not connected...", Abstract::MACADDR::_fromHASHToString(hEthDst).c_str());
            }
            else if(headers.isARP)
            {
                if (        ntohs(headers.arph->ar_hrd) == 0x01 // EThernet
                        &&  ntohs(headers.arph->ar_pro) == 0x0800 // IPv4
                        &&  headers.arph->ar_hln == 6 // ETHERNET ADDR LEN
                        &&  headers.arph->ar_pln == 4 // IPV4 ADDR LEN
                        &&  ntohs(headers.arph->ar_op)  == 1 // ARP REQUEST
                        )
                {

                    // Where is the IP
                    uint32_t requestedIP = *((uint32_t *)headers.arph->ar_tip);
                    if (appOptions->peersDefinition.find(requestedIP) != appOptions->peersDefinition.end())
                    {
                        auto hEthRequested = appOptions->peersDefinition[requestedIP].macAddrHash;
                        TLS_Connection * connection = (TLS_Connection *)appOptions->connectedPeers.openElement( hEthRequested );
                        if (connection)
                        {
                            appOptions->connectedPeers.closeElement( hEthRequested );

                            // Prepare the ARP response:
                            char packetcpy[65536];
                            memset(packetcpy,0,sizeof (packetcpy));
                            memcpy(packetcpy,packet,len);
                            sDissectedPacketHeaders headerscpy;
                            if (dissectNetworkPacket(packetcpy,len,&headerscpy))
                            {
                                // Make the response here.
                                std::swap(headerscpy.ethh->h_dest,headerscpy.ethh->h_source);
                                headerscpy.arph->ar_op = htons(2); // ARP RESPONSE.
                                std::swap(headerscpy.arph->ar_sha,headerscpy.arph->ar_tha);  /* sender hardware address      */ /* target hardware address      */
                                std::swap(headerscpy.arph->ar_sip,headerscpy.arph->ar_tip); /* sender IP address            */  /* target IP address            */

                                // Define the sender hardware address using the MAC Address from the configuration..
                                Abstract::MACADDR::_fromHASH(hEthRequested,headerscpy.arph->ar_sha);
                                Abstract::MACADDR::_fromHASH(hEthRequested,headerscpy.ethh->h_source);

                                // Now send the response back to the virtual interface:
                                appOptions->tapIface.writePacket(packetcpy,len);
                            }
                        }
                        else
                            appOptions->log->log0(__func__,Logs::LEVEL_DEBUG, "ARP Request: Endpoint %s not connected...", Abstract::MACADDR::_fromHASHToString(hEthRequested).c_str());
                    }
                    else
                        appOptions->log->log0(__func__,Logs::LEVEL_WARN, "ARP Request: IP Address %s is not defined...", Abstract::IPV4::_toString(requestedIP).c_str());
                }
            }
        }
    }
}
