#define	D_NATSTR
#include	"empdef.h"

char	*
cname(num)
int	num;
{
	static char	buf[96];
	static int	nextbuf;
	char	*copy();

	if( getnat(num) == -1 ) return("");
	nextbuf = (nextbuf + 1) & 03;
	copy(&nat,  &buf[nextbuf * 20]);
	return(&buf[nextbuf * 20]);
}
