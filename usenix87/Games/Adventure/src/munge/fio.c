/*
**		    Copyright (c) 1985	Ken Wellsch
**
**     Permission is hereby granted to all users to possess, use, copy,
**     distribute, and modify the programs and files in this package
**     provided it is not for direct commercial benefit and secondly,
**     that this notice and all modification information be kept and
**     maintained in the package.
**
*/

#include "mdefs.h"
#undef EOF
#undef NULL
#include <stdio.h>

#define MAXPB		60

struct files
{
	char *fnam ;
	int cnt ;
	FILE *fd ;
} ;

static struct files fx[15] =
{
	{ "(stdin)", 0, stdin },
	{ "(stdout)", 0, stdout },
	{ "(stderr)", 0, stderr },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL }
} ;

static char pbuf[MAXPB] ;
static short int pbp = 0 ;

static char inbuf[MAXLINE] ;
static short int ibp = MAXLINE ;

int openf (name)

  char *name ;

{
	FILE *fd ;
	register int unit ;
	register char *s ;
	char *strsav(), *rindex() ;

	if ( ( fd = fopen (name,"r") ) == NULL )
		return (-1) ;
	unit = fileno (fd) ;
	if ( ( s = rindex (name,'/') ) == (char *)0 )
		fx[unit].fnam = strsav (name) ;
	else
		fx[unit].fnam = strsav (&(s[1])) ;
	fx[unit].cnt = 0 ;
	fx[unit].fd = fd ;
	return (unit) ;
}

int closef (unit)
  int unit ;
{
	if ( fx[unit].fd != NULL )
	{
		fx[unit].cnt = 0 ;
		fclose (fx[unit].fd) ;
		fx[unit].fd = NULL ;
	}
	return ;
}

int Getc ()
{
	register int c ;
	static int Eof = 0 ;

	if ( pbp > 0 )
	{
		c = pbuf[pbp--] ;
	}
	else
	{
		if ( ibp >= MAXLINE || inbuf[ibp] == EOS )
		{
			if ( Eof )
				return (-2) ;
			for (;;)
			{
				if ( fgets (inbuf,MAXLINE,fx[inunit].fd) != NULL )
					break ;
				closef (inunit) ;
				if ( ( inunit = pop () ) == ERROR )
				{
					ibp = MAXLINE ;
					Eof = 1 ;
					return (-2) ;
				}
			}
			ibp = 0 ;
			fx[inunit].cnt++ ;
			if (list)
				prlist (inbuf) ;
		}
		c = inbuf[ibp++] ;
	}

	return (c) ;
}

int Ungetc (c)
  int c ;
{
	if ( ++pbp >= MAXPB )
		error ("Ungetc","too many characters (%d) put back!",MAXPB) ;
	pbuf[pbp] = c ;
	return ;
}

int pbstr (s)
  char s[] ;
{
	register int i ;

	for ( i = strlen(s)-1 ; i >= 0 ; i-- )
		Ungetc (s[i]) ;
	return ;
}

int prlist (line)
  char *line ;
{
	printf ("%4d %-8s %s",fx[inunit].cnt,fx[inunit].fnam,line) ;
	return ;
}

/*VARARGS1*/
int error (rnam,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *rnam, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9 ;
{
	if ( inunit >= 0 )
		fprintf (stderr,"%s(%d)[%s]: ",fx[inunit].fnam,
			fx[inunit].cnt,rnam) ;
	else
		fprintf (stderr,"(EOF) %s: ",rnam) ;

	fprintf (stderr,a1,a2,a3,a4,a5,a6,a7,a8,a9) ;
	fprintf (stderr,"\n") ;
	exit (1) ;
}

/*VARARGS1*/
int errout (a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9 ;
{
	fprintf (stderr,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9) ;
	return ;
}

/*VARARGS1*/
int synerr (rnam,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *rnam, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9 ;
{
	if ( inunit >= 0 )
		fprintf (stderr,"%s(%d): ",fx[inunit].fnam,fx[inunit].cnt) ;
	else
		fprintf (stderr,"(EOF) %s: ",rnam) ;

	fprintf (stderr,a1,a2,a3,a4,a5,a6,a7,a8,a9) ;
	fprintf (stderr,"\n") ;
	flushline () ;
	return ;
}
