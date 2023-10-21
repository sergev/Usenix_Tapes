#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "ndir.h"

#define IMSINIT		"/usr/lib/imsinit"
#define IMSALIAS	"/usr/lib/imsaliases"

#ifndef USG
#define strchr		index
#define strrchr		rindex
#endif  USG

extern int readmsg(), printmsg(), reply(), gomsg(), mailto(), listmsg(),
	forward(), byebye(), help(), setfolder(), readmbox(), savemsg(),
	delmsg(), undelmsg(), nxbyebye(), expunge(), nextmsg(), prevmsg(),
	varops(), foldlist(), aliasops(), unalias();

extern char *location();
extern char *strchr();
extern char *strrchr();
extern char *getenv();
extern FILE *efopen();
extern char *mlocation();
extern char *mflocation();
extern char *calloc();
extern char *realloc();
extern FILE *epopen();
extern char *basename();
extern char *getlogin();
extern struct passwd *getpwnam();
extern struct tm *localtime();

extern char folder[];
extern char cabinet[];
extern char pager[];
extern char sysmbox[];
extern char prompt[];
extern char sender[];
extern char mailbox[];
extern char editor[];
extern char printcmd[];
extern char savefolder[];
extern char autonext[];
extern char autoread[];
extern char askappend[];
extern char lines[];
extern char edforward[];
extern char alicount[];
extern int msg;

#ifdef USG
#define SYSMAILBOX	"/usr/mail/%s"
#else
#define SYSMAILBOX	"/usr/spool/mail/%s"
#define strchr		index
#define strrchr		rindex
#endif USG

#ifndef USG
#include <sgtty.h>
#define TERMIO		sgttyb
#define TIO_GET		TIOCGETP
#define TIO_SET		TIOCSETN
#define TIO_ERASE	sg_erase
#else   USG
#include <termio.h>
#define TERMIO		termio
#define TIO_GET		TCGETA
#define TIO_SET		TCSETAW
#define TIO_ERASE	c_cc[VERASE]
#endif  USG
