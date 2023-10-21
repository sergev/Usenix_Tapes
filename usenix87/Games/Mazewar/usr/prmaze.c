#ifndef lint
static char rcsid[] = "$Header: prmaze.c,v 1.1 84/08/25 17:11:26 chris Exp $";
#endif

#include <stdio.h>

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/statmap.h"

main()
{
    register int i, j;

    for (j = 0; j < MAZELENGTH; j++) {
	for (i = 0; i < MAZEWIDTH; i++)
	    if (maze[i][j])
		fprintf(stdout, "[]");
	    else
		fprintf(stdout, "  ");
	fprintf(stdout, "\n");
    }
}
