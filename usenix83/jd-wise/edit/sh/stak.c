#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

STKPTR		stakbot=nullstr;



/* ========	storage allocation	======== */

STKPTR	getstak(asize)
int		asize;
{	/* allocate requested stack */
	register STKPTR	oldstak;
	register int		size;

	size=round(asize,BYTESPERWORD);
	oldstak=stakbot;
	staktop = stakbot += size;
	return(oldstak);
	}

STKPTR	locstak()
{	/* set up stack for local use
	 * should be followed by `endstak'
	 */
	if( brkend-stakbot<BRKINCR){	setbrk(brkincr);
	if( brkincr < BRKMAX){	brkincr += 256;
	}
}
return(stakbot);
}

STKPTR	savstak()
{
assert(staktop==stakbot);
return(stakbot);
}

STKPTR	endstak(argp)
register char*	argp;
{	/* tidy up after `locstak' */
register STKPTR	oldstak;
*argp++=0;
oldstak=stakbot; 
stakbot=staktop=round(argp,BYTESPERWORD);
return(oldstak);
}

int	tdystak(x)
register STKPTR 	x;
{
/* try to bring stack back to x */
while( ADR(stakbsy)>ADR(x)){ 
free(stakbsy);
stakbsy = stakbsy->word;
}
staktop=stakbot=max(ADR(x),ADR(stakbas));
rmtemp(x);
}

stakchk()
{
if( (brkend-stakbas)>BRKINCR+BRKINCR){	setbrk(-BRKINCR);
}
}

STKPTR	cpystak(x)
STKPTR		x;
{
return(endstak(movstr(x,locstak())));
}
