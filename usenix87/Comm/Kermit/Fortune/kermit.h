/*
 *      K e r m i t  File Transfer Utility
 *
 *      Unix Kermit, Columbia University, 1983
 *
 *      usage: kermit [csr][dlbe line baud escapechar] [f1 f2 ...]
 *
 *      where c=connect, s=send [files], r=receive, d=debug,
 *      l=tty line, b=baud rate, e=escape char (decimal ascii code).
 *      For "host" mode Kermit, format is either "kermit r" to
 *      receive files, or "kermit s f1 f2 ..." to send f1 .. fn.
 *
 *      Modfied by: Ronald R. Tribble for interactive use. 3-21-85
 */

#include <stdio.h>
#include <sgtty.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DBUGSW 0
#define MAXPACK         94      /* Maximum packet size */
#define SOH             1       /* Start of header */
#define SP              32      /* ASCII space */
#define DEL             127     /* Delete (rubout) */
#define CTRLD           4
#define BRKCHR          033     /* default break-connection character */
#define MAXTRY          10      /* Times to retry a packet */
#define MYQUOTE         '#'     /* Quote character I will use */
#define MYPAD           0       /* Number of pad characters needed */
#define MYPCHAR         0       /* Padding character I need */
#define MYEOL           '\r'    /* End-Of-Line character I need */
#define MYTIME          5       /* Seconds for time out */
#define MAXTIM          20      /* Maximum timeout interval */
#define MINTIM          2       /* Minumum timeout interval */
#define TRUE            -1
#define FALSE           0

/*
 *      Global Variables
 */

int     size,           /* Size of present data */
n,                      /* Message number */
rpsiz,                  /* Maximum receive packet size */
spsiz,                  /* Maximum send packet size */
pad,                    /* How much padding to send */
timint,                 /* Timeout for foreign host on sends */
numtry,                 /* Times this packet retried */
oldtry,                 /* Times previous packet retried */
fd,                     /* file pointer of file to read/write */
remfd,                  /* file pointer of the host's tty */
remspd,                 /* speed of this tty */
host,                   /* -1 means we're a host-mode kermit */
debug,                  /* -1 means debugging */
speed,                  /* baud rate to run at */
tstart,                 /* start time of i/o */
tstop,
sleep_len;              /* delay in seconds for kermit send */

unsigned long filch;    /* counter for rate of transfer */

char    state,          /* Present state of the automaton */
padchar,                /* Padding character to send */
eol,                    /* End-Of-Line character to send */
escchr,                 /* Connect command escape character */
quote,                  /* Quote character in incoming data */
**filelist,             /* list of files to be sent */
*filnam,                /* current file name */
remtty[128],
recpkt[MAXPACK],        /* Receive packet buffer */
packet[MAXPACK],        /* Packet buffer */
shell[32];              /* unix shell to run */

struct sgttyb
rawmode,                /* host tty "raw" mode */
cookedmode,             /* host tty "normal" mode */
oldmode,                /* remote tty original state */
remttymode;             /* remote tty line "raw" mode */

jmp_buf env;            /* environment ptr for timeout longjump */
