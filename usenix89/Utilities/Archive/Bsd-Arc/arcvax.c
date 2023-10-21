#include <stdio.h>
#include <ctype.h>
#include "arc.h"

/*
 * Misc routines to emulate IBM MSDOS functions under BSD
 *
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 */
INT upper(string)
char *string;
{
    char *p;

    for(p = string; *p != NULL; p++)
	if(islower(*p))
	    *p = toupper(*p);
}
char *setmem(dest,size,c)
char *dest,c;
INT size;
{
 INT i;

    for(i = 0; i < size; dest[i] = c, i++);
    return(&dest[0]);
}
char *gcdir(dirname)
char *dirname;

{
	if(dirname == NULL || strlen(dirname) == 0)
		dirname = (char *)malloc(1024);

	getwd(dirname);
}
INT abort(s,arg1,arg2,arg3)
char *s;
{
    fprintf(stderr,"ARC: ");
    fprintf(stderr,s,arg1,arg2,arg3);
    fprintf(stderr,"\n");
#if BSD
    perror("BSD");
#endif
    exit(1);
}
