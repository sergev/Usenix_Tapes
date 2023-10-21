/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */
/*
 * modified for use with edit:
 *	added "-E ppid" where ppid is the process id of the parent
 *		(i.e. the editor)	jdw 3-8-81
 */
int	ppid=0;	/* jdw 3-8-81 */

#include	"defs.h"

extern char* *copyargs();
static DOLPTR	dolh;

char	flagadr[10];

char	flagchar[] = {
	'x',	'n',	'v',	't',	's',	'i',	'e',	'r',	'k',	'u',	0
};
int	flagval[]  = {
	execpr,	noexec,	readpr,	oneflg,	stdflg,	intflg,	errflg,	rshflg,	keyflg,	setflg,	0
};

/* ========	option handling	======== */


options(argc,argv)
	char*		*argv;
	int		argc;
	{
	register char*	cp;
	register char*	*argp=argv;
	register char*	flagc;
	char*		flagp;

	if( argc>1 && *argp[1]=='-' ){	
		cp=argp[1];
		flags &= ~(execpr|readpr);
		while( *++cp ){	
			flagc=flagchar;

			while( *flagc && *flagc != *cp ){ 
				flagc++ ;
				}
			if( *cp == *flagc ){	
				flags |= flagval[flagc-flagchar];
				} 
			else if ( *cp=='c' && argc>2 && comdiv==0 ){	
				comdiv=argp[2];
				argp[1]=argp[0]; 
				argp++; 
				argc--;
				} 
			else if(*cp=='E' && argc>2){	/* jdw 3-8-81 */
				ppid=atoi(argv[2]);
				flags |= intflg;	/* otherwise no prompts */
debug("esh: ppid=%d flags=0%o\n",ppid,flags);
				argp[1]=argp[0];
				argp++;
				argc--;
				}
			else {	
				failed(argv[1],badopt);
				}
			}
		argp[1]=argp[0]; 
		argc--;
		}

	/* set up $- */
	flagc=flagchar;
	flagp=flagadr;
	while( *flagc ){ 
		if( flags&flagval[flagc-flagchar] ){ 
			*flagp++ = *flagc;
			;
			}
		flagc++;
		;
		}
	*flagp++=0;

	return(argc);
	}

int	setargs(argi)
char*		argi[];
{
	/* count args */
	register char*	*argp=argi;
	register int		argn=0;

	while( Rcheat(*argp++)!=ENDARGS ){ 
		argn++ ;
		}

	/* free old ones unless on for loop chain */
	freeargs(dolh);
	dolh=copyargs(argi,argn);	/* sets dolv */
	assnum(&dolladr,dolc=argn-1);
	}

freeargs(blk) DOLPTR		blk;
{
	register char*	*argp;
	register DOLPTR	argr=0;
	register DOLPTR	argblk;

	if( argblk=blk ){	
		argr = argblk->dolnxt;
		if( (--argblk->doluse)==0 ){	
			for( argp=argblk->dolarg; Rcheat(*argp)!=ENDARGS; argp++ ){ 
				free(*argp) ;
				}
			free(argblk);
			;
			}
		;
		}
	return(argr);
	}

static char* *	copyargs(from, n)
char*		from[];
{
	register char* *	np=alloc(sizeof(char* *)*n+3*BYTESPERWORD);
	register char* *	fp=from;
	register char* *	pp=np;

	np->doluse=1;	/* use count */
	np=np->dolarg;
	dolv=np;

	while( n-- ){ 
		*np++ = make(*fp++) ;
		}
	*np++ = ENDARGS;
	return(pp);
	}

clearup()
{
	/* force `for' $* lists to go away */
	while( argfor=freeargs(argfor) );

	/* clean up io files */
	while( pop() );
	}
DOLPTR	useargs()
{
	if( dolh ){	
		dolh->doluse++;
		dolh->dolnxt=argfor;
		return(argfor=dolh);
		} 
	else {	
		return(0);
		;
		}
	}
