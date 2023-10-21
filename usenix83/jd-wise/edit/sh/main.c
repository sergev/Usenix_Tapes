/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */
/*
 * modified for use with edit:
 *	appends a '\001' to the end of each prompt
 *	and sends a SIGSHELL to the parent process
 *		jdw 3-8-81
 */
int	ppid;	/* jdw 3-8-81 */
#include	<signal.h>
#ifdef vax
#define SIGSHELL SIGTRAP
#else
#define SIGSHELL SIGFPE
#endif

#include	"defs.h"
#include	"dup.h"
#include	"sym.h"
#include	"timeout.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sgtty.h>

UFD		output = 2;
static char	beenhere = FALSE;
char		tmpout[20] = "/tmp/sh-";
FILEBLK		stdfile;
FILE		standin = &stdfile;
#include	<execargs.h>

extern int	exfile();




main(c, v)
	int		c;
	char*		v[];
	{
	register int		rflag=ttyflg;

debug("entering esh\n");
	/* initialize storage allocation */
	stdsigs();
	setbrk(BRKINCR);
	addblok((unsigned)0);

	/* set names from userenv */
	getenv();

	/* look for restricted */
	/*	if( c>0 && any('r', *v) ){ rflag=0 ;} */

	/* look for options */
	dolc=options(c,v);
	if( dolc<2 ){ 
		flags |= stdflg ;
		}
	if( (flags&stdflg)==0 ){	
		dolc--;
		;
		}
	dolv=v+c-dolc; 
	dolc--;

	/* return here for shell file execution */
	setjmp(subshell);

	/* number of positional parameters */
	assnum(&dolladr,dolc);
	cmdadr=dolv[0];

	/* set pidname */
	assnum(&pidadr, getpid());

	/* set up temp file names */
	settmp();

	/* default ifs */
	dfault(&ifsnod, sptbnl);

	if( (beenhere++)==FALSE ){	/* ? profile */
		if( *cmdadr=='-'
		    && (input=pathopen(nullstr, profile))>=0 ){	
			exfile(rflag); 
			flags &= ~ttyflg;
			;
			}
		if( rflag==0 ){ 
			flags |= rshflg ;
			}

		/* open input file if specified */
		if( comdiv ){	
			estabf(comdiv); 
			input = -1;
			} 
		else {	
			input=((flags&stdflg) ? 0 : chkopen(cmdadr));
			comdiv--;
			;
			}
		} 
	else {	
		*execargs=dolv;	/* for `ps' cmd */
		;
		}

	exfile(0);
	done();
	}

static int	exfile(prof)
char		prof;
{
	register long	mailtime = 0;
	register int		userid;
	struct stat	statb;

	/* move input */
	if( input>0 ){	
		Ldup(input,INIO);
		input=INIO;
		;
		}

	/* move output to safe place */
	if( output==2 ){	
		Ldup(dup(2),OTIO);
		output=OTIO;
		;
		}

	userid=getuid();

	/* decide whether interactive */
	if( (flags&intflg) || ((flags&oneflg)==0 && gtty(output,&statb)==0 && gtty(input,&statb)==0) ){	
		dfault(&ps1nod, (userid?stdprompt:supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg|prompt; 
		ignsig(KILL);
		} 
	else {	
		flags |= prof; 
		flags &= ~prompt;
		;
		}

	if( setjmp(errshell) && prof ){	
		close(input); 
		return;
		;
		}

	/* error return here */
	loopcnt=breakcnt=peekc=0; 
	iopend=0;
	if( input>=0 ){ 
		initf(input) ;
		}

	/* command loop */
	for(;;){	
		tdystak(0);
		stakchk(); /* may reduce sbrk */
		exitset();
		if( (flags&prompt) && standin->fstak==0 && !eof ){	
			if( mailnod.namval
			    && stat(mailnod.namval,&statb)>=0
			    && statb.st_size
			    && (statb.st_mtime != mailtime)
			    && mailtime ){	
				prs(mailmsg);
				}
			mailtime=statb.st_mtime;
			prs(ps1nod.namval); 
			if(ppid){	/* jdw 3-8-81 */
				prc('\001');
debug("sending signal\n");
				kill(ppid,SIGSHELL);
				}
			alarm(TIMEOUT); 
			flags |= waiting;
			}

		trapnote=0; 
		peekc=readc();
		if( eof ){	
			return;
			}
		alarm(0); 
		flags &= ~waiting;
		execute(cmd('\n',MTFLG),0);
		eof |= (flags&oneflg);
		}
	}

chkpr(eor)
char eor;
{
	if( (flags&prompt) && standin->fstak==0 && eor=='\n' ){	
		prs(ps2nod.namval);
		if(ppid){	/* jdw 3-8-81 */
			prc('\001');
			kill(ppid,SIGSHELL);
			}
		}
	}

settmp()
{
	itos(getpid()); 
	serial=0;
	tmpnam=movstr(numbuf,&tmpout[TMPNAM]);
	}

Ldup(fa, fb)
register int		fa, fb;
{
	dup(fa|DUPFLG, fb);
	close(fa);
	ioctl(fb, FIOCLEX, 0);
	}
