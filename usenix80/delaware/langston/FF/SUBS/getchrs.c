#include    <stdio.h>
/*
**	GETCHRS -- Get a <NUL> terminated bunch of characters
** Compile: cc -O -c -q getchrs.c; ar r /lib/libP.a getchrs.o
*/

getchrs(nagp)
char *nagp;
{
	register char *cp;
	static char buffs[4][128];
	static int nextbuf;

	printf("%s", nagp);
	nextbuf = (nextbuf + 1) & 3;
	cp = buffs[nextbuf];
	if (fgets(cp, 128, stdin) == NULL)
	    return(NULL);
	while (*cp && *cp != '\n')
	    cp++;
	*cp = '\0';
	return(buffs[nextbuf]);
}
