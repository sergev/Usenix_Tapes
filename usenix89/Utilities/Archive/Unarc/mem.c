/*
**  Get some memory or die trying.
*/
/* LINTLIBRARY */
#include "shar.h"
RCS("$Header: mem.c,v 1.6 87/03/04 14:51:20 rs Exp $")


align_t
getmem(i, j)
    unsigned int	 i;
    unsigned int	 j;
{
    extern char		*calloc();
    align_t		 p;

    if ((p = (align_t)calloc(i, j)) == NULL) {
	/* Print the unsigned's as int's so ridiculous values show up. */
	fprintf(stderr, "Can't Calloc(%d,%d), %s.\n", i, j, Ermsg(errno));
	exit(1);
    }
    return(p);
}
