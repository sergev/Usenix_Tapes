/*
 *	@(#)ua.h	1.1 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "user.h"
#include "dir.h"
#include "system.h"
#ifdef BSD
#include <sys/time.h>
#else
#include <time.h>
#endif

#define SYSOP	parms.ua_sysop
#define LOG	"Logfile"
#define MOTD	"motd"
#define PASSWD	"userfile"
#define MSGBASE	"msgdir"
#define NEWMSGS	"userind"
#define NEWUSER	"NewMessage"
#define CONFIG	"ua-config"
#define MEMLIST	"members"

extern jmp_buf cmdloop;			/* so intrp() works */
extern int logsig(), quit(), intrp(), thatsall();
extern int doread(), doscan();
extern struct _himsg *readhigh();
extern struct tm *localtime();
extern struct passwd *getpwuid();
extern char *getowner(), *visible(), *mktemp(), *crypt(), *date(), *longdate(), *getenv(), *reads(), *fgets(), *today(), *calloc(), *ua_acl(), *upstr();
extern int errno, __recurse;
extern char conference[];

#define ToLower(x) (isupper(x)?tolower(x):x)	/* not all tolower() work */
#define ToUpper(x) (islower(x)?toupper(x):x)	/* not all toupper() work */
#define uncntrl(x) (x+'@')	/* beware of non-ASCII character sets! */

#ifdef SYS5
#  define SYS3
#endif SYS5

#ifndef SYS3
#  ifdef XENIX3
#    define RIndex(s,c) strrchr(s, c)
#    define Index(s,c) strchr(s, c)
extern char *strrchr(), *strchr();
#  else  XENIX3
#    ifdef XENIX5
#      define RIndex(s,c) strrchr(s, c)
#      define Index(s,c) strchr(s, c)
extern char *strrchr(), *strchr();
#    else  XENIX5
#      define RIndex(s,c) rindex(s,c)
#      define Index(s,c) index(s,c)
extern char *rindex(), *index();
#    endif XENIX5
#  endif XENIX3
#else
#  define RIndex(s,c) strrchr(s, c)
#  define Index(s,c) strchr(s, c)
extern char *strrchr(), *strchr();
#endif

#ifdef BSD
#  define CONFSIZE	32
#else
#  define CONFSIZE	14
#endif BSD

#ifdef XENIX3
#  define XENIX
#  define SYS3
#else  XENIX3
#  ifdef XENIX5
#    define XENIX
#    define SYS3
#    define SYS5	/* these only make a difference in uwho.c */
#  endif XENIX5
#endif XENIX3

struct cmd {
	char c_cmd;	/* command name */
	char *c_desc;	/* short help message */
	int (*c_func)();	/* function to run, passed rest of line */
};
