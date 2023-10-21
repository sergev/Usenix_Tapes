/*
 * tinytcp.h - header file for tinytcp.c
 *
 * Copyright (C) 1986, IMAGEN Corporation
 *  "This code may be duplicated in whole or in part provided that [1] there
 *   is no commercial gain involved in the duplication, and [2] that this
 *   copyright notice is preserved on all copies.  Any other duplication
 *   requires written notice of the author (Geoffrey H. Cooper)."
 *
 * Note: the structures herein must guarantee that the
 *       code only performs word fetches, since the
 *       imagenether card doesn't accept byte accesses.
 */

#define TRUE        1
#define true        1
#define FALSE       0
#define false       0
#define NULL        0               /* An empty value */
#define NIL         0               /* The distinguished empty pointer */

/* Useful type definitions */
typedef int (*procref)();
typedef short BOOL;                  /* boolean type */

/* Canonically-sized data */
typedef unsigned long longword;     /* 32 bits */
typedef unsigned short word;        /* 16 bits */
typedef unsigned char byte;         /*  8 bits */
typedef byte octet;                 /*  8 bits, for TCP */

#ifdef DDT
extern longword MsecClock();
#define clock_ValueRough() MsecClock()
#else
extern longword clock_MS;
#define clock_ValueRough() clock_MS
#endif

/* protocol address definitions */
typedef longword in_HwAddress;
typedef word eth_HwAddress[3];

/* The Ethernet header */
typedef struct {
    eth_HwAddress   destination;
    eth_HwAddress   source;
    word            type;
} eth_Header;

/* The Internet Header: */
typedef struct {
    word            vht;    /* version, hdrlen, tos */
    word            length;
    word            identification;
    word            frag;
    word            ttlProtocol;
    word            checksum;
    in_HwAddress    source;
    in_HwAddress    destination;
} in_Header;
#define in_GetVersion(ip) (((ip)->vht >> 12) & 0xf)
#define in_GetHdrlen(ip)  (((ip)->vht >> 8) & 0xf)
#define in_GetHdrlenBytes(ip)  (((ip)->vht >> 6) & 0x3c)
#define in_GetTos(ip)      ((ip)->vht & 0xff)

#define in_GetTTL(ip)      ((ip)->ttlProtocol >> 8)
#define in_GetProtocol(ip) ((ip)->ttlProtocol & 0xff)


typedef struct {
    word            srcPort;
    word            dstPort;
    longword        seqnum;
    longword        acknum;
    word            flags;
    word            window;
    word            checksum;
    word            urgentPointer;
} tcp_Header;


#define tcp_FlagFIN     0x0001
#define tcp_FlagSYN     0x0002
#define tcp_FlagRST     0x0004
#define tcp_FlagPUSH    0x0008
#define tcp_FlagACK     0x0010
#define tcp_FlagURG     0x0020
#define tcp_FlagDO      0xF000
#define tcp_GetDataOffset(tp) ((tp)->flags >> 12)

/* The TCP/UDP Pseudo Header */
typedef struct {
    in_HwAddress    src;
    in_HwAddress    dst;
    octet           mbz;
    octet           protocol;
    word            length;
    word            checksum;
} tcp_PseudoHeader;

/*
 * TCP states, from tcp manual.
 * Note: close-wait state is bypassed by automatically closing a connection
 *       when a FIN is received.  This is easy to undo.
 */
#define tcp_StateLISTEN  0      /* listening for connection */
#define tcp_StateSYNSENT 1      /* syn sent, active open */
#define tcp_StateSYNREC  2      /* syn received, synack+syn sent. */
#define tcp_StateESTAB   3      /* established */
#define tcp_StateFINWT1  4      /* sent FIN */
#define tcp_StateFINWT2  5      /* sent FIN, received FINACK */
/*#define tcp_StateCLOSEWT 6    /* received FIN waiting for close */
#define tcp_StateCLOSING 6      /* sent FIN, received FIN (waiting for FINACK) */
#define tcp_StateLASTACK 7      /* fin received, finack+fin sent */
#define tcp_StateTIMEWT  8      /* dally after sending final FINACK */
#define tcp_StateCLOSED  9      /* finack received */

/*
 * TCP Socket definition
 */
#define tcp_MaxData 32              /* maximum bytes to buffer on output */

typedef struct _tcp_socket {
    struct _tcp_socket *next;
    short           state;          /* connection state */
    procref         dataHandler;    /* called with incoming data */
    eth_HwAddress   hisethaddr;     /* ethernet address of peer */
    in_HwAddress    hisaddr;        /* internet address of peer */
    word            myport, hisport;/* tcp ports for this connection */
    longword        acknum, seqnum; /* data ack'd and sequence num */
    int             timeout;        /* timeout, in milliseconds */
    BOOL            unhappy;        /* flag, indicates retransmitting segt's */
    word            flags;          /* tcp flags word for last packet sent */
    short           dataSize;       /* number of bytes of data to send */
    byte            data[tcp_MaxData]; /* data to send */
} tcp_Socket;

extern eth_HwAddress sed_lclEthAddr;
extern eth_HwAddress sed_ethBcastAddr;
extern in_HwAddress  sin_lclINAddr;

/*
 * ARP definitions
 */
#define arp_TypeEther  1        /* ARP type of Ethernet address *

/* harp op codes */
#define ARP_REQUEST 1
#define ARP_REPLY   2

/*
 * Arp header
 */
typedef struct {
    word            hwType;
    word            protType;
    word            hwProtAddrLen;  /* hw and prot addr len */
    word            opcode;
    eth_HwAddress   srcEthAddr;
    in_HwAddress    srcIPAddr;
    eth_HwAddress   dstEthAddr;
    in_HwAddress    dstIPAddr;
} arp_Header;

/*
 * Ethernet interface:
 *   sed_WaitPacket(0) => ptr to packet (beyond eth header)
 *                          or NIL if no packet ready.
 *   sed_Receive(ptr) - reenables receive on input buffer
 *   sed_FormatPacket(&ethdest, ethtype) => ptr to packet buffer
 *   sed_Send(packet_length) - send the buffer last formatted.
 */
byte *sed_IsPacket(), *sed_FormatPacket();
