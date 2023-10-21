#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	"sym.h"

extern IOPTR	inout();
extern int	chkword();
extern int	chksym();
extern TREPTR	term();
extern TREPTR	makelist();
extern TREPTR	list();
extern REGPTR	syncase();
extern TREPTR	item();
extern int	skipnl();
extern int	prsym();
extern int	synbad();


/* ========	command line decoding	========*/




TREPTR	makefork(flgs, i)
int		flgs;
TREPTR		i;
{
	register TREPTR	t;

	t=getstak(FORKTYPE);
	t->forktyp=flgs|TFORK; 
	t->forktre=i; 
	t->forkio=0;
	return(t);
	}

static TREPTR	makelist(type,i,r)
int		type;
TREPTR		i, r;
{
	register TREPTR	t;

	if( i==0 || r==0 ){	
		synbad();
		} 
	else {	
		t = getstak(LSTTYPE);
		t->lsttyp = type;
		t->lstlef = i; 
		t->lstrit = r;
		}
	return(t);
	}

/*
 * cmd
 *	empty
 *	list
 *	list & [ cmd ]
 *	list [ ; cmd ]
 */

TREPTR	cmd(sym,flg)
register int		sym;
int		flg;
{
	register TREPTR	i, e;

	i = list(flg);

	if( wdval=='\n' ){	
		if( flg&NLFLG ){	
			wdval=';'; 
			chkpr('\n');
			}
		} 
	else if ( i==0 && (flg&MTFLG)==0 ){	
		synbad();
		}

	switch( wdval ){

	case '&':
		if( i ){	
			i = makefork(FINT|FPRS|FAMP, i);
			} 
		else {	
			synbad();
			}

	case ';':
		if( e=cmd(sym,flg|MTFLG) ){	
			i=makelist(TLST, i, e);
			}
		break;

	case EOFSYM:
		if( sym=='\n' ){	
			break;
			}

	default:
		if( sym ){	
			chksym(sym);
			}

		}
	return(i);
	}

/*
 * list
 *	term
 *	list && term
 *	list || term
 */

static TREPTR	list(flg)
{
	register TREPTR	r;
	register int		b;

	r = term(flg);
	while( r && ((b=(wdval==ANDFSYM)) || wdval==ORFSYM) ){	
		r = makelist((b ? TAND : TORF), r, term(NLFLG));
		;
		}
	return(r);
	}

/*
 * term
 *	item
 *	item |^ term
 */

static TREPTR	term(flg)
{
	register TREPTR	t;

	reserv++;
	if( flg&NLFLG ){	
		skipnl();
		} 
	else {	
		word();
		}

	if( (t=item(TRUE)) && (wdval=='^' || wdval=='|') ){	
		return(makelist(TFIL, makefork(FPOU,t), makefork(FPIN|FPCL,term(NLFLG))));
		} 
	else {	
		return(t);
		}
	}

static REGPTR	syncase(esym)
register int	esym;
{
	skipnl();
	if( wdval==esym ){	
		return(0);
		} 
	else {	
		register REGPTR	r=getstak(REGTYPE);
		r->regptr=0;
		for(;;){ 
			wdarg->argnxt=r->regptr;
			r->regptr=wdarg;
			if( wdval || ( word()!=')' && wdval!='|' ) ){ 
				synbad();
				}
			if( wdval=='|' ){ 
				word();
				} 
			else { 
				break;
				}
			}
		r->regcom=cmd(0,NLFLG|MTFLG);
		if( wdval==ECSYM ){	
			r->regnxt=syncase(esym);
			} 
		else {	
			chksym(esym);
			r->regnxt=0;
			}
		return(r);
		}
	}

/*
 * item
 *
 *	( cmd ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	if ... then ... else ... fi
 *	for ... while ... do ... done
 *	case ... in ... esac
 *	begin ... end
 */

static TREPTR	item(flag)
char		flag;
{
	register TREPTR	t;
	register IOPTR	io;

	if( flag ){	
		io=inout((IOPTR)0);
		} 
	else {	
		io=0;
		}

	switch( wdval ){

	case CASYM:
		{
			t=getstak(SWTYPE);
			chkword();
			t->swarg=wdarg->argval;
			skipnl(); 
			chksym(INSYM|BRSYM);
			t->swlst=syncase(wdval==INSYM?ESSYM:KTSYM);
			t->swtyp=TSW;
			break;
			}

	case IFSYM:
		{
			register int	w;
			t=getstak(IFTYPE);
			t->iftyp=TIF;
			t->iftre=cmd(THSYM,NLFLG);
			t->thtre=cmd(ELSYM|FISYM|EFSYM,NLFLG);
			t->eltre=((w=wdval)==ELSYM ? cmd(FISYM,NLFLG) : (w==EFSYM ? (wdval=IFSYM, item(0)) : 0));
			if( w==EFSYM )
				return(t);
			break;
			}

	case FORSYM:
		{
			t=getstak(FORTYPE);
			t->fortyp=TFOR;
			t->forlst=0;
			chkword();
			t->fornam=wdarg->argval;
			if( skipnl()==INSYM ){	
				chkword();
				t->forlst=item(0);
				if( wdval!='\n' && wdval!=';' ){	
					synbad();
					}
				chkpr(wdval); 
				skipnl();
				}
			chksym(DOSYM|BRSYM);
			t->fortre=cmd(wdval==DOSYM?ODSYM:KTSYM,NLFLG);
			break;
			}

	case WHSYM:
	case UNSYM:
		{
			t=getstak(WHTYPE);
			t->whtyp=(wdval==WHSYM ? TWH : TUN);
			t->whtre = cmd(DOSYM,NLFLG);
			t->dotre = cmd(ODSYM,NLFLG);
			break;
			}

	case BRSYM:
		t=cmd(KTSYM,NLFLG);
		break;

	case '(':
		{
			register PARPTR	 p;
			p=getstak(PARTYPE);
			p->partre=cmd(')',NLFLG);
			p->partyp=TPAR;
			t=makefork(0,p);
			break;
			}

	default:
		if( io==0 ){	
			return(0);
			}

	case 0:
		{
			register ARGPTR	argp;
			register ARGPTR	*argtail;
			register ARGPTR	*argset=0;
			int		keywd=1;
			t=getstak(COMTYPE);
			t->comio=io; /*initial io chain*/
			argtail = &(t->comarg);
			while( wdval==0 ){	
				argp = wdarg;
				if( wdset && keywd ){	
					argp->argnxt=argset; 
					argset=argp;
					} 
				else {	
					*argtail=argp; 
					argtail = &(argp->argnxt); 
					keywd=flags&keyflg;
					}
				word();
				if( flag ){ 
					t->comio=inout(t->comio);
					}
				;
				}

			t->comtyp=TCOM; 
			t->comset=argset; 
			*argtail=0;
			return(t);
			}

		}
	reserv++; 
	word();
	if( io=inout(io) ){	
		t=makefork(0,t); 
		t->treio=io;
		}
	return(t);
	}


static int	skipnl()
{
	while( (reserv++, word()=='\n') ){ 
		chkpr('\n') ;
		}
	return(wdval);
	}

static IOPTR	inout(lastio)
IOPTR		lastio;
{
	register int		iof;
	register IOPTR	iop;
	register char	c;

	iof=wdnum;

	switch( wdval ){

	case DOCSYM:
		iof |= IODOC; 
		break;

	case APPSYM:
	case '>':
		if( wdnum==0 )
			iof |= 1;
		iof |= IOPUT;
		if( wdval==APPSYM ){	
			iof |= IOAPP; 
			break;
			}

	case '<':
		if( (c=nextc(0))=='&' ){	
			iof |= IOMOV;
			} 
		else if ( c=='>' ){	
			iof |= IORDW;
			} 
		else {	
			peekc=c|MARK;
			}
		break;

	default:
		return(lastio);
		}

	chkword();
	iop=getstak(IOTYPE); 
	iop->ioname=wdarg->argval; 
	iop->iofile=iof;
	if( iof&IODOC ){ 
		iop->iolst=iopend; 
		iopend=iop;
		}
	word(); 
	iop->ionxt=inout(lastio);
	return(iop);
	}

static int	chkword()
{
	if( word() ){	
		synbad();
		}
	}

static int	chksym(sym)
{
	register int		x = sym&wdval;
	if( ((x&SYMFLG) ? x : sym) != wdval ){	
		synbad();
		}
	}

static int	prsym(sym)
{
	if( sym&SYMFLG ){	
		register SYSPTR	sp=reserved;
		while( sp->sysval
		    && sp->sysval!=sym ){ 
			sp++ ;
			}
		prs(sp->sysnam);
		} 
	else if ( sym==EOFSYM ){	
		prs(endoffile);
		} 
	else {	
		if( sym&SYMREP )
			prc(sym);
		if( sym=='\n' ){	
			prs("newline");
			} 
		else {	
			prc(sym);
			}
		}
	}

static int	synbad()
{
	prp(); 
	prs(synmsg);
	if( (flags&ttyflg)==0 ){	
		prs(atline); 
		prn(standin->flin);
		}
	prs(colon);
	prc(LQ);
	if( wdval ){	
		prsym(wdval);
		} 
	else {	
		prs(wdarg->argval);
		}
	prc(RQ); 
	prs(unexpected);
	newline();
	exitsh(SYNBAD);
	}
