/*
 * This program handles the posting or mailing of replies and followups
 * for vnews.  It can also be invoked directly.
 *
 * Copyright 1984 by Kenneth Almquist.  All rights reserved.
 *     Permission is granted for anyone to use and distribute, but not
 *     sell, this program provided that this copyright notice is retained.
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>	/* for .signature kludge */
#include <signal.h>
#include "config.h"
#include "newsdefs.h"
#include "libextern.h"
#include "str.h"
#include "time.h"

extern char *scanp;		/* string scanning pointer */

#define MODGROUPS	"fa.all,mod.all,all.mod,all.announce"

/* types of header fields */
#define HTTO 0
#define HTCC 1
#define HTBCC 2
#define HTNEWSGROUPS 3
#define HTINREPLYTO 5
#define HTREFERENCES 6
#define HTSUBJECT 7
#define HTEXPIRES 8
#define HTFROM 9
#define HTREPLYTO 10
#define HTFOLLOWUPTO 11
#define HTDIST 12
#define HTKEYWORDS 14
#define HTCONTROL 15
#define HTORGANIZATION 16
#define HTSUMMARY 17
#define HTAPPROVED 18
#define HTCOMMENTS 19
#define HTMESSAGEID 20
#define HTCOMMAND 21
#define HTUNKNOWN 27


struct hline {
      struct hline *next;
      short type;
      short linno;
      char *text;
      char *hdrname;
      char *body;
      char *fixed;
};

#ifndef EXTERN
#define EXTERN extern
#define INIT(val)
#else
#define INIT(val) = val
#endif

EXTERN struct hline *hfirst, *hlast;		/* list of header lines */
EXTERN struct hline *hdrline[HTCOMMAND + 1];	/* indexed by type */
EXTERN struct hline *curhdr;			/* current header line */


#define MAXADDR 100
EXTERN char *addrlist[MAXADDR + 1];	/* mail destinations */
EXTERN char **addrp INIT(addrlist);	/* next entry in addrlist */
EXTERN char *moderator;			/* address of moderator */
EXTERN char *references;		/* specified on command line */

#include "setjmp.h"

#define setexit() setjmp(syntaxjmp)
#define reset() longjmp(syntaxjmp, 1)

EXTERN jmp_buf syntaxjmp;		/* jump here on syntax errors */

EXTERN int nerrors;			/* number of errors */
EXTERN int linno;			/* input line number */
EXTERN FILE *infp INIT(stdin);		/* input file */
EXTERN FILE *newsfp, *mailfp;		/* temp files */
EXTERN char mailtemp[32], newstemp[32];	/* temp files */
#define MAXGRPS 10
EXTERN char *nglist[MAXGRPS + 1];	/* entries on Newsgroup line */
EXTERN char *follist[MAXGRPS + 1];	/* entries on Followup-To */
EXTERN char *nlist[MAXGRPS + 1];	/* temporary */
EXTERN int debug;			/* don't actuall post message */
EXTERN int verbose;			/* verbose flag */
EXTERN int background INIT(1);		/* run mail/inews in background */
EXTERN char *outp;			/* for generating output lines */
EXTERN int signit;			/* if set, append .signiture file */
EXTERN char signfile[FPATHLEN];		/* .signiture file */
EXTERN int signmode;			/* .signature kludge */
EXTERN char *envkludge[200];		/* organization kludge */


long atol();
FILE *ckfopen();
time_t cgtdate();
char *ckmalloc();

char *getval();
