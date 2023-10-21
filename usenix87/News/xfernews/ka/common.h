#define USG 1

#include <stdio.h>
#include <sys/types.h>
#ifdef USG
#define u_short ushort
#endif
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
#define RNEWS	"/usr/bin/rnews"
#define RMAIL	"/bin/rmail"
#define UUCP	"/usr/bin/uucp"
#define MAILCMD	"/bin/mail usenet"
#define RECVLOCK "/tmp/recv.lock"
#define LIBDIR	"/usr/lib/news"
#define MAXBATSIZE 66000L	/* approximate max size of batch */
#define MAXBATCH 6	/* max # batches outstanding */
#define MINACK (10 * 12)
#define NETNEWS	72	/* netnews user id */
#define ENDMARK 0203	/* char to mark end of file in batch format 'a' */

struct arglist {
      int nargs ;
      char *arg[MAXARGS] ;
} ;

#ifdef USG
#define index strchr
#endif

/* routine to determine if a process exists */
#define procexists(pid)	(kill(pid, 0) >= 0 || errno == EPERM)
#define equal(s1, s2) (strcmp(s1, s2) == 0)

#ifdef USG
#include <setjmp.h>
#define setexit() setjmp(nextdir)
#define reset() longjmp(nextdir, 1)
#ifdef RECVNEWS
jmp_buf nextdir ;	/* label to jump to on major error */
#endif
#endif

long time(), atol() ;
char *strcpy(), *index() ;
char *malloc() ;
FILE *popen() ;
int comp() ;
FILE *ckopen() ;
long fsize() ;

extern int errno ;
