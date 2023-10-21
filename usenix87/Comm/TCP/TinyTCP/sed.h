/* 
 *  Header file for very simple ethernet driver, based on 3Com Multibus
 *  board.
 *
 * Copyright (C) 1986, IMAGEN Corporation
 *  "This code may be duplicated in whole or in part provided that [1] there
 *   is no commercial gain involved in the duplication, and [2] that this
 *   copyright notice is preserved on all copies.  Any other duplication
 *   requires written notice of the author (Geoffrey H. Cooper)."
 */

#define	en10size        (8*1024)	/* size of interface memory */
#define	en10pages	((en10size) >> pageshift)
#define E10P_MIN	60              /* Minimum Ethernet packet size */

/* 
 * The position of the 3Com interface in virtual memory.  If we're
 * Running the bootloader function, then it must be in the last 8k
 * of virtual addresses.
 */
#ifdef BOOTLOADER
#define SED3CVA vm_3ComAdr /* hack, only need pb68.h if bootloader */
#endif
#ifndef SED3CVA
#define SED3CVA 0x1c000
#endif

/* 10Mb Ethernet interface addresses */

#define	MECSR(eth_va)	*(word*)(((octet *) eth_va) + 0x0)
#define	MEBACK(eth_va)	*(word*)(((octet *) eth_va) + 0x2)
#define	MEAROM(eth_va)	(word*)(((octet *) eth_va) + 0x400)
#define	MEARAM(eth_va)	(word*)(((octet *) eth_va) + 0x600)
#define	MEXHDR(eth_va)	*(word*)(((octet *) eth_va) + 0x800)
#define	MEXBUF(eth_va)	(word*)(((octet *) eth_va) + 0x1000)
#define	MEAHDR(eth_va)	(word*)(((octet *) eth_va) + 0x1000)
#define	MEBHDR(eth_va)	(word*)(((octet *) eth_va) + 0x1800)

/* control/status register fields */

#define	BBSW		0x8000	/* Buffer B belongs to Network */
#define	ABSW		0x4000	/* Buffer A belongs to Network */
#define	TBSW		0x2000	/* Transmit buffer belongs to Network */
#define	JAM		0x1000	/* Set when transmit collision */
#define	AMSW		0x0800	/* 
#define	RBBA		0x0400	/* Oldest received packet is in B */
/*#define	UNUSED		0x0200 */
#define	RESET		0x0100	/* Reset the controller */
#define	BINT		0x0080	/* Interrupt when BBSW=>0 (packet in B) */
#define	AINT		0x0040	/* Interrupt when ABSW=>0 (packet in A) */
#define	TINT		0x0020	/* Interrupt when TBSW=>0 (transmit done) */
#define	JINT		0x0010	/* Enable interrupts when JAM=>1 */
#define	PA		0x000F	/* Which packets should be received? */
#define INTENABLS	0x00F0

/*
 * Receiver Header Fields: 
 * The receiver header is the first (short) word of the receive buffer.  It
 * includes such information as how big the packet is, whether it was a
 * broadcast, whether there was an error in receiving it, etc.
 */

#define	R_FCS		0x8000	/* fcs error */
#define	R_BCAST		0x4000	/* packet was NOT a broadcast */
#define	R_RANGE		0x2000	/* range error (size of pkt?) */
#define	R_MATCH		0x1000	/* packet is multicast (i.e., address
				   received is not that of the interface) */
#define	R_FRAME		0x0800	/* framing error */
#define	R_ERROR		0x8800	/* was there any error */
#define	R_OFFSET	0x07FF	/* packet length + 1 word */

extern octet *sed_FormatPacket(), *sed_WaitPacket();

#ifdef BOOTLOADER
#define ConsPrintf printf
#endif
