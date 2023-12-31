char *versio = "C-Kermit 4.2(030) PRERELEASE # 2, 5 March 85";

/* C K M A I N  --  C-Kermit Main program */

/*
 Authors: Frank da Cruz, Bill Catchings, Jeff Damens;
 Columbia University Center for Computing Activities, 1984-85.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to copy or use this
 program except for explicitly commercial purposes, provided this copyright
 notice is retained.
*/
/*
 The Kermit file transfer protocol was developed at Columbia University.
 It is named after Kermit the Frog, star of the television series THE
 MUPPET SHOW; the name is used by permission of Henson Associates, Inc.
 "Kermit" is also Celtic for "free".
*/
/*
 Thanks to Herm Fischer, Litton Data Systems, for extensive contributions to
 version 4, and to the following people for their contributions over the years:

   Bob Cattani & Chris Maio, Columbia CS Dept
   Alan Crosswell, CUCCA
   Carl Fongheiser, CWRU
   Jim Guyton, Rand Corp
   Stan Hanks, Rice U.
   Ken Harrenstein, SRI
   Steve Hemminger, Tektronix
   Tony Movshon, NYU
   Ken Poulton, HP Labs
   Dave Tweten, NASA Ames
   Walter Underwood, Ford Aerospace
   Pieter Van Der Linden, Centre Mondial (Paris)
   Lauren Weinstein, Vortex

 and many others.
*/

#include "ckermi.h"

/* Text message definitions */

#ifndef XENIX

char *hlptxt = "C-Kermit Server\n\
\n\
Server Function    Customary Command to Invoke the Function\n\
 Who's Logged In?   REMOTE WHO [user]\n\
 Help               REMOTE HELP\n\
 Finish Serving     FINISH\n\
 Send File(s)       GET filespec\n\
 Receive File(s)    SEND filespec\n\
 Directory Listing  REMOTE DIRECTORY [filespec]\n\
 Change Directory   REMOTE CWD [directory]\n\
 Type File(s)       REMOTE TYPE filespec\n\
 Delete File(s)     REMOTE DELETE filespec\n\
 Disk Usage Query   REMOTE SPACE [directory]\n\
 Unix Shell Command REMOTE HOST command-string\n\
\n\0";

#else

char *hlptxt = "C-Kermit Server Commands Supported:\n\
\n\
GET filespec	REMOTE CWD [directory]		REMOTE SPACE [directory]\n\
SEND filespec	REMOTE DIRECTORY [filespec]	REMOTE HOST command-string\n\
FINISH		REMOTE DELETE filespec		REMOTE WHO [user]\n\
REMOTE HELP	REMOTE TYPE filespec\n\
\n\0";

#endif

char *srvtxt = "\r\n\
C-Kermit server starting.  Return to your local machine by typing\r\n\
its escape sequence for closing the connection, and issue further\r\n\
commands from there.  To shut down the C-Kermit server, issue the\r\n\
FINISH command and then reconnect.\n\
\r\n\0";

/* Declarations for Send-Init Parameters */

int spsiz =  DSPSIZ,			/* maximum packet size we can send */
    timint = DMYTIM,			/* My timeout interval */
    timef = 0,				/* Flag for override packet timeout */
    npad = MYPADN,			/* How much padding to send */
    mypadn = MYPADN,			/* How much padding to ask for */
    chklen = 1,				/* Length of block check */
    bctr = 1,				/* Block check type requested */
    bctu = 1,				/* Block check type used */
    ebq =  MYEBQ,			/* 8th bit prefix */
    ebqflg = 0,				/* 8th-bit quoting flag */
    rpt = 0,				/* Repeat count */
    rptq = MYRPTQ,			/* Repeat prefix */
    rptflg = 0,				/* Repeat processing flag */
    capas = 0;				/* Capabilities */

char padch = MYPADC,			/* Padding character to send */
    mypadc = MYPADC,			/* Padding character to ask for */
    eol = MYEOL,			/* End-Of-Line character to send */
    ctlq = CTLQ,			/* Control prefix in incoming data */
    myctlq = CTLQ;			/* Outbound control character prefix */


/* Packet-related variables */

int pktnum = 0,				/* Current packet number */
    prvpkt = -1,			/* Previous packet number */
    sndtyp,				/* Type of packet just sent */
    size,				/* Current size of output pkt data */
    osize,				/* Previous output packet data size */
    maxsize,				/* Max size for building data field */
    spktl;				/* Length packet being sent */

char sndpkt[MAXPACK*2], 		/* Entire packet being sent */
    recpkt[RBUFL], 			/* Packet most recently received */
    data[MAXPACK+4],   			/* Packet data buffer */
    srvcmd[MAXPACK*2],			/* Where to decode server command */
    *srvptr,				/* Pointer to above */
    mystch = SOH,			/* Outbound packet-start character */
    stchr = SOH;			/* Incoming packet-start character */

/* File-related variables */

char filnam[50];			/* Name of current file. */

int nfils,				/* Number of files in file group */
    fsize;				/* Size of current file */

/* Communication line variables */

char ttname[50];			/* Name of communication line. */

int parity,				/* Parity specified, 0,'e','o',etc */
    flow,				/* Flow control, 1 = xon/xoff */
    speed = -1,				/* Line speed */
    turn = 0,				/* Line turnaround handshake flag */
    turnch = XON,			/* Line turnaround character */
    duplex = 0,				/* Duplex, full by default */
    escape = 034,			/* Escape character for connect */
    delay = DDELAY,			/* Initial delay before sending */
    mdmtyp = 0;				/* Type of modem 1=hayes 2=ventel */


/* Statistics variables */

long filcnt,			/* Number of files in transaction */
    flci,			/* Characters from line, current file */
    flco,			/* Chars to line, current file  */
    tlci,			/* Chars from line in transaction */
    tlco,   	    	    	/* Chars to line in transaction */
    ffc,			/* Chars to/from current file */
    tfc;			/* Chars to/from files in transaction */

/* Flags */

int deblog = 0,				/* Flag for debug logging */
    pktlog = 0,				/* Flag for packet logging */
    seslog = 0,				/* Session logging */
    tralog = 0,				/* Transaction logging */
    displa = 0,				/* File transfer display on/off */
    stdouf = 0,				/* Flag for output to stdout */
    xflg   = 0,				/* Flag for X instead of F packet */
    hcflg  = 0,				/* Doing Host command */
    fncnv  = 1,				/* Flag for file name conversion */
    binary = 0,				/* Flag for binary file */
    warn   = 0,				/* Flag for file warning */
    quiet  = 0,				/* Be quiet during file transfer */
    local  = 0,				/* Flag for external tty vs stdout */
    server = 0,				/* Flag for being a server */
    cnflg  = 0,				/* Connect after transaction */
    cxseen = 0,				/* Flag for cancelling a file */
    czseen = 0; 	    	    	/* Flag for cancelling file group */

/* Variables passed from command parser to protocol module */

char sstate  = 0;			/* Starting state for automaton */
char *cmarg  = "";			/* Pointer to command data */
char *cmarg2 = "";			/* Pointer to second data field */
char **cmlist;				/* Pointer to file list in argv */

/* Miscellaneous */

char **xargv;				/* Global copies of argv */
int  xargc;				/* and argc  */

extern char *dftty;			/* Default tty name from ckx???.c */
extern int dfloc;			/* Default location: remote/local */
extern int dfprty;			/* Default parity */
extern int dfflow;			/* Default flow control */


/*  M A I N  --  C-Kermit main program  */

main(argc,argv) int argc; char **argv; {

    char *strcpy();

/* Do some initialization */

    xargc = argc;			/* Make global copies of argc */
    xargv = argv;			/* ...and argv. */
    sstate = 0;				/* No default start state. */
    strcpy(ttname,dftty);		/* Set up default tty name. */
    local = dfloc;			/* And whether it's local or remote. */
    parity = dfprty;			/* Set initial parity, */
    flow = dfflow;			/* and flow control. */
    
/* Look for a UNIX-style command line... */

    if (argc > 1) {			/* Command line arguments? */
	sstate = cmdlin();		/* Yes, parse. */
	if (sstate) {
	    proto();			/* Take any requested action, then */
	    if (!quiet) conoll("");	/* put cursor back at left margin, */
	    if (cnflg) conect();	/* connect if requested, */
	    doexit(0);			/* and then exit with status 0. */
    	}
    }	
    
/* If no action requested on command line, enter interactive parser */

    cmdini();				/* Initialize command parser */
    while(sstate = parser()) {		/* Loop getting commands. */
	if (sstate) proto();		/* Enter protocol if requested. */
    }
}
