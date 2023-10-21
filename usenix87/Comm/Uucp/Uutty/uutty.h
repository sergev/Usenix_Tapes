#ifndef uutty_h
#include "dbg.h"
/* 
** This is the header file for John Chambers' "Serial Port Daemon".
** Several programs use this header, including "uutty", so there 
** is likely to be stuff here not needed by a particular program.
** A single header is used to make them somewhat consistent.
*/
/*
** The following stuff may be specific to Unix SYS/V:
*/
#ifdef   SYS5
#include <pwd.h>
#include <termio.h>
#include <utmp.h>
#include <sys/errno.h>
#include <sys/stat.h>
  extern struct passwd*getpwnam();
  extern int           ttyslot(); 
  extern struct utmp  *getutent(); 
  extern struct utmp  *getutline(); 
  extern void          pututline(); 
  extern void          setutent(); 
  extern struct stat   status;		/* Child process's status */
  extern struct termio trminit;
  extern int           ttyslot(); 
  extern struct utmp  *up;
  extern struct utmp   utmp;
#endif
/*
** Unix library stuff:
*/
extern char **environ;	/* Environment vector */
extern char  *crypt();	/* Password encryption */
extern char  *ctime();	/* Date/time in ASCII */
/*
** State-type codes, see also the state[] array.
*/
#define S_INIT		0
#define S_IDLE		1
#define S_LOGIN		2
#define S_PASSWD	3
#define S_DL		11
#define S_RX		12
#define S_TC		13
#define S_BV_E		14
#define S_HSUQ		15
#define S_HSU		16
#define S_READS		17
#define S_GO		18
#define S_TRANS		19
#define S_CTRLA		20
#define S_CTRLB		21
#define S_CTRLC		22
#define S_CTRLD		22
#define S_CONNECT	33
#define S_ID		34
#define S_SREC		36
#define S_SR		37
#define S_EXIT		38
#define S_LOAD		39
/*
** Array sizes.
*/
#define BUF    80		/* Size of scratch buffer */
#define IBUF 1000		/* Size of input buffer */
#define OBUF   80		/* Size of output buffer */
#define RSP  1000		/* Size of response buffer */
#define SREC   80		/* Size of S-record buffer */
/*
** Assorted pseudo-functions.
*/
#define ASCII(c) ((c)&0x7F)
#define Awrite(m) awrite(m)
#define CTRL(c)  ((c)&0x1F)
#define Dmp(a,s) dmp((long)(a),(long)(s))
#define Pread(c,p,s) pread((int)(c),(char*)(p),(int)(s))
#define Pwrite(p)    pwrite((char*)(p))
#define Resync       resync()
#define Slowly	     if (slowfl) slowly()
/*
** These might be in some standard libraries:
*/
#define islower(c) ('a'<=(c)&&(c)<='z')
#define isupper(c) ('A'<=(c)&&(c)<='Z')
#define htob(c) ((('0'<=c)&&(c<='9'))?(c-'0'):\
				 (('A'<=c)&&(c<='F'))?(c-'A'+10):\
				 (('a'<=c)&&(c<='f'))?(c-'a'+10):-1)
/*
** Limit type codes:
*/
#define L_READS		10
#define L_ES		2
#define L_TC		3
#define L_DL		4
#define L_LOAD		4
#define L_RX		5
#define L_START		6
#define L_TRIES     	10
/*
** Misc constants:
*/
#define READS      3	/* Read attempts before failure */
#define NUDGES     5	/* Nudge attempts before failure */
#define EGARBAGE 101
#define ETIMEOUT 102
#define THRESH     3	/* Number of failures that elicits prompt */
/*
** Sleep times for various routines.
*/
#define SLEEP1	1
#define SLEEP2	1
#define SLEEP3	1
#define SLEEPR	1
#define SLEEPF	10
#define SLEEPL	10
/*
** Pseudo-commands.
*/
#define Dead		goto dead
#define Response	goto response
#define Nudge		goto nudge
/*
** Pseudo-types.
*/
#define uint unsigned int
/*
** Assorted messages.
*/
extern char m_CTRLA [];
extern char*m_exit;
extern char m_login [];
extern char m_passwd[];
extern char*m_init;
extern char*m_init1;
extern char*m_init2;
extern char*m_init3;
extern char*m_nudge;
/*
** Assorted global function typess.
*/
extern char        * ctime();
extern char        * gestate();
extern char        * getime();
extern char        * lastfield();
extern struct utmp * findutmp();
/*
** Assorted global variables.
*/
extern uint  count;		/* Delay between "typed" chars */
extern char *ctime();		/* Time in ASCII format */
extern long  currtime;		/* Timestamp */
extern char *ctimep;		/* Current ctime() value */
extern char *device;		/* Name of serial port */
extern int   dev;		/* Number of serial port file */
extern char *devfld;		/* Last field of device name */
extern Flag  echofl;		/* True if input is to be echoed */
extern Flag  echoing;		/* True if input is being echoed */
extern int   eol0;		/* Alternate EOL character */
extern int   exitstat;		/* Status to report to exit() */
extern int   files;		/* Number of files processed */
extern Flag  forkfl;
extern char  ibuf[IBUF];	/* Buffer for input from port */
extern char *ibfa;		/* First valid char of ibuf */
extern char *ibfz;		/* Last+1 valid char of ibuf */
extern int   iomask;		/* Constant to convert values to ASCII */
extern int   l_tries;		/* Number of reads between nudges */
extern int   l_reads;		/* Number of reads before timeout */
extern Flag  locked;		/* True if lockfile exists*/
extern Flag  lockfl;		/* Create lockfile on login */
extern int   lockfn;		/* Lockfile number */
extern char  lockfile[50];	/* Place to build lockfile name */
extern char  lockroot[];	/* Pathname for creating lockfiles */
extern int   nudges;		/* Number of times we've nudged */
extern int   Nudges;		/* Limit to nudge attempts */
extern int   lsleep;		/* Sleep time for locks */
extern char *oldtarg;		/* Previous name of target */
extern char *path;		/* Default search path */
extern Flag  pathfl;
extern int   pid;		/* Our process id number */
extern char *prgnam;		/* Last field of program's name */
extern Flag  raw;		/* True if I/O is to be raw */
extern char  rsp[1+RSP];	/* Response buffer */
extern uint  rspmax;		/* Max response we can handle */
extern uint  rspsiz;		/* Current size of response buffer */
extern Flag  slow;		/* True if output should be slow */
extern Flag  slowfl;		/* True if count or slow turned on */
extern int   ss;		/* State: code for last action */
extern char *target;		/* Name of thing on other side of port */
extern Flag  termfl;		/* True if terminal state changed */
extern struct termio trminit;	/* Initial ioctl() setting for device */
extern uint  timeout;		/* Global timeout interval */
/*
** User identification stuff:
*/
#define PASSWD 16		/* Max length of a passwd */
#define USERID 16		/* Max length of a userid */
extern char  passwd[1+PASSWD];	/* Current passwd */
extern char  userid[1+USERID];	/* Current userid */
extern int   euid;		/* Effective user id number */
extern int   ruid;		/* Real user id number */
extern int   egid;		/* Effective group id number */
extern int   rgid;		/* Real group id number */
/*
** Definitions of non-int-valued functions:
*/
struct utmp *findutmp();
struct utmp *findutmp();
       char *getime();
       char *lastfield();
/*
** Some common gotos:
*/
#define Done goto done
#define Fail goto fail
#define Loop goto loop

#ifdef SYS5
#endif

#define uutty_h
#endif
