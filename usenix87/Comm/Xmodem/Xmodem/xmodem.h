#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sgtty.h>
#include <signal.h>

/* define macros to print messages in log file */
#define  logit(string) if(LOGFLAG)fprintf(LOGFP,string)
#define  logitarg(string,argument) if(LOGFLAG)fprintf(LOGFP,string,argument)

#define	     VERSION	32	/* Version Number */
#define      FALSE      0
#define      TRUE       1


/*  ASCII Constants  */
#define      SOH  	001 
#define	     STX	002
#define	     ETX	003
#define      EOT	004
#define	     ENQ	005
#define      ACK  	006
#define	     LF		012   /* Unix LF/NL */
#define	     CR		015  
#define      NAK  	025
#define	     SYN	026
#define	     CAN	030
#define	     ESC	033

/*  XMODEM Constants  */
#define      TIMEOUT  	-1
#define      ERRORMAX  	10    /* maximum errors tolerated */
#define      NAKMAX	2     /* maximum times to wait for initial NAK when sending */
#define      RETRYMAX  	5     /* maximum retries to be made */
#define      CRCSWMAX	4     /* maximum time to try CRC mode before switching */
#define      KSWMAX	5     /* maximum errors before switching to 128 byte packets */
#define      SLEEPNUM	100   /* target number of characters to collect during sleepy time */
#define	     BBUFSIZ	1024  /* buffer size */
#define      NAMSIZ	11    /* length of a CP/M file name string */
#define	     CTRLZ	032   /* CP/M EOF for text (usually!) */
#define      CRCCHR	'C'   /* CRC request character */
#define      BAD_NAME	'u'   /* Bad filename indicator */

#define      CREATMODE	0644  /* mode for created files */

/* GLOBAL VARIABLES */

int ttyspeed;		/* tty speed (bits per second) */
unsigned char buff[BBUFSIZ];	/* buffer for data */
int nbchr;		/* number of chars read so far for buffered read */
char filename[256];	/* place to construct filenames */
FILE *LOGFP;		/* descriptor for LOG file */

/* option flags and state variables */
char	XMITTYPE;	/* text or binary? */
int	DEBUG;		/* keep debugging info in log? */
int	RECVFLAG;	/* receive? */
int	SENDFLAG;	/* send? */
int	BATCH;		/* batch? (Now used as a state variable) */
int	CRCMODE;	/* CRC or checksums? */
int	DELFLAG;	/* don't delete old log file? */
int	LOGFLAG;	/* keep log? */
int	LONGPACK; 	/* do not use long packets on transmit? */
int	MDM7BAT;	/* MODEM7 batch protocol */
int	YMDMBAT;	/* YMODEM batch protocol */


/*   CRC-16 constants.  From Usenet contribution by Mark G. Mendel, 
     Network Systems Corp.  (ihnp4!umn-cs!hyper!mark)
*/

    /* the CRC polynomial. */
#define	P	0x1021

    /* number of bits in CRC */
#define W	16

    /* this the number of bits per char */
#define B	8
