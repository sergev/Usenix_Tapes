/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"



/* ========	error handling	======== */

exitset()
	{
	assnum(&exitadr,exitval);
	}

sigchk()
	{
	/* Find out if it is time to go away.
		 * `trapnote' is set to SIGSET when fault is seen and
		 * no trap has been set.
		 */
	if( trapnote&SIGSET ){	
		exitsh(SIGFAIL);
		;
		}
	}

failed(s1,s2)
	char	*s1, *s2;
	{
char *cp;
for(cp=s1; *cp; cp++)
debug("%3o ",*cp);
debug("\n");
	prp(); 
	prs(s1); 
	if( s2 ){	
		prs(colon); 
		prs(s2);
		;
		}
	newline(); 
	exitsh(ERROR);
	}

error(s)
char*	s;
{
	failed(s,NIL);
	}

exitsh(xno)
int	xno;
{
	/* Arrive here from `FATAL' errors
		 *  a) exit command,
		 *  b) default trap,
		 *  c) fault with no trap set.
		 *
	 * Action is to return to command level or exit.
		 */
	exitval=xno;
	if( (flags & (forked|errflg|ttyflg)) != ttyflg ){	
		done();
		} 
	else {	
		clearup();
		longjmp(errshell,1);
		;
		}
	}

done()
{
	register char*	t;
	if( t=trapcom[0] ){	
		trapcom[0]=0; /*should free but not long */
		execexp(t,0);
		;
		}
	rmtemp(0);
	exit(exitval);
	}

rmtemp(base)
IOPTR		base;
{
	while( iotemp>base ){  
		unlink(iotemp->ioname);
		iotemp=iotemp->iolst;
		;
		}
	}
