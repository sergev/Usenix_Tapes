/*
 * rparams.h - parameters for readnews, rfuncs, and readr.
 */

/* static char *Rparams = "@(#)rparams.h	2.8	5/28/83"; */

#include "config.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#ifdef SIGXCPU
/* 4.2BSD moved this file, grumble grumble */
#include <sys/time.h>
#else
#include <time.h>
#endif
#include "defs.h"
#include "arthead.h"
#include "libextern.h"
#include "roptions.h"

#define	NEXT	0
#define SPEC	1

#define	FORWARD	0
#define BACKWARD 1

/* external declarations stolen from params.h */
#ifndef SHELL
extern char	*SHELL;
#endif

/* external function declarations */
extern	char	*strcpy(), *strncpy(), *strcat(), *index(), *rindex();
extern	char	*ctime(), *mktemp(), *malloc(), *realloc(), *getenv();
extern	char	*arpadate(), *dirname();
extern	time_t	time(), getdate(), cgtdate();
extern	int	broadcast(), save(), newssave(), ushell(), pshell(), onsig();
extern	long	atol();

char *savestr();

/* external declarations specific to readnews */
extern int	mode;
extern time_t	atime;
