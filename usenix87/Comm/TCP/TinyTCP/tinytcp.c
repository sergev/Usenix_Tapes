/*
 * tinytcp.c - Tiny Implementation of the Transmission Control Protocol
 *
 * Written March 28, 1986 by Geoffrey Cooper, IMAGEN Corporation.
 *
 * This code is a small implementation of the TCP and IP protocols, suitable
 * for burning into ROM.  The implementation is bare-bones and represents
 * two days' coding efforts.  A timer and an ethernet board are assumed.  The
 * implementation is based on busy-waiting, but the tcp_handler procedure
 * could easily be integrated into an interrupt driven scheme.
 *
 * IP routing is accomplished on active opens by broadcasting the tcp SYN
 * packet when ARP mapping fails.  If anyone answers, the ethernet address
 * used is saved for future use.  This also allows IP routing on incoming
 * connections.
 * 
 * The TCP does not implement urgent pointers (easy to add), and discards
 * segments that are received out of order.  It ignores the received window
 * and always offers a fixed window size on input (i.e., it is not flow
 * controlled).
 *
 * Special care is taken to access the ethernet buffers only in word
 * mode.  This is to support boards that only allow word accesses.
 *
 * Copyright (C) 1986, IMAGEN Corporation
 *  "This code may be duplicated in whole or in part provided that [1] there
 *   is no commercial gain involved in the duplication, and [2] that this
 *   copyright notice is preserved on all copies.  Any other duplication
 *   requires written notice of the author (Geoffrey H. Cooper)."
 */

#include "tinytcp.h"

/*
 * Local IP address
 */
in_HwAddress sin_lclINAddr;

/*
 * IP identification numbers
 */
int tcp_id;

tcp_Socket *tcp_allsocs;

/* Timer definitions */
#define tcp_RETRANSMITTIME 1000     /* interval at which retransmitter is called */
#define tcp_LONGTIMEOUT 31000       /* timeout for opens */
#define tcp_TIMEOUT 10000           /* timeout during a connection */

#ifdef DEBUG
/* 
 * Primitive logging facility
 */
#define tcp_LOGPACKETS 1        /* log packet headers */
word tcp_logState;
#endif

/*
 * Initialize the tcp implementation
 */
tcp_Init()
{
    extern eth_HwAddress sed_lclEthAddr;

    /* initialize ethernet interface */
    sed_Init();

    tcp_allsocs = NIL;
#ifdef DEBUG
    tcp_logState = 0;
#endif
    tcp_id = 0;

    /* hack - assume the network number */
    sin_lclINAddr = 0x7d000000 + (*((longword *)&sed_lclEthAddr[1]) & 0xFFFFFF);
}

/*
 * Actively open a TCP connection to a particular destination.
 */
tcp_Open(s, lport, ina, port, datahandler)
    tcp_Socket *s;
    in_HwAddress ina;
    word lport, port;
    procref datahandler;
{
    extern eth_HwAddress sed_ethBcastAddr;

    s->state = tcp_StateSYNSENT;
    s->timeout = tcp_LONGTIMEOUT;
    if ( lport == 0 ) lport = clock_ValueRough();
    s->myport = lport;
    if ( ! sar_MapIn2Eth(ina, &s->hisethaddr[0]) ) {
        printf("tcp_Open of 0x%x: defaulting ethernet address to broadcast\n", ina);
        Move(&sed_ethBcastAddr[0], &s->hisethaddr[0], sizeof(eth_HwAddress));
    }
    s->hisaddr = ina;
    s->hisport = port;
    s->seqnum = 0;
    s->dataSize = 0;
    s->flags = tcp_FlagSYN;
    s->unhappy = true;
    s->dataHandler = datahandler;
    s->next = tcp_allsocs;
    tcp_allsocs = s;
    tcp_Send(s);
}

/*
 * Passive open: listen for a connection on a particular port
 */
tcp_Listen(s, port, datahandler, timeout)
    tcp_Socket *s;
    word port;
    procref datahandler;
{
    s->state = tcp_StateLISTEN;
    if ( timeout == 0 ) s->timeout = 0x7ffffff; /* forever... */
    else s->timeout = timeout;
    s->myport = port;
    s->hisport = 0;
    s->seqnum = 0;
    s->dataSize = 0;
    s->flags = 0;
    s->unhappy = 0;
    s->dataHandler = datahandler;
    s->next = tcp_allsocs;
    tcp_allsocs = s;
}

/*
 * Send a FIN on a particular port -- only works if it is open
 */
tcp_Close(s)
    tcp_Socket *s;
{
    if ( s->state == tcp_StateESTAB || s->state == tcp_StateSYNREC ) {
        s->flags = tcp_FlagACK | tcp_FlagFIN;
        s->state = tcp_StateFINWT1;
        s->unhappy = true;
    }
}

/*
 * Abort a tcp connection
 */
tcp_Abort(s)
    tcp_Socket *s;
{
    if ( s->state != tcp_StateLISTEN && s->state != tcp_StateCLOSED ) {
        s->flags = tcp_FlagRST | tcp_FlagACK;
        tcp_Send(s);
    }
    s->unhappy = 0;
    s->dataSize = 0;
    s->state = tcp_StateCLOSED;
    s->dataHandler(s, 0, -1);
    tcp_Unthread(s);
}

/*
 * Retransmitter - called periodically to perform tcp retransmissions
 */
tcp_Retransmitter()
{
    tcp_Socket *s;
    BOOL x;

    for ( s = tcp_allsocs; s; s = s->next ) {
        x = false;
        if ( s->dataSize > 0 || s->unhappy ) {
            tcp_Send(s);
            x = true;
        }
        if ( x || s->state != tcp_StateESTAB )
            s->timeout -= tcp_RETRANSMITTIME;
        if ( s->timeout <= 0 ) {
            if ( s->state == tcp_StateTIMEWT ) {
                printf("Closed.    \n");
                s->state = tcp_StateCLOSED;
                s->dataHandler(s, 0, 0);
                tcp_Unthread(s);
            } else {
                printf("Timeout, aborting\n");
                tcp_Abort(s);
            }
        }
    }
}

/*
 * Unthread a socket from the socket list, if it's there 
 */
tcp_Unthread(ds)
    tcp_Socket *ds;
{
    tcp_Socket *s, **sp;

    sp = &tcp_allsocs;
    for (;;) {
        s = *sp;
        if ( s == ds ) {
            *sp = s->next;
            break;
        }
        if ( s == NIL ) break;
        sp = &s->next;
    }
}

/*
 * busy-wait loop for tcp.  Also calls an "application proc"
 */
tcp(application)
    procref application;
{
    in_Header *ip;
    longword timeout, start;
    int x;

    sed_Receive(0);

    timeout = 0;
    while ( tcp_allsocs ) {
        start = clock_ValueRough();
        ip = sed_IsPacket();
        if ( ip == NIL ) {
            if ( clock_ValueRough() > timeout ) {
                tcp_Retransmitter();
                timeout = clock_ValueRough() + tcp_RETRANSMITTIME;
            }

            application();

            continue;
        }

        if ( sed_CheckPacket(ip, 0x806) == 1 ) {
            /* do arp */
            sar_CheckPacket(ip);

        } else if ( sed_CheckPacket(ip, 0x800) == 1 ) {
            /* do IP */
            if ( ip->destination == sin_lclINAddr &&
                 in_GetProtocol(ip) == 6 &&
                 checksum(ip, in_GetHdrlenBytes(ip)) == 0xFFFF ) {
                tcp_Handler(ip);
            }
        }
        /* recycle buffer */
        sed_Receive(ip);

        x = clock_ValueRough() - start;
        timeout -= x;
    }

    return ( 1 );
}

/*
 * Write data to a connection.
 * Returns number of bytes written, == 0 when connection is not in
 * established state.
 */
tcp_Write(s, dp, len)
    tcp_Socket *s;
    byte *dp;
    int len;
{
    int x;

    if ( s->state != tcp_StateESTAB ) len = 0;
    if ( len > (x = tcp_MaxData - s->dataSize) ) len = x;
    if ( len > 0 ) {
        Move(dp, &s->data[s->dataSize], len);
        s->dataSize += len;
        tcp_Flush(s);
    }

    return ( len );
}

/*
 * Send pending data
 */
tcp_Flush(s)
    tcp_Socket *s;
{
    if ( s->dataSize > 0 ) {
        s->flags |= tcp_FlagPUSH;
        tcp_Send(s);
    }
}

/*
 * Handler for incoming packets.
 */
tcp_Handler(ip)
    in_Header *ip;
{
    tcp_Header *tp;
    tcp_PseudoHeader ph;
    int len;
    byte *dp;
    int x, diff;
    tcp_Socket *s;
    word flags;

    len = in_GetHdrlenBytes(ip);
    tp = (tcp_Header *)((byte *)ip + len);
    len = ip->length - len;

    /* demux to active sockets */
    for ( s = tcp_allsocs; s; s = s->next )
        if ( s->hisport != 0 &&
             tp->dstPort == s->myport &&
             tp->srcPort == s->hisport &&
             ip->source == s->hisaddr ) break;
    if ( s == NIL ) {
        /* demux to passive sockets */
        for ( s = tcp_allsocs; s; s = s->next )
            if ( s->hisport == 0 && tp->dstPort == s->myport ) break;
    }
    if ( s == NIL ) {
#ifdef DEBUG
        if ( tcp_logState & tcp_LOGPACKETS ) tcp_DumpHeader(ip, tp, "Discarding");
#endif
        return;
    }

#ifdef DEBUG
    if ( tcp_logState & tcp_LOGPACKETS )
        tcp_DumpHeader(ip, tp, "Received");
#endif

    /* save his ethernet address */
    MoveW(&((((eth_Header *)ip) - 1)->source[0]), &s->hisethaddr[0], sizeof(eth_HwAddress));

    ph.src = ip->source;
    ph.dst = ip->destination;
    ph.mbz = 0;
    ph.protocol = 6;
    ph.length = len;
    ph.checksum = checksum(tp, len);
    if ( checksum(&ph, sizeof ph) != 0xffff )
         printf("bad tcp checksum, received anyway\n");

    flags = tp->flags;
    if ( flags & tcp_FlagRST ) {
        printf("connection reset\n");
        s->state = tcp_StateCLOSED;
        s->dataHandler(s, 0, -1);
        tcp_Unthread(s);
        return;
    }

    switch ( s->state ) {

    case tcp_StateLISTEN:
        if ( flags & tcp_FlagSYN ) {
            s->acknum = tp->seqnum + 1;
            s->hisport = tp->srcPort;
            s->hisaddr = ip->source;
            s->flags = tcp_FlagSYN | tcp_FlagACK;
            tcp_Send(s);
            s->state = tcp_StateSYNREC;
            s->unhappy = true;
            s->timeout = tcp_TIMEOUT;
            printf("Syn from 0x%x#%d (seq 0x%x)\n", s->hisaddr, s->hisport, tp->seqnum);
        }
        break;

    case tcp_StateSYNSENT:
        if ( flags & tcp_FlagSYN ) {
            s->acknum++;
            s->flags = tcp_FlagACK;
            s->timeout = tcp_TIMEOUT;
            if ( (flags & tcp_FlagACK) && tp->acknum == (s->seqnum + 1) ) {
                printf("Open\n");
                s->state = tcp_StateESTAB;
                s->seqnum++;
                s->acknum = tp->seqnum + 1;
                s->unhappy = false;
            } else {
                s->state = tcp_StateSYNREC;
            }
        }
        break;

    case tcp_StateSYNREC:
        if ( flags & tcp_FlagSYN ) {
            s->flags = tcp_FlagSYN | tcp_FlagACK;
            tcp_Send(s);
            s->timeout = tcp_TIMEOUT;
            printf(" retransmit of original syn\n");
        }
        if ( (flags & tcp_FlagACK) && tp->acknum == (s->seqnum + 1) ) {
            s->flags = tcp_FlagACK;
            tcp_Send(s);
            s->seqnum++;
            s->unhappy = false;
            s->state = tcp_StateESTAB;
            s->timeout = tcp_TIMEOUT;
            printf("Synack received - connection established\n");
        }
        break;

    case tcp_StateESTAB:
        if ( (flags & tcp_FlagACK) == 0 ) return;
        /* process ack value in packet */
        diff = tp->acknum - s->seqnum;
        if ( diff > 0 ) {
            Move(&s->data[diff], &s->data[0], diff);
            s->dataSize -= diff;
            s->seqnum += diff;
        }
        s->flags = tcp_FlagACK;
        tcp_ProcessData(s, tp, len);
        break;

    case tcp_StateFINWT1:
        if ( (flags & tcp_FlagACK) == 0 ) return;
        diff = tp->acknum - s->seqnum - 1;
        s->flags = tcp_FlagACK | tcp_FlagFIN;
        if ( diff == 0 ) {
            s->state = tcp_StateFINWT2;
            s->flags = tcp_FlagACK;
            printf("finack received.\n");
        }
        tcp_ProcessData(s, tp, len);
        break;

    case tcp_StateFINWT2:
        s->flags = tcp_FlagACK;
        tcp_ProcessData(s, tp, len);
        break;

    case tcp_StateCLOSING:
        if ( tp->acknum == (s->seqnum + 1) ) {
            s->state = tcp_StateTIMEWT;
            s->timeout = tcp_TIMEOUT;
        }
        break;

    case tcp_StateLASTACK:
        if ( tp->acknum == (s->seqnum + 1) ) {
            s->state = tcp_StateCLOSED;
            s->unhappy = false;
            s->dataSize = 0;
            s->dataHandler(s, 0, 0);
            tcp_Unthread(s);
            printf("Closed.    \n");
        } else {
            s->flags = tcp_FlagACK | tcp_FlagFIN;
            tcp_Send(s);
            s->timeout = tcp_TIMEOUT;
            printf("retransmitting FIN\n");
        }
        break;

    case tcp_StateTIMEWT:
        s->flags = tcp_FlagACK;
        tcp_Send(s);
    }
}

/*
 * Process the data in an incoming packet.
 * Called from all states where incoming data can be received: established,
 * fin-wait-1, fin-wait-2
 */
tcp_ProcessData(s, tp, len)
    tcp_Socket *s;
    tcp_Header *tp;
    int len;
{
    int diff, x;
    word flags;
    byte *dp;

    flags = tp->flags;
    diff = s->acknum - tp->seqnum;
    if ( flags & tcp_FlagSYN ) diff--;
    x = tcp_GetDataOffset(tp) << 2;
    dp = (byte *)tp + x;
    len -= x;
    if ( diff >= 0 ) {
        dp += diff;
        len -= diff;
        s->acknum += len;
        s->dataHandler(s, dp, len);
        if ( flags & tcp_FlagFIN ) {
            s->acknum++;
#ifdef DEBUG
            printf("consumed fin.\n");
#endif
            switch(s->state) {
              case tcp_StateESTAB:
                /* note: skip state CLOSEWT by automatically closing conn */
                x = tcp_StateLASTACK;
                s->flags |= tcp_FlagFIN;
                s->unhappy = true;
#ifdef DEBUG
                printf("sending fin.\n");
#endif
                break;
              case tcp_StateFINWT1:
                x = tcp_StateCLOSING;
                break;
              case tcp_StateFINWT2:
                x = tcp_StateTIMEWT;
                break;
            }
            s->state = x;
        }
    }
    s->timeout = tcp_TIMEOUT;
    tcp_Send(s);
}

/*
 * Format and send an outgoing segment
 */
tcp_Send(s)
    tcp_Socket *s;
{
    tcp_PseudoHeader ph;
    struct _pkt {
        in_Header in;
        tcp_Header tcp;
        longword maxsegopt;
    } *pkt;
    byte *dp;

    pkt = (struct _pkt *)sed_FormatPacket(&s->hisethaddr[0], 0x800);
    dp = &pkt->maxsegopt;

    pkt->in.length = sizeof(in_Header) + sizeof(tcp_Header) + s->dataSize;

    /* tcp header */
    pkt->tcp.srcPort = s->myport;
    pkt->tcp.dstPort = s->hisport;
    pkt->tcp.seqnum = s->seqnum;
    pkt->tcp.acknum = s->acknum;
    pkt->tcp.window = 1024;
    pkt->tcp.flags = s->flags | 0x5000;
    pkt->tcp.checksum = 0;
    pkt->tcp.urgentPointer = 0;
    if ( s->flags & tcp_FlagSYN ) {
        pkt->tcp.flags += 0x1000;
        pkt->in.length += 4;
        pkt->maxsegopt = 0x02040578; /* 1400 bytes */
        dp += 4;
    }
    MoveW(s->data, dp, s->dataSize);

    /* internet header */
    pkt->in.vht = 0x4500;   /* version 4, hdrlen 5, tos 0 */
    pkt->in.identification = tcp_id++;
    pkt->in.frag = 0;
    pkt->in.ttlProtocol = (250<<8) + 6;
    pkt->in.checksum = 0;
    pkt->in.source = sin_lclINAddr;
    pkt->in.destination = s->hisaddr;
    pkt->in.checksum = ~checksum(&pkt->in, sizeof(in_Header));

    /* compute tcp checksum */
    ph.src = pkt->in.source;
    ph.dst = pkt->in.destination;
    ph.mbz = 0;
    ph.protocol = 6;
    ph.length = pkt->in.length - sizeof(in_Header);
    ph.checksum = checksum(&pkt->tcp, ph.length);
    pkt->tcp.checksum = ~checksum(&ph, sizeof ph);

#ifdef DEBUG
    if ( tcp_logState & tcp_LOGPACKETS )
        tcp_DumpHeader(&pkt->in, &pkt->tcp, "Sending");
#endif

    sed_Send(pkt->in.length);
}

/*
 * Do a one's complement checksum
 */
checksum(dp, length)
    word *dp;
    int length;
{
    int len;
    longword sum;

    len = length >> 1;
    sum = 0;
    while ( len-- > 0 ) sum += *dp++;
    if ( length & 1 ) sum += (*dp & 0xFF00);
    sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);
    sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);

    return ( sum );
}

/*
 * Dump the tcp protocol header of a packet
 */
tcp_DumpHeader( ip, tp, mesg )
    in_Header *ip;
    char *mesg;
{
    register tcp_Header *tp = (tcp_Header *)((byte *)ip + in_GetHdrlenBytes(ip));
    static char *flags[] = { "FIN", "SYN", "RST", "PUSH", "ACK", "URG" };
    int len;
    word f;

    len =  ip->length - ((tcp_GetDataOffset(tp) + in_GetHdrlen(ip)) << 2);
    printf("TCP: %s packet:\nS: %x; D: %x; SN=%x ACK=%x W=%d DLen=%d\n",
           mesg, tp->srcPort, tp->dstPort, tp->seqnum, tp->acknum,
           tp->window, len);
    printf("DO=%d, C=%x U=%d",
           tcp_GetDataOffset(tp), tp->checksum, tp->urgentPointer);
    /* output flags */
    f = tp->flags;
    for ( len = 0; len < 6; len++ )
        if ( f & (1 << len) ) printf(" %s", flags[len]);
    printf("\n");
}

/*
 * Move bytes from hither to yon
 */
Move( src, dest, numbytes )
    register byte *src, *dest;
    register numbytes;
{
    if ( numbytes <= 0 ) return;
    if ( src < dest ) {
        src += numbytes;
        dest += numbytes;
        do {
            *--dest = *--src;
        } while ( --numbytes > 0 );
    } else
        do {
             *dest++ = *src++;
        } while ( --numbytes > 0 );
}
