#include	"../godef.h"
/*
**      GLB -- globals for Go routines
**      psl 5/84
*/

char	*color[]	= { "oops!", "black", "white", "oops!!", };
char	fmtbuf[128];

int	liblist[MAXLIBS], nlibs;
int     dd[4] = { MUP, MRIGHT, MDOWN, MLEFT, };

struct  plyrstr	p[3];
struct	bdstr	b;
