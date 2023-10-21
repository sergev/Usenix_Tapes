/* 
 * SAR: Simple Address Resolution Protocol Implementation
 * Written by Geoffrey Cooper, September 27, 1983
 * 
 * This package implements a very simple version of the Plummer Address
 * Resolution Protocol (RFC 826).  It allows clients to resolve Internet
 * addresses into Ethernet addresses, and knows how to respond to an
 * address resolution request (when the transmit buffer is free).
 * 
 * Routines:
 * 
 *  sar_CheckPacket( pb ) => 1, if ARP packet and processed, 0 otherwise
 *  sar_MapIn2Eth( ina, ethap ) => 1 if did it, 0 if couldn't.
 *
 * Copyright (C) 1983, 1986 IMAGEN Corporation
 *  "This code may be duplicated in whole or in part provided that [1] there
 *   is no commercial gain involved in the duplication, and [2] that this
 *   copyright notice is preserved on all copies.  Any other duplication
 *   requires written notice of the author (Geoffrey H. Cooper)."
 * 
 */
#include "tinytcp.h"

sar_CheckPacket(ap)
    register arp_Header *ap;
{
    register arp_Header *op;

    if ( ap->hwType != arp_TypeEther || /* have ethernet hardware, */
         ap->protType != 0x800 ||       /* and internet software, */
         ap->opcode != ARP_REQUEST ||   /* and be a resolution req. */
         ap->dstIPAddr != sin_lclINAddr /* for my addr. */
       ) return ( 0 );                  /* .... or we ignore it. */

    /* format response. */
    op = (arp_Header *)sed_FormatPacket(ap->srcEthAddr, 0x806);
    op->hwType = arp_TypeEther;
    op->protType = 0x800;
    op->hwProtAddrLen = (sizeof(eth_HwAddress) << 8) + sizeof(in_HwAddress);
    op->opcode = ARP_REPLY;
    op->srcIPAddr = sin_lclINAddr;
    MoveW(sed_lclEthAddr, op->srcEthAddr, sizeof(eth_HwAddress));
    ap->dstIPAddr = op->srcIPAddr;
    MoveW(ap->srcEthAddr, op->dstEthAddr, sizeof(eth_HwAddress));

    sed_Send(sizeof(arp_Header));
    
    return ( 1 );
}

/* 
 * Do an address resolution bit.
 */
sar_MapIn2Eth(ina, ethap)
    longword ina;
    eth_HwAddress *ethap;
{
    register arp_Header *op;
    extern in_HwAddress sin_lclINAddr;
    register i;
    longword endTime;
    longword rxMitTime;

    sed_Receive( 0 );
    endTime = clock_ValueRough() + 2000;
    while ( endTime > clock_ValueRough() ) {
        op = (arp_Header *)sed_FormatPacket(&sed_ethBcastAddr[0], 0x806);
        op->hwType = arp_TypeEther;
        op->protType = 0x800;
        op->hwProtAddrLen = (sizeof(eth_HwAddress) << 8) + sizeof(in_HwAddress);
        op->opcode = ARP_REQUEST;
        op->srcIPAddr = sin_lclINAddr;
        MoveW(sed_lclEthAddr, op->srcEthAddr, sizeof(eth_HwAddress));
        op->dstIPAddr = ina;

        /* ...and send the packet */
        sed_Send( sizeof(arp_Header) );

        rxMitTime = clock_ValueRough() + 250;
        while ( rxMitTime > clock_ValueRough() ) {
            op = (arp_Header *)sed_IsPacket();
            if ( op ) {
                if ( sed_CheckPacket(op, 0x806) == 1 &&
                    op->protType == 0x800 &&
                    op->srcIPAddr == ina &&
                    op->opcode == ARP_REPLY ) {
                    MoveW(op->srcEthAddr, ethap, sizeof(eth_HwAddress));
                    return ( 1 );
                }
                sed_Receive(op);
            }
        }
    }
    return ( 0 );
}
