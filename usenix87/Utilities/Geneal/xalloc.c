/* xalloc - allocate memory, give error message and die if no more */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 15-Sep-84 16:50:25 by jimmc (Jim McBeath) */
/* last edit 09-Sept-85 21:50:00 by tlr (Terry L. Ridder) */

#include <stdio.h>

#define ERROR_EXIT 1

static int totalused = 0;

char *
xalloc(size, msg)
int size;		/* number of bytes to allocate */
char *msg;		/* error string */
{
char *x;

    extern char *malloc();
	
    x = malloc((unsigned)size);
    if (x == 0)
    {
	fprintf(stderr, "\nNo more memory (%s)\n", msg);
	fprintf(stderr, "Previously used: %d; this request: %d\n",
			totalused, size);
	exit(ERROR_EXIT);
    }
    totalused += size;
    return x;
}

/* end */
