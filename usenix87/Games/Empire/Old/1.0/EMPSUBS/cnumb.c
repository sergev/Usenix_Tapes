#define	D_NATSTR
#include	"empdef.h"

cnumb(string)
char *string;
{
	register char	*cnp, *sp;
	register int	cno;
	int	pmatch, pmcnt;

	pmcnt = 0;
	for (cno = 0; getnat(cno) != -1; cno++) {
		cnp = nat.nat_cnam;
		sp = string;
		while (*sp++ == *cnp++) {
			if (*sp == 0)
				if (*cnp == 0) {
					return (cno);
				} else {
					pmatch = cno;
					pmcnt++;
					break;
				}
		}
	}

	if (pmcnt == 1)
		return(pmatch);
	else
		return(-1);
}
