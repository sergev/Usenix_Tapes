/*
 * TREK73: parseopts.c
 *
 * Parse the environment variable TREK73OPTS and command line options
 *
 * parse_opts, get_comlineopts
 */

#include <ctype.h>
#include <stdio.h>
#include "externs.h"
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


OPTION	optlist[] = {
    {"terse",	 (int *)&terse, BOOLEAN},
    {"name", (int *)captain, STRING},
    {"sex", (int *)sex, STRING},
    {"science", (int *)science, STRING},
    {"engineer", (int *)engineer, STRING},
    {"com", (int *)com, STRING},
    {"nav", (int *)nav, STRING},
    {"helmsman", (int *)helmsman, STRING},
    {"shipname", (int *)shipname, STRING},
    {"enemy", (int *)racename, STRING},
    {"foename", (int *)foename, STRING},
    {"silly", (int *)&silly, BOOLEAN},
    {"class", (int *)class, STRING},
    {"foeclass", (int *)foeclass, STRING},
    {"time", (int *)com_delay, STRING},
    {"teletype", (int *)&teletype, BOOLEAN},
    {"trace", (int *)&trace, BOOLEAN},
    {"savefile", (int *)savefile, STRING}
};

/*
 * parse_opts:
 *	Parse options from string, usually taken from the environment.
 *	The string is a series of comma separated values, with booleans
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
	for (sp = str; isascii(*sp) && isalpha(*sp); sp++)
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
	while (*sp && !(isascii(*sp) && isalpha(*sp)))
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
	if (isascii(*s2) && isprint(*s2))
	    *s1++ = *s2;
	s2++;
    }
    *s1 = '\0';
}

get_comlineopts(argc, argv)
int argc;
char *argv[];
{
	int c;
	char *opts = "Rtc:s:S:E:C:N:H:f:r:d:yTn:F:l:v";
	int errflg = 0;

	while ((c = getopt(argc, argv, opts)) != EOF) {
		switch(c) {
		case 'R':
			restart = 1;
			break;
		case 't':
			terse = 1;
			break;
		case 'c':
			(void) strncpy(captain, optarg, sizeof captain);
			captain[sizeof captain - 1] = '\0';
			break;
		case 's':
			(void) strncpy(sex, optarg, sizeof sex);
			sex[sizeof sex - 1] = '\0';
			break;
		case 'S':
			(void) strncpy(science, optarg, sizeof science);
			science[sizeof science - 1] = '\0';
			break;
		case 'E':
			(void) strncpy(engineer, optarg, sizeof engineer);
			engineer[sizeof engineer - 1] = '\0';
			break;
		case 'C':
			(void) strncpy(com, optarg, sizeof com);
			com[sizeof com - 1] = '\0';
			break;
		case 'N':
			(void) strncpy(nav, optarg, sizeof nav);
			nav[sizeof nav - 1] = '\0';
			break;
		case 'H':
			(void) strncpy(helmsman, optarg, sizeof helmsman);
			helmsman[sizeof helmsman - 1] = '\0';
			break;
		case 'f':
			(void) strncpy(foename, optarg, sizeof foename);
			foename[sizeof foename - 1] = '\0';
			break;
		case 'r':
			(void) strncpy(racename, optarg, sizeof racename);
			racename[sizeof racename - 1] = '\0';
			break;
		case 'd':
			(void) strncpy(com_delay, optarg, sizeof com_delay);
			com_delay[sizeof com_delay - 1] = '\0';
			break;
		case 'y':
			silly = 1;
			break;
		case 'T':
			teletype = 1;
			break;
		case 'n':
			(void) strncpy(shipname, optarg, sizeof shipname);
			shipname[sizeof shipname - 1] = '\0';
			break;
		case 'F':
			(void) strncpy(foeclass, optarg, sizeof foeclass);
			foeclass[sizeof foeclass - 1] = '\0';
			break;
		case 'l':
			(void) strncpy(class, optarg, sizeof class);
			class[sizeof class - 1] = '\0';
			break;
		case 'v':
			trace = TR_ON;
			break;
		case '?':
		default:
			errflg++;
			break;
		}
	}
	if (errflg) {
		fprintf(stderr, "Usage: trek73 [-t] [-c captain's name] [-s sex] [-S science officer]\n        [-E engineer] [-C communications officer] [-N navigator] [-H helmsman]\n");
		fprintf(stderr, "        [-f enemy captain] [-r enemy race] [-d command delay time] [-y] [-T]  \n        [-n shipname] [-F enemy ship type] [-l federation ship type] [-v]\n");
		fprintf(stderr, "       or trek73 -R");
		exit(1);
	}
}
