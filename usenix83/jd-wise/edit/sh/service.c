#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"


extern int	gsort();

#define ARGMK	01

int		errno;
char*		sysmsg[];

/* fault handling */
#define ENOMEM	12
#define ENOEXEC 8
#define E2BIG	7
#define ENOENT	2
#define ETXTBSY 26



/* service routines for `execute' */

int	initio(iop)
IOPTR		iop;
{
	register char*	ion;
	register int		iof, fd;

	if( iop ){	
		iof=iop->iofile;
		ion=mactrim(iop->ioname);
		if( *ion && (flags&noexec)==0 ){	
			if( iof&IODOC ){	
				subst(chkopen(ion),(fd=tmpfil()));
				close(fd); 
				fd=chkopen(tmpout); 
				unlink(tmpout);
				} 
			else if ( iof&IOMOV ){	
				if( eq(minus,ion) ){	
					fd = -1;
					close(iof&IOUFD);
					} 
				else if ( (fd=stoi(ion))>=USERIO ){	
					failed(ion,badfile);
					} 
				else {	
					fd=dup(fd);
					;
					}
				} 
			else if ( (iof&IOPUT)==0 ){	
				fd=chkopen(ion);
				} 
			else if ( flags&rshflg ){	
				failed(ion,restricted);
				} 
			else if ( iof&IOAPP && (fd=open(ion,1))>=0 ){	
				lseek(fd, 0L, 2);
				} 
			else {	
				fd=create(ion);
				;
				}
			if( fd>=0 ){	
				rename(fd,iof&IOUFD);
				;
				}
			;
			}
		initio(iop->ionxt);
		;
		}
	}

char*	getpath(s)
char*		s;
{
	register char*	path;
	if( any('/',s) ){	
		if( flags&rshflg ){	
			failed(s, restricted);
			} 
		else {	
			return(nullstr);
			;
			}
		} 
	else if ( (path = pathnod.namval)==0 ){	
		return(defpath);
		} 
	else {	
		return(cpystak(path));
		;
		}
	}

pathopen(path, name)
register char	*path, *name;
{
	register UFD		f;

	do{ 
		path=catpath(path,name);
		}
	while( (f=open(curstak(),0))<0 && path );
	return(f);
	}

char*	catpath(path,name)
register char*	path;
char*		name;
{
	/* leaves result on top of stack */
	register char	*scanp = path,
			*argp = locstak();

	while( *scanp && *scanp!=':' ){ 
		*argp++ = *scanp++ ;
		}
	if( scanp!=path ){ 
		*argp++='/' ;
		}
	if( *scanp==':' ){ 
		scanp++ ;
		}
	path=(*scanp ? scanp : 0); 
	scanp=name;
	while( (*argp++ = *scanp++) );
	return(path);
	}

static char*	xecmsg;
static char*	*xecenv;

execa(at)
	char*		at[];
	{
	register char*	path;
	register char*	*t = at;

	if( (flags&noexec)==0 ){	
		xecmsg=notfound; 
		path=getpath(*t);
		namscan(exname);
		xecenv=setenv();
		while( path=execs(path,t) );
		failed(*t,xecmsg);
		}
	}

static char*
execs(ap,t)
	char*		ap;
	register char*	t[];
	{
	register char	*p, *prefix;

	prefix=catpath(ap,t[0]);
	trim(p=curstak());

	sigchk();
	execve(p, &t[0] ,xecenv);
	switch( errno ){

	case ENOEXEC:
		flags=0;
		comdiv=0; 
		ioset=0;
		clearup(); /* remove open files and for loop junk */
		if( input ){ 
			close(input) ;
			}
		close(output); 
		output=2;
		input=chkopen(p);

		/* set up new args */
		setargs(t);
		longjmp(subshell,1);

	case ENOMEM:
		failed(p,toobig);

	case E2BIG:
		failed(p,arglist);

	case ETXTBSY:
		failed(p,txtbsy);

	default:
		xecmsg=badexec;
	case ENOENT:
		return(prefix);
		}
	}

/* for processes to be waited for */
#define MAXP 20
static int	pwlist[MAXP];
static int	pwc;

postclr()
{
	register int		*pw = pwlist;

	while( pw <= &pwlist[pwc] ){ 
		*pw++ = 0 ;
		}
	pwc=0;
	}

int	post(pcsid)
int		pcsid;
{
	register int		*pw = pwlist;

	if( pcsid ){	
		while( *pw ){ 
			pw++ ;
			}
		if( pwc >= MAXP-1 ){	
			pw--;
			} 
		else {	
			pwc++;
			;
			}
		*pw = pcsid;
		;
		}
	}

int	await(i)
int		i;
{
	int		rc=0, wx=0;
	int		w;
	int		ipwc = pwc;

	post(i);
	while( pwc ){	
		register int		p;
		register int		sig;
		int		w_hi;

		{
			register int	*pw=pwlist;
			p=wait(&w);
			while( pw <= &pwlist[ipwc] ){ 
				if( *pw==p ){ 
					*pw=0; 
					pwc--;
					} 
				else { 
					pw++;
					;
					}
				;
				}
			}

		if( p == -1 ){ 
			continue ;
			}

		w_hi = (w>>8)&LOBYTE;

		if( sig = w&0177 ){	
			if( sig == 0177	/* ptrace! return */ ){	
				prs("ptrace: ");
				sig = w_hi;
				;
				}
			if( sysmsg[sig] ){	
				if( i!=p || (flags&prompt)==0 ){ 
					prp(); 
					prn(p); 
					blank() ;
					}
				prs(sysmsg[sig]);
				if( w&0200 ){ 
					prs(coredump) ;
					}
				;
				}
			newline();
			;
			}

		if( rc==0 ){	
			rc = (sig ? sig|SIGFLG : w_hi);
			;
			}
		wx |= w;
		;
		}

	if( wx && flags&errflg ){	
		exitsh(rc);
		;
		}
	exitval=rc; 
	exitset();
	}

char		nosubst;

trim(at)
char*		at;
{
	register char*	p;
	register char	c;
	register char	q=0;

	if( p=at ){	
		while( c = *p ){ 
			*p++=c&STRIP; 
			q |= c ;
			}
		;
		}
	nosubst=q&QUOTE;
	}

char*	mactrim(s)
char*		s;
{
	register char*	t=macro(s);
	trim(t);
	return(t);
	}

char*	*scan(argn)
int		argn;
{
	register ARGPTR	argp = Rcheat(gchain)&~ARGMK;
	register char*	*comargn, *comargm;

	comargn=getstak(BYTESPERWORD*argn+BYTESPERWORD); 
	comargm = comargn += argn; 
	*comargn = ENDARGS;

	while( argp ){	
		*--comargn = argp->argval;
		if( argp = argp->argnxt ){ 
			trim(*comargn);
			;
			}
		if( argp==0 || Rcheat(argp)&ARGMK ){	
			gsort(comargn,comargm);
			comargm = comargn;
			;
			}
		/* Lcheat(argp) &= ~ARGMK; */
		argp = Rcheat(argp)&~ARGMK;
		;
		}
	return(comargn);
	}

static
gsort(from,to)
	char		*from[], *to[];
	{
	int		k, m, n;
	register int		i, j;

	if( (n=to-from)<=1 ){ 
		return ;
		}

	for( j=1; j<=n; j*=2 );

	for( m=2*j-1; m/=2; ){  
		k=n-m;
		for( j=0; j<k; j++ ){	
			for( i=j; i>=0; i-=m ){  
				register char* *fromi; 
				fromi = &from[i];
				if( cf(fromi[m],fromi[0])>0 ){ 
					break;
					} 
				else { 
					char* s; 
					s=fromi[m]; 
					fromi[m]=fromi[0]; 
					fromi[0]=s;
					;
					}
				;
				}
			;
			}
		;
		}
	}

/* Argument list generation */

int	getarg(ac)
COMPTR		ac;
{
	register ARGPTR	argp;
	register int		count=0;
	register COMPTR	c;

	if( c=ac ){	
		argp=c->comarg;
		while( argp ){	
			count += split(macro(argp->argval));
			argp=argp->argnxt;
			;
			}
		;
		}
	return(count);
	}

static int	split(s)
register char*	s;
{
	register char*	argp;
	register int		c;
	int		count=0;

	for(;;){	
		sigchk(); 
		argp=locstak()+BYTESPERWORD;
		while( (c = *s++, !any(c,ifsnod.namval) && c) ){ 
			*argp++ = c ;
			}
		if( argp==staktop+BYTESPERWORD ){	
			if( c ){	
				continue;
				} 
			else {	
				return(count);
				;
				}
			} 
		else if ( c==0 ){	
			s--;
			;
			}
		if( c=expand((argp=endstak(argp))->argval,0) ){	
			count += c;
			} 
		else {	/* assign(&fngnod, argp->argval); */
			makearg(argp); 
			count++;
			;
			}
		Lcheat(gchain) |= ARGMK;
		}
	}
