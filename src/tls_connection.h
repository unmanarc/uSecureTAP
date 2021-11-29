#ifndef TLS_CONNECTION_H
#define TLS_CONNECTION_H

#include <cx2_thr_safecontainers/map_element.h>
#include <cx2_net_sockets/socket_tls.h>
#include "peer_definition.h"

class TLS_Connection : public CX2::Threads::Safe::Map_Element
{
public:
    TLS_Connection(CX2::Network::Streams::StreamSocket * sock, const sPeerDefinition & peerDefinition) {
        this->sock = sock;
        this->peerDefinition = peerDefinition;
    }

    CX2::Network::Streams::StreamSocket * sock;
    sPeerDefinition peerDefinition;
};


#endif // TLS_CONNECTION_H
