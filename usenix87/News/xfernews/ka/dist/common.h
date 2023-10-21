#define USG 1

#include <stdio.h>
#include <sys/types.h>
#include "dir.h"
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#ifdef USG
#include <fcntl.h>
#endif

#define FNLEN	15	/* max file name length (including nul) */
#define PATHLEN	100	/* max path name length */
#define DESTLEN 256	/* max length of mail destination */
#define MAXARGS	200	/* max number of args to uucp */
#define SETIN	01	/* flag to run:  reset stdin */
#define RNEWS	"/usr/bin/rnews"
#define RMAIL	"/bin/rmail"
#define UUCP	"/usr/bin/uucp"
#define MAILCMD	"/bin/mail usenet"
#define RECVLOCK "/usr/tmp/recv.lock"
#define MINACK (10 * 12)
#define NETNEWS	72	/* netnews user id */


#ifndef USG
#define strchr index
#endif

/* routine to determine if a process exists */
#define procexists(pid)	(kill(pid, 0) >= 0 || errno == EPERM)

long time(), atol() ;
char *strcpy(), *strchr() ;
char *malloc() ;
FILE *popen() ;
int comp() ;

extern int errno ;
