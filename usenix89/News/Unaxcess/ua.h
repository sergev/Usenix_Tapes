/*
 *	@(#)ua.h	1.1 (TDI) 7/18/86 21:01:59
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:01:59 by brandon
#     Converted to SCCS 07/18/86 21:01:59
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:01:59 by brandon
#     Converted to SCCS 07/18/86 21:01:59
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
#
# Modification History:
#
#   Ver. 1.1 created 07/18/86 at 21:01:59 by brandon
#     Converted to SCCS 07/18/86 21:01:59
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
#include "sys.h"

#ifndef SIGUSR1
#define SIGUSR1		NSIG-1
#endif

#define SYSOP	parms.ua_sysop
#define LOG	"Logfile"
#define MOTD	"motd"
#define PASSWD	"userfile"
#define MSGBASE	"msgdir"
#define NEWMSGS	"userind"
#define NEWUSER	"NewMessage"
#define CONFIG	"ua-config"

extern jmp_buf cmdloop;			/* so intrp() works */
extern int logsig(), quit(), intrp(), thatsall();
extern int doread(), doscan();
extern struct _himsg *readhigh();
extern struct tm *localtime();
extern struct passwd *getpwuid();
extern char *getowner(), *visible(), *mktemp(), *crypt(), *date(), *longdate(), *getenv(), *gets(), *fgets();
extern int errno, nopause;
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
#else  XENIX3
#  ifdef XENIX5
#    define XENIX
#  endif XENIX5
#endif XENIX3
