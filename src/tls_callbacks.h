#ifndef TLS_CALLBACKS_H
#define TLS_CALLBACKS_H

#include <cx2_net_sockets/socket_tls.h>

class TLS_Callbacks
{
public:

    TLS_Callbacks();

    /**
     * callback when connection is fully established (if the callback returns false, connection socket won't be automatically closed/deleted)
     */
    static bool onConnect(void *, CX2::Network::Streams::StreamSocket *sock, const char *, bool);
    /**
     * callback when protocol initialization failed (like bad X.509 on TLS) (if the callback returns false, connection socket won't be automatically closed/deleted)
     */
    static bool onInitFailed(void *, CX2::Network::Streams::StreamSocket *, const char *, bool);
    /**
     * callback when timed out (all the thread queues are saturated) (this callback is called from acceptor thread, you should use it very quick)
     */
    static void onTimeOut(void *, CX2::Network::Streams::StreamSocket *, const char *, bool);


};

#endif // TLS_CALLBACKS_H
