#ifndef TLS_CONNECTION_H
#define TLS_CONNECTION_H

#include <mdz_thr_safecontainers/map_element.h>
#include <mdz_net_sockets/socket_tls.h>
#include "peer_definition.h"

#include <mutex>

class TLS_Connection : public Mantids::Threads::Safe::Map_Element
{
public:
    TLS_Connection(Mantids::Network::Streams::StreamSocket * sock, const sPeerDefinition & peerDefinition) {
        this->sock = sock;
        this->peerDefinition = peerDefinition;
    }

    Mantids::Network::Streams::StreamSocket * sock;
    sPeerDefinition peerDefinition;
    std::mutex writeLock;
};


#endif // TLS_CONNECTION_H
