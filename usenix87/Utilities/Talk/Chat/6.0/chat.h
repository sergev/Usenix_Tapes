/*      This file contains all the major variables for the Chat       */
/*	system.		->slb\@bnl.ARPA                                */
/* chat.h :-: 9/19/85 */

#include <stdio.h>			/* All those .h	files! */
#include "defs.h"
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <time.h>

#define	ON	1
#define	OFF	0
#define	OK	0
#define FERROR	0
#define ERR	(-1)
#define	INLEN	(80 * 4)	/* Max number of chars input */

#ifndef VOID
#define	void	int
#endif

struct	stat	sbuf;
struct	logs	lbuf;

extern  char				/* String Function Defs */
	*strcpy(),
	*strncpy(),
	*strcat(),
	*strncat(),
	*strtok(),
#ifdef INDEX
	*rindex(),
	*index();
#else
	*strrchr(),
	*strchr();
#endif INDEX

char	mytty[6],			/* User's tty */
	myname[16],			/* Username */
	mixname[32],			/* Both	uname and handle */
	myfile[64],			/* User	message	file */
	buffer[512];

int	quit(),
	bye(),
	length,				/* Length of ttyname */
	lfile;				/* Descriptor for LOGFILE */

unsigned int	users;		/* No of users in Chat	  */

long	time(),
	lseek();

unsigned alarm();

FILE	*wfd;

void	exit(),perror(),
	_exit(), qsort();

/* Termcap  Stuff */
extern char *BC, *CM, *CL, *tgoto(),
	    *CE, *SO, *SE, *CD,
	    *TI, *TE;
extern int CO, LI;
