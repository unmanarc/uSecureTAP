#include "tls_ping.h"


void tlsPeersPingThread(sAppOptions *appOptions)
{

    if (!(appOptions->pingEvery))
    {
        sleep(1);
        return;
    }


    for (;;)
    {
        auto keys = appOptions->connectedPeers.getKeys();
        for (const auto & peerKey : keys)
        {
            TLS_Connection * connection = (TLS_Connection *)appOptions->connectedPeers.openElement( peerKey );
            if (connection)
            {
                connection->writeLock.lock();
                // pinger...
                connection->sock->writeBlockEx<uint16_t>("",0);
                connection->writeLock.unlock();

                appOptions->connectedPeers.closeElement(peerKey);
            }
        }

        sleep(appOptions->pingEvery);
    }

}
