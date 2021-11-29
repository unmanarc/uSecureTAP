#include "pkt_dissector.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

bool dissectNetworkPacket(char *data, uint32_t datalen, sDissectedPacketHeaders *headers)
{
    headers->fullPacketLen = datalen;
    if (!assignDataStruct(&data,(void **)&(headers->ethh),sizeof(ethhdr),&datalen))
    {
        return false;
    }

    // ONLY PROCESS IPv4 PACKETS...
    if (ntohs(headers->ethh->h_proto) == ETH_P_IP)
    {
        headers->ipPacketLen = datalen;
        if (!assignDataStruct(&data,(void **)&(headers->iph),sizeof(iphdr),&datalen))
        {
            return false;
        }

        // Check IP Header lenght..
        headers->ipHeaderLen = (uint32_t)((headers->iph->ihl)*4);

        headers->isIPv4 = true;
        return true;
    }

    if (ntohs(headers->ethh->h_proto) == ETH_P_ARP)
    {
        headers->arpPacketLen = datalen;
        if (!assignDataStruct(&data,(void **)&(headers->arph),sizeof(arphdr2),&datalen))
        {
            return false;
        }

        // Check IP Header lenght..
        headers->arpHeaderLen = sizeof(arphdr2);
        headers->isARP = true;
        return true;
    }


    return false;
}



bool assignDataStruct(char **data, void **structure, const uint32_t &structureSize, uint32_t *len)
{
    if (!structureSize)
        return true;

    if (structureSize>*len)
        return false;

    *structure = *data;
    *data = *data + structureSize;
    (*len)-=structureSize;

    return true;
}
