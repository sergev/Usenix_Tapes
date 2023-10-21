#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	<sys/types.h>
#define DIRSIZ 15
#include	<sys/stat.h>
#include	<sys/dir.h>



/* globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 */

extern int	addg();


int	expand(as,rflg)
char*		as;
{
	int		count, dirf;
	char		dir=0;
	char*		rescan = 0;
	register char	*s, *cs;
	ARGPTR		schain = gchain;
	struct direct	entry;
	STATBUF		statb;

	if( trapnote&SIGSET ){ 
		return(0); 
		;
		}

	s=cs=as; 
	entry.d_name[DIRSIZ-1]=0; /* to end the string */

	/* check for meta chars */
	{
		register char slash; 
		slash=0;
		while( !fngchar(*cs) ){	
			if( *cs++==0 ){	
				if( rflg && slash ){ 
					break; 
					} 
				else { 
					return(0) ;
					}
				} 
			else if ( *cs=='/' ){	
				slash++;
				;
				}
			;
			}
		}

	for(;;){	
		if( cs==s ){	
			s=nullstr;
			break;
			} 
		else if ( *--cs == '/' ){	
			*cs=0;
			if( s==cs ){ 
				s="/" ;
				}
			break;
			;
			}
		}
	if( stat(s,&statb)>=0
	    && (statb.st_mode&S_IFMT)==S_IFDIR
	    && (dirf=open(s,0))>0 ){	
		dir++;
		;
		}
	count=0;
	if( *cs==0 ){ 
		*cs++=0200 ;
		}
	if( dir ){	/* check for rescan */
		register char* rs; 
		rs=cs;

		do{	
			if( *rs=='/' ){ 
				rescan=rs; 
				*rs=0; 
				gchain=0 ;
				}
			}
		while(	*rs++ );

		while( read(dirf, &entry, 16) == 16 && (trapnote&SIGSET) == 0 ){	
			if( entry.d_ino==0 ||
			    (*entry.d_name=='.' && *cs!='.') ){	
				continue;
				;
				}
			if( gmatch(entry.d_name, cs) ){	
				addg(s,entry.d_name,rescan); 
				count++;
				;
				}
			;
			}
		close(dirf);

		if( rescan ){	
			register ARGPTR	rchain;
			rchain=gchain; 
			gchain=schain;
			if( count ){	
				count=0;
				while( rchain ){	
					count += expand(rchain->argval,1);
					rchain=rchain->argnxt;
					;
					}
				;
				}
			*rescan='/';
			;
			}
		;
		}

	{
		register char	c;
		s=as;
		while( c = *s ){	
			*s++=(c&STRIP?c:'/') ;
			}
		}
	return(count);
	}

gmatch(s, p)
register char	*s, *p;
{
	register int		scc;
	char		c;

	if( scc = *s++ ){	
		if( (scc &= STRIP)==0 ){	
			scc=0200;
			;
			}
		;
		}
	switch( c = *p++ ){

	case '[':
		{
			char ok; 
			int lc;
			ok=0; 
			lc=077777;
			while( c = *p++ ){	
				if( c==']' ){	
					return(ok?gmatch(s,p):0);
					} 
				else if ( c=='-' ){	
					if( lc<=scc && scc<=(*p++) ){ 
						ok++ ;
						}
					} 
				else {	
					if( scc==(lc=(c&STRIP)) ){ 
						ok++ ;
						}
					;
					}
				;
				}
			return(0);
			}

	default:
		if( (c&STRIP)!=scc ){ 
			return(0) ;
			}

	case '?':
		return(scc?gmatch(s,p):0);

	case '*':
		if( *p==0 ){ 
			return(1) ;
			}
		--s;
		while( *s ){  
			if( gmatch(s++,p) ){ 
				return(1) ;
				} 
			;
			}
		return(0);

	case 0:
		return(scc==0);
		}
	}

static int	addg(as1,as2,as3)
char		*as1, *as2, *as3;
{
	register char	*s1, *s2;
	register int		c;

	s2 = locstak()+BYTESPERWORD;

	s1=as1;
	while( c = *s1++ ){	
		if( (c &= STRIP)==0 ){	
			*s2++='/';
			break;
			;
			}
		*s2++=c;
		;
		}
	s1=as2;
	while( *s2 = *s1++ ){ 
		s2++ ;
		}
	if( s1=as3 ){	
		*s2++='/';
		while( *s2++ = *++s1 );
		;
		}
	makearg(endstak(s2));
	}

makearg(args)
register char*	args;
{
	args->argnxt=gchain;
	gchain=args;
	}

