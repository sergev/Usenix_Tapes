#ifndef lint
static char rcsid[] = "$Header: util.c,v 1.1 84/08/25 17:11:33 jcoker Exp $";
#endif

/*
 *  Return a string version of the non-printing
 *  character.
 */

#include <stdio.h>
#include <ctype.h>

#define FIRST_PRINT	' '
#define DELETE		127

static char	*control_strings[] = {
	"^@", "^A", "^B", "^C", "^D", "^E", "^F", "^G", "^H", "^I", "^J",
	"^K", "^L", "^M", "^N", "^O", "^P", "^Q", "^R", "^S", "^T", "^U",
	"^V", "^W", "^X", "^Y", "^Z", "^[", "^\\", "^]", "^^", "^_", NULL
};
static char	*delete_string = "^?";

char *
unctrl(c)
{
	static char	buf[10];

	if (c < FIRST_PRINT)
		strcpy(buf, control_strings[c]);
	else if (c == DELETE)
		strcpy(buf, delete_string);
	else {
		buf[0] = c;
		buf[1] = '\0';
	}

	return(buf);
}


/*
 *  Quickie more.
 */

do_more()
{
	int	c;

	fputs(" --More--", stdout);

	do
		c = getchar();
	while (c != EOF && c != ' ' && c != '\n');

	if (c == EOF)		/* clear the error */
		rewind(stdin);
}
