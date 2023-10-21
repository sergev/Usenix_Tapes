#include "dca2troff.h"

/* output a char to a buffer */
/*  so that the "underline previous word" will work */
/*  buffer flushed on seeing white space */

outachar(c)
char c;
{
	int ic;
	ic = c;
	ic &= 0177;
	switch(ic)
	    {
	    case ' ':
	    case '\n':
	    case '\t':			/* flush buffer and new character */
		printf("%s", bufline);
		blpt = 0;
		bufline[blpt] = NULL;
		printf("%c", c);
		return;
	    case '\177':		/* flush buffer but don't dump char */
		printf("%s", bufline);
		blpt = 0;
		bufline[blpt] = NULL;
		return;
	    case NULL:
		return;			/* jic */
	    default:			/* store the character */
		bufline[blpt++] = c;
		bufline[blpt] = NULL;
		return;
	    }
}

outstr(c)
char *c;
{
	char ic;
	if ( *c == NULL )
	    return;
	for (; *c != NULL; *c++)
	    {
	    ic = *c;
	    outachar(ic);
	    }
}
