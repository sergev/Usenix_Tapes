/* do an accented char */

/* input is a string with the accent to print then a space then the  */
/*	character to accent */

#include "dca2troff.h"
#include <ctype.h>

struct acc {
	char *generic;
	char *output;
} lacc[] = {		/* lower case accents */
	"`",	"\\(ga",		/* grave */
	"'",	"\\(aa",		/* acute */
	"^",	"^",			/* circumflex */
	"~",	"~",			/* tilde */
	"..",	"\\(..",		/* diaresis */
	"de",	"\\(de",		/* angstrom */
	"cd",	"\\(cd",		/* cedilla */
	"",	""
};
struct acc uacc[] = {	/* upper case accents */
	"`",	"\\f(Sr\\(ga\\fP",	/* grave */
	"'",	"\\f(Sr\\(aa\\fP",	/* acute */
	"^",	"\\f(Sr^\\fP",		/* circumflex */
	"~",	"\\f(Sr~\\fP",		/* tilde */
	"..",	"\\f(Sr\\(..\\fP",	/* diaresis */
	"de",	"\\f(Sr\\(de\\fP",	/* angstrom */
	"cd",	"\\(cd",		/* cedilla */
	"",	""
};
#include "dca2troff.h"
do_accent(str)
char *str;
{
	char accent[10];
	char character;
	int i;

	sscanf(str, "%s %c", accent, &character);

	if ( islower(character) )	/* lower case character no problem */
		{
		i = checkacc(lacc, accent);
		if ( i == -1 )
			{
			fprintf(stderr, "unknown accent %s\n", accent);
			return(0);
			}
		sprintf(tline, "\\o'%s%c'", lacc[i], character);
		}
	else
		{
		i = checkacc(uacc, accent);
		if ( i == -1 )
			{
			fprintf(stderr, "unknown accent %s\n", accent);
			return(0);
			}
		sprintf(tline, "\\u\\z%s\\d%c", uacc[i], character);
		}
	outstr(tline);
}

/* search for accent string in struct */
/* return index if ok, else -1 */
checkacc(str, s)
char *str[];
char *s;
{
	int i;
	for(i=0;;i++)
		{
		if((strcmp(str[i], "\0") == 0 ))
			return(-1);
		if((strcmp(str[i], s) == 0 ))
			return(i);
		}
}
