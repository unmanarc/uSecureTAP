#ifndef PKT_DISSECTOR_H
#define PKT_DISSECTOR_H

#ifndef _WIN32
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <netinet/in.h>
#else
#include <cx2_net_interfaces/netheaders-windows.h>
#endif

#include <stdint.h>


// From if_arp.h
struct arphdr2 {
    uint16_t      ar_hrd;         /* format of hardware address   */
    uint16_t      ar_pro;         /* format of protocol address   */
    unsigned char ar_hln;         /* length of hardware address   */
    unsigned char ar_pln;         /* length of protocol address   */
    uint16_t      ar_op;          /* ARP opcode (command)         */

    uint8_t       ar_sha[ETH_ALEN];       /* sender hardware address      */
    uint8_t       ar_sip[4];              /* sender IP address            */
    uint8_t       ar_tha[ETH_ALEN];       /* target hardware address      */
    uint8_t       ar_tip[4];              /* target IP address            */
};

struct sDissectedPacketHeaders {
    sDissectedPacketHeaders()
    {
        isARP=false;
        isIPv4=false;
        ethh=nullptr;
        iph=nullptr;
        arph=nullptr;

        ipPacketLen = 0;
        ipHeaderLen = 0;
        fullPacketLen = 0;
    }

    bool isIPv4, isARP;

    ethhdr * ethh;
    iphdr * iph;
    arphdr2 * arph;

    uint32_t fullPacketLen;
    uint32_t ipPacketLen, arpPacketLen;
    uint32_t ipHeaderLen, arpHeaderLen;
};

bool assignDataStruct(char ** data, void ** structure, const uint32_t & structureSize, uint32_t * datalen);
bool dissectNetworkPacket(char * data, uint32_t datalen, sDissectedPacketHeaders * headers);

#endif // PKT_DISSECTOR_H
