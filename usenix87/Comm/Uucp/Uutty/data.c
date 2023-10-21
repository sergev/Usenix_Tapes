#include "uutty.h"
#include <signal.h>

/*
** Assorted messages.
*/
char*m_exit = 0;			/* May be set to special exit message */
char m_login [] = "\r\nlogin:";		/* Unix login prompt */
char m_passwd[] = "\r\npassword:";	/* Unix password prompt */
char*m_init     = 0;			/* Special init string for device */
char*m_init1    = 0;			/* May be set to special init message */
char*m_init2    = "\r";			/* Normal nudge to get first response */
char*m_init3    = 0;			/* May be set to special init message */
char*m_nudge    = "\r";			/* What to send to trigger a response */
/*
** Assorted global variables.
*/
int   baudmask = 0;	/* CBAUD mask for termio ioctl (B1200, B9600, etc) */
int   baudrate = 0;	/* Numeric value of baud rate (1200, 9600, etc) */
uint  count  = 0;	/* Delay between "typed" chars */
long  currtime;		/* Timestamp */
char *ctimep = "?";	/* ASCII form of currtime */
char *device = 0;	/* Name of serial port */
int   dev    = 0;	/* Number of serial port */
char *devfld= "?";	/* Last field of device name */
Flag  echofl = 0;	/* True if input is to be echoed */
Flag  echoing= 0;	/* True if input is being echoed */
int   eol0   = -1;	/* Extra EOL char to recognize */
int   exitstat = 0;
int   files  = 0;
char  ibuf[IBUF];	/* Buffer for input from port */
char *ibfa   = ibuf;	/* First valid char of ibuf */
char *ibfz   = ibuf;	/* Last+1 valid char of ibuf */
int   iomask = 0x7F;	/* Converts chars to ASCII */
int   l_tries= L_TRIES;	/* Number of reads between nudges */
int   l_reads= L_READS;	/* Number of reads before timeout */
int   lsleep = 30;	/* Sleep time when we hit a lockfile */
Flag  locked = 0;	/* Lockfile created */
Flag  lockfl = 0;	/* Create lockfile on login */
int   lockfn = -1;	/* Lockfile number */
char  lockfile[50] = {0};	/* Place to build lockfile name */
char  lockroot[] = "/usr/spool/uucp/LCK..";
int   nudges = 0;	/* Number of times we've nudged */
int   Nudges = NUDGES;	/* Limit to nudge attempts */
char *oldtarg = "?";	/* Previous name for thing on other side of port */
int   pid    = -1;	/* Our process id number */
char *prgnam = "?";	/* Last field of program's name */
Flag  raw    = 1;	/* Raw I/O on device? */
char  rsp[1+RSP];	/* Response buffer */
uint  rspmax = RSP;	/* Max response we can handle */
Flag  slow   = 0;
Flag  slowfl = 0;	/* True if count or slow turned on */
int   ss     = S_INIT;	/* State: code for last action */
char *target  = "port";	/* Name of thing on other side of port */
Flag  termfl  = 0;	/* True if terminal state changed */
/*
** User identification stuff:
*/
char  passwd[1+PASSWD] = {0};	/* Current passwd */
char  userid[1+USERID] = {0};	/* Current userid */
int   euid    = -1;		/* Effective user id number */
int   ruid    = -1;		/* Real user id number */
int   egid    = -1;		/* Effective group id number */
int   rgid    = -1;		/* Real group id number */

#ifdef SYS5
	struct stat   status;		/* Child process's status */
	struct termio trminit  = {0};	/* For saving terminal (ioctl) status */
	struct utmp  *up = 0;		/* Pointer to /etc/utmp entry */
	struct utmp   utmp = {0};	/* Scratch /etc/utmp entry */
#endif

