#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"

setbrk(incr)
{
	register BYTPTR	a=sbrk(incr);
	brkend=a+incr;
	return(a);
	}
