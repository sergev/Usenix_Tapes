/*
 * TREK73: parse_opt.c
 *
 * Parse the environment variable TREK73OPTS
 *
 * parse_opts
 */

#include <ctype.h>
#include "structs.h"
#include "options.h"

#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)

#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

/*
 * description of an option and what to do with it
 */
struct optstruct {
    char	*o_name;	/* option name */
    int		*o_opt;		/* pointer to thing to set */
    int		o_type		/* Boolean or string */
};

typedef struct optstruct	OPTION;

extern	int terse;
extern	char captain[];
extern	char sex[];
extern	char shipbuf[];
extern	char science[];
extern	char engineer[];
extern	char com[];
extern	char nav[];
extern	char helmsman[];
extern	char shipname[];
extern  char foename[];
extern  char racename[];
extern	int silly;


OPTION	optlist[] = {
    {"terse",	 (int *)&terse, BOOLEAN},
    {"name", (int *)captain, STRING},
    {"sex", (int *)sex, STRING},
    {"ships", (int *)shipbuf, STRING},
    {"science", (int *)science, STRING},
    {"engineer", (int *)engineer, STRING},
    {"com", (int *)com, STRING},
    {"nav", (int *)nav, STRING},
    {"helmsman", (int *)helmsman, STRING},
    {"shipname", (int *)shipname, STRING},
    {"enemy", (int *)racename, STRING},
    {"foename", (int *)foename, STRING},
    {"silly", (int *)&silly, BOOLEAN},
};

/*
 * parse_opts:
 *	Parse options from string, usually taken from the environment.
 *	The string is a series of comma seperated values, with booleans
 *	being stated as "name" (true) or "noname" (false), and strings
 *	being "name=....", with the string being defined up to a comma
 *	or the end of the entire option string.
 */
parse_opts(str)
register char *str;
{
    register char *sp;
    register OPTION *op;
    register int len;

    while (*str)
    {
	/*
	 * Get option name
	 */
	for (sp = str; isalpha(*sp); sp++)
	    continue;
	len = sp - str;
	/*
	 * Look it up and deal with it
	 */
	for (op = optlist; op < &optlist[NUM_OPTS]; op++)
	    if (EQSTR(str, op->o_name, len))
	    {
		if (op->o_type == BOOLEAN)	/* if option is a boolean */
		    *op->o_opt = 1;
		else				/* string option */
		{
		    register char *start;
		    /*
		     * Skip to start of string value
		     */
		    for (str = sp + 1; *str == '='; str++)
			continue;
		    start = (char *) op->o_opt;
		    /*
		     * Skip to end of string value
		     */
		    for (sp = str + 1; *sp && *sp != ','; sp++)
			continue;
		    strucpy(start, str, sp - str);
		}
		break;
	    }
	    /*
	     * check for "noname" for booleans
	     */
	    else if (op->o_type == BOOLEAN
	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2))
	    {
		*op->o_opt = 0;
		break;
	    }

	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha(*sp))
	    sp++;
	str = sp;
    }
}

/*
 * strucpy:
 *	Copy string using unctrl for things
 */
strucpy(s1, s2, len)
register char *s1, *s2;
register int len;
{
    if (len > 100)
	len = 100;
    while (len--)
    {
	if (isprint(*s2) || *s2 == ' ')
	    *s1++ = *s2;
	s2++;
    }
    *s1 = '\0';
}

