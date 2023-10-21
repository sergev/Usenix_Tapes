/*
 * poparg - pop next argument off argv list
 *	    returns pointer to arg; else NULL when no more
 *
 * Rex Sanders, USGS Pacific Marine Geology, 2/87
 */

#include <stdio.h>

char   *
poparg (argc, argv)
int    *argc;
char ***argv;

{
    char   *newarg;

    if (*argc > 0) {
	newarg = **argv;
	(*argc)--;
	(*argv)++;
    }
    else
	newarg = NULL;

    return (newarg);
}
