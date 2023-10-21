/*
 * Copyright 1986 by Larry Campbell, 73 Concord Street, Maynard MA 01754 USA
 * (maynard!campbell).  You may freely copy, use, and distribute this software
 * subject to the following restrictions:
 *
 *  1)	You may not charge money for it.
 *  2)	You may not remove or alter this copyright notice.
 *  3)	You may not claim you wrote it.
 *  4)	If you make improvements (or other changes), you are requested
 *	to send them to me, so there's a focal point for distributing
 *	improved versions.
 *
 * John Chmielewski (tesla!jlc until 9/1/86, then rogue!jlc) assisted
 * by doing the System V port and adding some nice features.  Thanks!
 */

/*
 * BSR X-10 "Powerhouse" Computer Interface Definitions
 */

/***********************************************************************/
/* You probably only need to hack this section to reconfigure for      */
/* your system                                                         */
/***********************************************************************/

/* Some lints don't know about the void type */

#ifdef VOID
#define void int
#endif

#define DEVNAME	 "/dev/x10"	/* terminal line X10 box is connected to */
#define ROOTNAME "/dev/x10fs"	/* name of root filesystem */

#ifndef XDIR
#define XDIR "."		/* directory containing X10 files */
#endif

#define IDFILE "/id"		/* description file for X10 modules */

#ifdef  MINIEXCH		/* if talking through a DEC Mini-Exchange */
#define MINIXPORT 3		/* port number X10 gizmo is plugged in to */
#endif

#ifdef  VENIX
#define SMALLPAUSE -10		/* 1/6th of a second sleep(3) (VENIX only) */
#else
#define SMALLPAUSE 1
#endif

#define TIMEOUT 10		/* seconds to wait for data */
#define DTIMEOUT 15		/* timeout for dim and diagnostic commands */

/***********************************************************************/
/*                  End of configuration section                       */
/***********************************************************************/


#define SYNCN 16		/* number of FF chars to send before packet */

#define CHKSUM(buf) chksum(buf, sizeof(buf))

/* Event item as stored in event file */

struct evitem
    {
    unsigned e_num;
    unsigned char e_buf[8];
    };

#define EVSIZE sizeof(struct evitem)

#define EVENTS "events"		/* event data keyword */
#define ETOTAL 128		/* total number of events */
#define ESIZE  8		/* size of event data field */
#define EVCMD  12		/* size of event command */

/* Data item as stored in data file */

struct ditem
    {
    unsigned d_num;
    unsigned char d_buf[2];
    };

#define DISIZE 6	/* sizeof not used as it includes holes */

#define DATA   "data"		/* id data keyword */
#define DTOTAL 256		/* total number of id's */
#define DSIZE  2		/* size of id data field */
#define DICMD  6		/* size of data command */

/* description field structure */

#define DLENGTH 40		/* length of the description field */

struct id
    {
    char describe[DLENGTH];
    };

/* Command codes */

#define SETHCODE	0	/* load house code */
#define DIRCMD		1	/* direct command */
#define SETCLK		2	/* set clock */
#define DATALOAD	3	/* timer/graphics data download */
#define GETINFO		4	/* get house code and clock */
#define GETEVENTS	5	/* get timer events */
#define GETDATA		6	/* get graphics data */
#define DIAGNOSE	7	/* run diagnostic */

#define XMTSYNC		16	/* transmitted sync length */
#define RCVSYNC		6	/* received sync length */

/* House code magic numbers */

#define HC_A	0x60
#define HC_B	0xE0
#define HC_C	0x20
#define HC_D	0xA0
#define HC_E	0x10
#define HC_F	0x90
#define HC_G	0x50
#define HC_H	0xD0
#define HC_I	0x70
#define HC_J	0xF0
#define HC_K	0x30
#define HC_L	0xB0
#define HC_M	0x00
#define HC_N	0x80
#define HC_O	0x40
#define HC_P	0xC0

struct hstruct
    {
    unsigned char h_code;
    char h_letter;
    };

struct nstruct
    {
    char *n_name;
    char n_code;
    };

/* Message definitions */

#define EM_2MANY	"Too many command line arguments"
#define EM_INVCN	"Invalid command name"
#define EM_WNA		"Wrong number of arguments"
#define EM_NMA		"Need more command line arguments"
#define EM_NOCMD	"No command argument specified"

/* External Variables */

extern char *E_2MANY, *E_INVCN, *E_WNA, *E_NMA, *E_NOCMD;
