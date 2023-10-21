#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

char		numbuf[6];


/* printing and io conversion */

newline()
	{	
	prc('\n');
	}

blank()
	{	
	prc(' ');
	}

prp()
	{
	if( (flags&prompt)==0 && cmdadr){	
		prs(cmdadr); 
		prs(colon);
		}
	}

prs(as)
	char*		as;
	{
	register char*	s;

	if( s=as){	
		write(output,s,length(s)-1);
		}
	}

prc(c)
	char		c;
	{
	if( c){	
		write(output,&c,1);
		}
	}

prt(t)
	long		t;
	{
	register int	hr, min, sec;

	t += 30; 
	t /= 60;
	sec=t%60; 
	t /= 60;
	min=t%60;
	if( hr=t/60){	
		prn(hr); 
		prc('h');
		}
	prn(min); 
	prc('m');
	prn(sec); 
	prc('s');
	}

prn(n)
	int		n;
	{
	itos(n); 
	prs(numbuf);
	}

itos(n)
	{
	register char *abuf; 
	register unsigned a, i; 
	int pr, d;
	abuf=numbuf; 
	pr=FALSE; 
	a=n;
	for( i=10000; i!=1; i/=10){	
		if( (pr |= (d=a/i)) )
			*abuf++=d+'0';
		a %= i;
		}
	*abuf++=a+'0';
	*abuf++=0;
	}

stoi(icp)
	char*	icp;
	{
	register char	*cp = icp;
	register int		r = 0;
	register char	c;

	while( (c = *cp, digit(c)) && c && r>=0){ 
		r = r*10 + c - '0'; 
		cp++;
 		}
	if( r<0 || cp==icp){	
		failed(icp,badnum);
		} 
	else {	
		return(r);
		}
	}
