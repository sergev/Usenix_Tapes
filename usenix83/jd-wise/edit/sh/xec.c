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

static int	parent;

SYSTAB		commands;



/* ========	command execution	========*/


execute(argt, execflg, pf1, pf2)
TREPTR		argt;
int		*pf1, *pf2;
{
	/* `stakbot' is preserved by this routine */
	register TREPTR	t;
	STKPTR		sav=savstak();

	sigchk();

	if( (t=argt) && execbrk==0 ){	
		register int		treeflgs;
		int		oldexit, type;
		register char*	*com;

		treeflgs = t->tretyp; 
		type = treeflgs&COMMSK;
		oldexit=exitval; 
		exitval=0;

		switch( type ){

		case TCOM:
			{
				char*		a1;
				int		argn, internal;
				ARGPTR		schain=gchain;
				IOPTR		io=t->treio;
				gchain=0;
				argn = getarg(t);
				com=scan(argn);
				a1=com[1]; 
				gchain=schain;

				if( (internal=syslook(com[0],commands)) || argn==0 ){	
					setlist(t->comset, 0);
					;
					}

				if( argn && (flags&noexec)==0 ){	/* print command if execpr */
					if( flags&execpr ){	
						argn=0;	
						prs(execpmsg);
						while( com[argn]!=ENDARGS ){ 
							prs(com[argn++]); 
							blank() ;
							}
						newline();
						;
						}

					switch( internal ){

					case SYSDOT:
						if( a1 ){	
							register int		f;

							if( (f=pathopen(getpath(a1), a1)) < 0 ){ 
								failed(a1,notfound);
								} 
							else { 
								execexp(0,f);
								;
								}
							;
							}
						break;

					case SYSTIMES:
						{
							long	t[4]; 
							times(t);
							prt(t[2]); 
							blank(); 
							prt(t[3]); 
							newline();
							}
						break;

					case SYSLOGOUT: /* logout - dhj 12/6/79 */
						flags |= errflg; /* dhj */
					case SYSEXIT:
						exitsh(a1?stoi(a1):oldexit);

					case SYSNULL:
						io=0;
						break;

					case SYSCONT:
						execbrk = -loopcnt; 
						break;

					case SYSBREAK:
						if( (execbrk=loopcnt) && a1 ){ 
							breakcnt=stoi(a1);
							;
							}
						break;

					case SYSTRAP:
						if( a1 ){	
							char	clear;
							if( (clear=digit(*a1))==0 ){	
								++com;
								;
								}
							while( *++com ){ 
								int	i;
								if( (i=stoi(*com))>=MAXTRAP || i<MINTRAP ){	
									failed(*com,badtrap);
									} 
								else if ( clear ){	
									clrsig(i);
									} 
								else {	
									replace(&trapcom[i],a1);
									if( *a1 ){	
										getsig(i);
										} 
									else {	
										ignsig(i);
										;
										}
									;
									}
								;
								}
							} 
						else {	/* print out current traps */
							int		i;

							for( i=0; i<MAXTRAP; i++ ){ 
								if( trapcom[i] ){	
									prn(i); 
									prs(colon); 
									prs(trapcom[i]); 
									newline();
									;
									}
								;
								}
							;
							}
						break;

					case SYSEXEC:
						com++;
						initio(io); 
						ioset=0; 
						io=0;
						if( a1==0 ){ 
							break ;
							}

					case SYSLOGIN:
						flags |= forked;
						oldsigs(); 
						execa(com); 
						done();

					case SYSCD:
						if( flags&rshflg ){	
							failed(com[0],restricted);
							} 
						else if ( (a1==0 && (a1=homenod.namval)==0) || chdir(a1)<0 ){	
							failed(a1,baddir);
							;
							}
						break;

					case SYSSHFT:
						if( dolc<1 ){	
							error(badshift);
							} 
						else {	
							dolv++; 
							dolc--;
							;
							}
						assnum(&dolladr, dolc);
						break;

					case SYSWAIT:
						await(-1);
						break;

					case SYSREAD:
						exitval=readvar(&com[1]);
						break;

						/*
										case SYSTST:
											exitval=testcmd(com);
											break;
						*/

					case SYSSET:
						if( a1 ){	
							int	argc;
							argc = options(argn,com);
							if( argc>1 ){	
								setargs(com+argn-argc);
								;
								}
							} 
						else if ( t->comset==0 ){	/*scan name chain and print*/
							namscan(printnam);
							;
							}
						break;

					case SYSRDONLY:
						exitval=N_RDONLY;
					case SYSXPORT:
						if( exitval==0 ){ 
							exitval=N_EXPORT; 
							;
							}

						if( a1 ){	
							while( *++com ){ 
								attrib(lookup(*com), exitval) ;
								}
							} 
						else {	
							namscan(printflg);
							;
							}
						exitval=0;
						break;

					case SYSEVAL:
						if( a1 ){	
							execexp(a1,&com[2]);
							;
							}
						break;

					case SYSUMASK:
						if (a1) {
							int c, i
							    i = 0;
							while ((c = *a1++) >= '0' &&
							    c <= '7')
								i = (i << 3) + c - '0';
							umask(i);
							} 
						else {
							int i, j;
							umask(i = umask(0));
							prc('0');
							for (j = 6; j >= 0; j -= 3)
								prc(((i>>j)&07) + '0');
							newline();
							}
						break;

					default:
						internal=builtin(argn,com);

						}

					if( internal ){	
						if( io ){ 
							error(illegal) ;
							}
						chktrap();
						break;
						;
						}
					} 
				else if ( t->treio==0 ){	
					break;
					;
					}
				}

		case TFORK:
			if( execflg && (treeflgs&(FAMP|FPOU))==0 ){	
				parent=0;
				} 
			else {	
				while( (parent=fork()) == -1 ){ 
					sigchk(); 
					alarm(10); 
					pause() ;
					}
				;
				}

			if( parent ){	/* This is the parent branch of fork;    */
				/* it may or may not wait for the child. */
				if( treeflgs&FPRS && flags&ttyflg ){	
					prn(parent); 
					newline();
					;
					}
				if( treeflgs&FPCL ){ 
					closepipe(pf1) ;
					}
				if( (treeflgs&(FAMP|FPOU))==0 ){	
					await(parent);
					} 
				else if ( (treeflgs&FAMP)==0 ){	
					post(parent);
					} 
				else {	
					assnum(&pcsadr, parent);
					;
					}

				chktrap();
				break;


				} 
			else {	/* this is the forked branch (child) of execute */
				flags |= forked; 
				iotemp=0;
				postclr();
				settmp();

				/* Turn off INTR and QUIT if `FINT'  */
				/* Reset ramaining signals to parent */
				/* except for those `lost' by trap   */
				oldsigs();
				if( treeflgs&FINT ){	
					signal(INTR,1); 
					signal(QUIT,1);
					;
					}

				/* pipe in or out */
				if( treeflgs&FPIN ){	
					rename(pf1[INPIPE],0);
					close(pf1[OTPIPE]);
					;
					}
				if( treeflgs&FPOU ){	
					rename(pf2[OTPIPE],1);
					close(pf2[INPIPE]);
					;
					}

				/* default std input for & */
				if( treeflgs&FINT && ioset==0 ){	
					rename(chkopen(devnull),0);
					;
					}

				/* io redirection */
				initio(t->treio);
				if( type!=TCOM ){	
					execute(t->forktre,1);
					} 
				else if ( com[0]!=ENDARGS ){	
					setlist(t->comset,N_EXPORT);
					execa(com);
					;
					}
				done();
				;
				}

		case TPAR:
			rename(dup(2),output);
			execute(t->partre,execflg);
			done();

		case TFIL:
			{
				int pv[2]; 
				chkpipe(pv);
				if( execute(t->lstlef, 0, pf1, pv)==0 ){	
					execute(t->lstrit, execflg, pv, pf2);
					} 
				else {	
					closepipe(pv);
					;
					}
				}
			break;

		case TLST:
			execute(t->lstlef,0);
			execute(t->lstrit,execflg);
			break;

		case TAND:
			if( execute(t->lstlef,0)==0 ){	
				execute(t->lstrit,execflg);
				;
				}
			break;

		case TORF:
			if( execute(t->lstlef,0)!=0 ){	
				execute(t->lstrit,execflg);
				;
				}
			break;

		case TFOR:
			{
				NAMPTR	n = lookup(t->fornam);
				char*	*args; 
				DOLPTR	argsav=0;

				if( t->forlst==0 ){    
					args=dolv+1;
					argsav=useargs();
					} 
				else {	   
					ARGPTR	schain=gchain;
					gchain=0;
					trim((args=scan(getarg(t->forlst)))[0]);
					gchain=schain;
					;
					}
				loopcnt++;
				while( *args!=ENDARGS && execbrk==0 ){	
					assign(n,*args++);
					execute(t->fortre,0);
					if( execbrk<0 ){ 
						execbrk=0 ;
						}
					;
					}
				if( breakcnt ){ 
					breakcnt-- ;
					}
				execbrk=breakcnt; 
				loopcnt--;
				argfor=freeargs(argsav);
				}
			break;

		case TWH:
		case TUN:
			{
				int		i=0;

				loopcnt++;
				while( execbrk==0 && (execute(t->whtre,0)==0)==(type==TWH) ){ 
					i=execute(t->dotre,0);
					if( execbrk<0 ){ 
						execbrk=0 ;
						}
					;
					}
				if( breakcnt ){ 
					breakcnt-- ;
					}
				execbrk=breakcnt; 
				loopcnt--; 
				exitval=i;
				}
			break;

		case TIF:
			if( execute(t->iftre,0)==0 ){	
				execute(t->thtre,execflg);
				} 
			else {	
				execute(t->eltre,execflg);
				;
				}
			break;

		case TSW:
			{
				register char*	r = mactrim(t->swarg);
				t=t->swlst;
				while( t ){	
					ARGPTR		rex=t->regptr;
					while( rex ){	
						register char*	s;
						if( gmatch(r,s=macro(rex->argval)) || (trim(s), eq(r,s)) ){	
							execute(t->regcom,0);
							t=0; 
							break;
							} 
						else {	
							rex=rex->argnxt;
							;
							}
						;
						}
					if( t ){ 
						t=t->regnxt ;
						}
					;
					}
				}
			break;
			}
		exitset();
		;
		}

	sigchk();
	tdystak(sav);
	return(exitval);
	}


execexp(s,f)
char*		s;
UFD		f;
{
	FILEBLK		fb;
	push(&fb);
	if( s ){	
		estabf(s); 
		fb.feval=f;
		} 
	else if ( f>=0 ){	
		initf(f);
		;
		}
	execute(cmd('\n', NLFLG|MTFLG),0);
	pop();
	}
