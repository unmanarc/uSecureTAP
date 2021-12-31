#ifndef APP_OPTIONS_H
#define APP_OPTIONS_H

#include "peer_definition.h"
#include "tls_connection.h"

#include <mdz_net_interfaces/virtualnetworkinterface.h>
#include <mdz_thr_safecontainers/map.h>


#include <string>
#include <map>
#include <stdint.h>

struct sAppOptions
{
    bool ipv4,listenMode, notls;
    std::string cafile,keyfile,certfile,ifaceName,addr,upScript,downScript;
    uint16_t port, threadsLimit, uid,gid;
    uint32_t pingEvery;

    // LOG:
    Mantids::Application::Logs::AppLog * log;

    // TAP:
    Mantids::Network::Interfaces::VirtualNetworkInterface tapIface;
    sPeerDefinition localTapOptions;
    uint64_t tapHwAddrHash;

    // Map IP->Peer Definition (IP is the authentication User)
    std::map<uint32_t,sPeerDefinition> peersDefinition;

    // IPADDR -> Peer connection...
    Mantids::Threads::Safe::Map<uint32_t> connectedPeers;
};


#endif // APP_OPTIONS_H
