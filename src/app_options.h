#ifndef APP_OPTIONS_H
#define APP_OPTIONS_H

#include "peer_definition.h"
#include "tls_connection.h"

#include <cx2_net_interfaces/virtualnetworkinterface.h>
#include <cx2_thr_safecontainers/map.h>


#include <string>
#include <map>
#include <stdint.h>

struct sAppOptions
{
    bool ipv4,listenMode, notls;
    std::string cafile,keyfile,certfile;

    std::string ifaceName;
    std::string addr;
    uint16_t port, threadsLimit, uid,gid;
    CX2::Application::Logs::AppLog * log;

    CX2::Network::Interfaces::VirtualNetworkInterface tapIface;

    sPeerDefinition localTapOptions;
    uint64_t tapHwAddrHash;

    // Map IP->Peer Definition (IP is the authentication User)
    std::map<uint32_t,sPeerDefinition> peersDefinition;

    // IPADDR -> Peer connection...
    CX2::Threads::Safe::Map<uint32_t> connectedPeers;
};


#endif // APP_OPTIONS_H
