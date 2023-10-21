#ifndef lint
static char rcsid[] = "$Header: efopen.c,v 1.2 87/03/02 17:45:15 root Exp $";
static char rcswhere[] = "$Source: /usr/src/local/local/mkprog/RCS/efopen.c,v $";
#endif

#include <stdio.h>

FILE *
efopen(file, mode)	/* fopen file, die if cannot */
char *file, *mode;	/* from K & P with addition of perror() and handling
			   of "-" as stdin */
{
    FILE *fp;
    extern char *progname;

    if (strcmp(file, "-") == 0)
	return(stdin);

    if ((fp = fopen(file, mode)) != NULL)
	return (fp);

    if (progname)
	fprintf(stderr, "%s: ", progname);
    fprintf(stderr, "can't open file %s mode %s: ", file, mode);
    perror("");
    exit(1);
	/* NOTREACHED */
}

/* This makes it more portable to a non-unix environment */

#ifndef unix
perror(s)
char *s;
{
    putc('\n', stderr);
}
#endif
