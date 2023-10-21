/* 
 * Ethernet Driver.
 * A Very Simple set of ethernet driver primitives.  The ethernet (3com Mbus)
 * interface is controlled by busy-waiting, the application is handed the
 * location of on-board packet buffers, and allowed to fill in the
 * transmit buffer directly.  The interface is entirely blocking.
 * 
 * Written March, 1986 by Geoffrey Cooper
 *
 * Copyright (C) 1986, IMAGEN Corporation
 *  "This code may be duplicated in whole or in part provided that [1] there
 *   is no commercial gain involved in the duplication, and [2] that this
 *   copyright notice is preserved on all copies.  Any other duplication
 *   requires written notice of the author (Geoffrey H. Cooper)."
 * 
 * Primitives:
 *  sed_Init()  -- Initialize the package
 *  sed_FormatPacket( destEAddr ) => location of transmit buffer
 *  sed_Send( pkLength ) -- send the packet that is in the transmit buffer
 *  sed_Receive( recBufLocation ) -- enable receiving packets.
 *  sed_IsPacket() => location of packet in receive buffer
 *  sed_CheckPacket( recBufLocation, expectedType )
 *
 * Global Variables:
 *  sed_lclEthAddr -- Ethernet address of this host.
 *  sed_ethBcastAddr -- Ethernet broadcast address.
 */

#include "tinytcp.h"
#include "sed.h"

#define en10pages        ((en10size) >> pageshift)

octet *sed_va;                          /* virtual address of ethernet card */
eth_HwAddress sed_lclEthAddr;           /* local ethernet address */
eth_HwAddress sed_ethBcastAddr;         /* Ethernet broadcast address */
BOOL sed_respondARPreq;                 /* controls responses to ARP req's */
char bufAinUse, bufBinUse;              /* tell whether bufs are in use */

/* 
 *  Initialize the Ethernet Interface, and this package.  Enable input on
 * both buffers.
 */
sed_Init()
{
    int recState;
    register i, j;

    recState = 7;                       /* == mine + broad - errors */

    /* Map the Ethernet Interface in, and initialize sed_va */
    sed_va = (octet *)SED3CVA;        /* our virtual addr */

    /* Map memory for 3Com board (must be 8k boundary) */
    /* INSERT CODE HERE */
    map_ethernet_board();

    /* Reset the Ethernet controller */
    MECSR(sed_va) = RESET;
    for (i=0; i<10; i++);           /* delay a bit... */

    /* just copy on-board ROM to on-board RAM, to use std. address */
    Move(MEAROM(sed_va), sed_lclEthAddr, 6);
    Move(sed_lclEthAddr, MEARAM(sed_va), 6);
    
    MECSR(sed_va) |= AMSW;        /* and tell board we did it */

    /*
     * and initialize the exported variable which gives the Eth broadcast
     * address, for everyone else to use. 
     */
    for (i=0; i<3; i++) sed_ethBcastAddr[i] = 0xFFFF;
    
    /* accept packets addressed for us and broadcast packets */

    MECSR(sed_va) = (MECSR(sed_va)&~PA) | recState;

    /* declare both buffers in use... */
    bufAinUse = bufBinUse = TRUE;

}

/* 
 * Format an ethernet header in the transmit buffer, and say where it is.
 * Note that because of the way the 3Com interface works, we need to know
 * how long the packet is before we know where to put it.  The solution is
 * that we format the packet at the BEGINNING of the transmit buffer, and
 * later copy it (carefully) to where it belongs.  Another hack would be
 * to be inefficient about the size of the packet to be sent (always send
 * a larger ethernet packet than you need to, but copying should be ok for
 * now.
 */
octet *
sed_FormatPacket( destEAddr, ethType )
        register octet *destEAddr;
{
    register octet *xMitBuf;
    
    xMitBuf = &((octet *)MEXBUF(sed_va))[-0x800];
    Move( destEAddr, xMitBuf, 6 );
    Move( sed_lclEthAddr, xMitBuf + 6, 6 );
    *((short *)(xMitBuf+12)) = ethType;
    return ( xMitBuf+14 );
}

/*
 *  Send a packet out over the ethernet.  The packet is sitting at the
 * beginning of the transmit buffer.  The routine returns when the
 * packet has been successfully sent.
 */
sed_Send( pkLengthInOctets )
    register int pkLengthInOctets;
{
    register octet *fromO, *toO;
    register pkLength;
    register csr;

    pkLengthInOctets += 14;             /* account for Ethernet header */
    pkLengthInOctets = (pkLengthInOctets + 1) & (~1);

    if (pkLengthInOctets < E10P_MIN) 
        pkLengthInOctets = E10P_MIN; /* and min. ethernet len */

    /*  and copy the packet where it belongs */
    pkLength = pkLengthInOctets;
    fromO = &((octet *)MEXBUF(sed_va))[-0x800] + pkLength;
    toO = ((octet *)MEXBUF(sed_va));

    while ( pkLength-- ) *--toO = *--fromO;
    
    /* send the packet */
    
    MEXHDR(sed_va) = 2048 - pkLengthInOctets;
    MECSR(sed_va) |= TBSW;

    /* and wait until it has really been sent. */

    for (pkLength=0; pkLength < 15; pkLength++) {
        while ( (! ((csr = MECSR(sed_va)) & JAM)) && (csr & TBSW) )
            ;
        if (csr & JAM ) {
            /* Ethernet Collision detected... */
#ifdef DEBUG
            printf("sed: JAM: MECSR=%x\n", csr);
#endif
            MEBACK(sed_va) = clock_ValueRough();
            MECSR(sed_va) |= JAM;
        } else break;
    }
    if ( pkLength == 15 ) SysBug("Go and Buy a New Ethernet Interface.");

    /*  else we sent the packet ok. */
}

/* 
 *  Enable the Ethernet interface to receive packets from the network.  If
 * the argument is zero, enable both buffers.  If the argument is nonzero,
 * take it as the address of the buffer to be enabled.
 */
sed_Receive( recBufLocation )
    octet *recBufLocation;
{
    word enables = 0;

    if (recBufLocation == 0) {
        bufAinUse = FALSE;
        bufBinUse = FALSE;
        enables = (ABSW | BBSW);
    }
    recBufLocation -= 16;
    if (recBufLocation == ((octet *)MEAHDR(sed_va))) {
        bufAinUse = FALSE;
        enables = ABSW;
        }
    if (recBufLocation == ((octet *)MEBHDR(sed_va))) {
        bufBinUse = FALSE;
        enables = BBSW;
    }

    MECSR (sed_va) |= enables;
}

/* 
 * Test for the arrival of a packet on the Ethernet interface.  The packet may
 * arrive in either buffer A or buffer B; the location of the packet is
 * returned.  If no packet is returned withing 'timeout' milliseconds,
 * then the routine returns zero.
 * 
 * Note: ignores ethernet errors.  may occasionally return something
 * which was received in error.
 */

octet *
sed_IsPacket()
{
    register oldStatus;
    register octet *pb;
    
    pb = 0;
    if ( ! bufAinUse && (MECSR(sed_va)&ABSW) == 0 ) 
        pb = (octet *)MEAHDR(sed_va);
    if ( ! pb && ! bufBinUse && (MECSR(sed_va)&BBSW) == 0 )
        pb = (octet *)MEBHDR(sed_va);

    if ( pb ) {
        if ( ((octet *)pb) == ((octet *)MEAHDR(sed_va)) ) bufAinUse = 1;
        else bufBinUse = 1;
        pb += 16;                       /* get past the ethernet header */
    }

    return ( pb );
}

/* 
 *  Check to make sure that the packet that you received was the one that
 * you expected to get.
 */
sed_CheckPacket( recBufLocation, expectedType )
    word *recBufLocation;
    word expectedType;
{
    register recHeader = recBufLocation[-8];
    if ( (recHeader&R_ERROR) != 0 ||
         (recHeader&R_OFFSET) < E10P_MIN ) {
         return ( -1 );
    }
    if ( recBufLocation[-1] != expectedType ) {
        return ( 0 );
    }
    return (1);
}
