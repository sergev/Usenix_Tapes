#include <stdio.h>
#include <ctype.h>
/*
 * number
 *
 *	Number is a program that counts in lots of languages.
 *	It was originally written during a sanity break while
 *	I was writing my PhD thesis.  That version got left
 *	on a machine somewhere in upstate New York.  This one
 *	was done while on leave in Grenoble, after realizing
 *	that I hadn't written a computer program in over two
 *	months.
 *
 *	Number is inspired, of course, from /usr/games/number, but
 *	it uses a series of grammars that define counting in
 *	different languages.  The language that is used to
 *	write the grammars is described below, in evalrule().
 *	If you write any new grammars, I'd greatly appreciate
 *	having them.  Grammars aren't very hard to write, if
 *	you know how to count in something that isn't defined
 *	here.  The longest grammar (french) only has 30 rules
 *	and 5 macros, and correctly pronounces any number less
 *	1,000,000,000,000.  The shortest is for cantonese, which
 *	has 14 rules.
 *
 * A note on the output of number:
 *
 *	The characters that are output conform to the TIRA
 *	character representation standard.  Essentially, strings
 *	in anything except the latin alphabet (what you're reading
 *	now) are preceded by an indication of the alphabet that
 *	they are part of.  The exceptions to this are mandarin,
 *	cantonese and japanese.  These three are written in
 *	pin-yin, roughly Wade Giles, and romanji, respectively.
 *	The only other thing special about this format is that
 *	accents and tone markings are given in [] brackets
 *	before the letter to which they are attached.
 *
 *	TIRA stands for Textual Information Retrieval and Analysis
 *	research group, and is a research group at the University
 *	of Chicago containing computer and information scientists,
 *	literary scholars and linguists.  TIRA is working on a
 *	research environment for doing textual research.  Watch
 *	this space.
 *
 * Copyright 1987, Scott Deerwester.
 *
 *	This code may be freely distributed and copied, provided
 *	that a copy of this notice accompanies all copies and
 *	that no copy is sold for profit.
 */

/*
 *	Constants for array bounds.  Both of these are overkill.
 */

#define	MAXRULES	100
#define	MAXSPECIALS	50

/*
 *	Structure to hold macro definitions.
 */

struct {
	char	c;
	char	*rule;
} specials [MAXSPECIALS];

int	nspecials = 0;
int	maxdigits;

/*
 *	Definition of a grammar rule.
 */

struct {
	int	base;
#ifdef	COND
	int	cond;
#endif	COND
	char	*rule;
} rule[MAXRULES];

int	nrules;
char	*lang = "english";		/* You can change this if you like */
char	*malloc ();
unsigned long	parsenumber();
long	atol(), random();

int	dbgflag = 0;

main (argc, argv)
     char *argv[];
{
        int errflg = 0;

	chkdbg ();
	srandom (getpid ());
	domaxdigits ();
/*
 *	Someday, maybe, I'll enable this to take a number
 *	on the command line.
 */

        switch (argc) {
	case 1:	break;
	case 2:	lang = argv[1];	break;
	default:
		errflg++;
	        break;
	}

	if (errflg)
	{
	        fprintf (stderr, "Usage: number [language]\n");
		exit (0);
	}

/*
 *	read_grammar finds the grammar for the language and
 *	reads it in.  It exits if it can't find the grammar.
 */
	read_grammar (lang);
/*
 *	Main loop.  Read in numbers.  Make sure that the input
 *	is a number, and spell it in the requested language.
 */
	while (1)
	{
		char lbuf [512];
		register i, l;
		unsigned long u;
		long n;

		if (isatty (0))
			printf ("> ");

		if (!gets (lbuf))
		{
			break;
		}

		if ((l = strlen (lbuf)) > maxdigits)
		{
			printf ("My limit is ");
			for (i = 0; i < maxdigits; i++)
				putchar ('9');
			putchar ('\n');
			continue;
		} else if (l == 0)
			continue;

		n = 0;
		if (sscanf (lbuf, "%ld", &n) != 1)
		{
			printf ("%s is not a non-negative integer.\n", lbuf);
			continue;
		}

		if (n < 0)
		{
			printf ("I don't handle negative numbers.\n");
			continue;
		}

		sscanf (lbuf, "%ld", &u);
		spell (u, 0);
		outchar ('\n');
	}
	outchar ('\n');
}

domaxdigits ()
{
	unsigned long maxint = 0;
	register i;
	char str [128];

	for (i = 0; i < sizeof (long) * 8; i++)
		maxint |= 1 << i;
	sprintf (str, "%lu", maxint);
	maxdigits = strlen (str) - 1;

dbg ("domaxdigits computes %lu as %d reliable digits.\n", maxint, maxdigits);
}

/*
 *	read_to_eol is equivalent to fgets, except that it
 *	reads the string into a temporary buffer, allocates
 *	enough space for it, and copies the string into the
 *	allocated space.  In other words, it does what fgets()
 *	would do if C had proper memory management. :-)
 */

char *read_to_eol (fp)
	FILE *fp;
{
	char	*tmpbuf, *cp;
	char	rbuf [512];
	register l = 0;

	cp = rbuf;
	while (1)
	{
		fgets (cp, sizeof (rbuf) - l, fp);
		l = strlen (rbuf);
		if (rbuf [l - 2] != '\\')
			break;
		cp = rbuf + l - 2;
		if (getc (fp) != '\t')
		{
			fprintf (stderr, "read_to_eol didn't find a tab\n");
			exit (0);
		}
		if (l >= sizeof (rbuf))
		{
			fprintf (stderr, "rule too long in read_to_eol\n");
			exit (0);
		}
	}
	
	tmpbuf = malloc (l = strlen (rbuf));
	rbuf [l - 1] = '\0';	/* get rid of the newline */

	strcpy (tmpbuf, rbuf);

	return (tmpbuf);
}


static char filename[128];

/*
 *	Cutesy error messages.  They all say the same thing.
 */

char *errorfmt[] =
{
	"No se habla \"%s\".  Se habla:\n",
	"I don't speak \"%s\".  I speak:\n",
	"On ne parle pas \"%s\" ici.  On parle plut[^]ot:\n",
	"Ich kann nicht \"%s\" sprechen.  Ich spreche:\n",
	"Ng[?]o [_]m s[^]ik g[']ong \"%s\" w[`]a.  Ng[?]o s[^]ik:\n",
	"W[?]o b[`]u hu[`]e \"%s\".  W[?]o hu[`]e:\n",
	"\CYR'Ya' n'ye' govor'yu' po-\"%s\".  'Ya' govor'yu':\n",
	"Nt[`]e \"%s\" kan m[`]en.  N[`]e be:\n"
};

#define	nerrfmt	8

rand (n)
{
	return (random () % n);
}

/*
 *	read_grammar depends on a set of grammar files being
 *	found in GRAMMARDIR.  It expects to find a file with
 *	the name of its parameter, which it opens and reads.
 *	If it can't find one, it prints out a message saying
 *	that it doesn't speak the language, and lists the
 *	known languages by exec'ing /bin/ls.  Note that this
 *	is equivalent to exitting.  It simply puts each of
 *	the rules and macros into arrays.  The format of the
 *	rules in the grammar files is:
 *
 *		n \t rule
 *
 *	where "n" is the base unit of the rule, and "rule"
 *	conforms to the syntax described below in evalrule().
 *	Macros definitions are of the form:
 *
 *		/ \t c \t rule
 *
 *	where "c" is the character to be expanded.  The character
 *	must not be a reserved character.
 *
 *	Grammars may also contain comment lines, which begin with
 *	a '#'.
 */
read_grammar (lang)
	char *lang;
{
	register i, c;
	FILE *fp;

	strcat (filename, GRAMMARDIR);
	strcat (filename, lang);

	if ((fp = fopen (filename, "r")) == NULL)
	{
		if ((fp = fopen (lang, "r")) == NULL)
		{
			printf (errorfmt [rand (nerrfmt)], lang);
			execl ("/bin/ls", "number-ls", GRAMMARDIR, 0);
		}
	}

	for (i = 0; !feof (fp);)
	{
#ifdef	COND
		rule[i].cond = 0;
#endif	COND

		if ((c = getc (fp)) == '/')
		{
			register j;

			while ((c = getc (fp)) == '\t')
				;
			j = nspecials++;
			specials[j].c = c;
			while ((c = getc (fp)) == '\t')
				;
			ungetc (c, fp);
			specials[j].rule = read_to_eol (fp);

dbg ("macro '%c': %s\n", specials[j].c, specials[j].rule);

			continue;
		} else if (c == EOF)
		{
			break;
		} else if (c == '\n')
		{
			continue;
		} else if (c == '#')
		{
			while (getc (fp) != '\n')
				;
			continue;
		} else if (!isdigit (c))
		{
			printf ("Read a '%c' in rule %d\n", c, i);
			break;
		} else
			ungetc (c, fp);

		if (fscanf (fp, "%d", &rule[i].base) != 1)
			break;

		if ((c = getc (fp)) != '\t')
		{
#ifdef COND
			rule[i].cond = c;
#endif COND
			while (getc (fp) != '\t')
				;
		}

		rule[i].rule = read_to_eol (fp);

dbg ("rule %d: %d %s\n", i, rule[i].base, rule[i].rule);

		i++;
	}
	nrules = i;
}

/*
 *	spell is the function called to spell a number.  It
 *	is initially called with condition 'I' (init).  This
 *	is a hack to get around the problem of when to pronounce
 *	0.  Spell essentially just figures out what the appropriate
 *	rule is, and calls evalrule() to do the work.
 */
spell (n, level)
	unsigned long n;
{
	register i;

	if (n == 0 && level)
		return;

	for (i = nrules - 1; rule[i].base > n; i--)
		;

	evalrule (rule[i].rule, rule[i].base, n, level);
}

/*
 * next
 *	This is a simple function to bounce around in strings
 *	with a syntax that includes balanced parens and double
 *	quotes. There's something like this in Icon, but this
 *	program is in C, so...
 */
char *next (s, c)
	char *s, c;
{
	register char *e;

	for (e = s; *e != c; e++)
	{
		if (*e == '"')
			e = next (e + 1, '"');
		if (*e == '(' && c != '"')
			e = next (e + 1, ')');
	}
	return (e);
}

/*
 *	evalrule does the dirty work.  It takes a rule, a
 *	base, and a number, and prints the number according
 *	to the rule.  Rules may use the following characters:
 *
 *	B	the base
 *	%	n % base
 *	/	n / base
 *	,	no-op
 *	"..."	for strings
 *
 *	conditionals are of the form:
 *
 *		(L C R \t rule)
 *
 *	where L and R are either a special character or a
 *	number, and C is one of '>', '<', '=' and '~', meaning,
 *	of course, less than, greater than, equal, and not equal.
 *	Conditionals are evaluated by doconditional(), which
 *	evaluates the condition, and, if it is true, evaluates
 *	the rule.
 *
 *	To give an example of a rule, taken from the grammar
 *	for mandarin:
 *
 *	10	/ "sh[']i" %
 *
 *	means that if the largest number that is smaller than
 *	the number we're trying to say is 10, then we say the
 *	number by saying the number divided by 10, followed
 *	by the word "sh[']i", followed by the remainder of the
 *	number divided by ten.  In other words, to say 23,
 *	you say (23 / 10) = 2, then "sh[']i", then (23 % 10) = 3,
 *	or 2 "sh[']i" 3.  After evaluating the rules for 2 and
 *	3, the string "e[`]r sh[']i s[^]an" is printed.
 */

evalrule (rule, base, n, level)
	char	*rule;
	unsigned long	n;
	int	base, level;
{	
	register j, c;

dbg ("evalrule (\"%s\", %d, %ld)\n", rule, base, n);

	while (c = *rule)
	{
		if (isdigit (c))
		{
			spell (atol (rule), level + 1);
			while (isdigit (*++rule))
				;
			continue;
		} else switch (c) {
		case ',':	break;
		case 'B':	spell ((long) base, level + 1);	break;
		case '%':	spell (n % base, level + 1);	break;
		case '/':	spell (n / base, level + 1);	break;

		case '"':	while ((c = *++rule) != '"')
					outchar (c);
				break;
		case '(':	docondition (rule, base, n, level);
				rule = next (rule + 1, ')');
				break;
		default:	for (j = 0; j < nspecials; j++)
				{
				    if (specials[j].c == c)
				    {
					evalrule (specials[j].rule, base,
						  n, level);
					break;
				    }
				}
				if (j == nspecials)
					outchar (c);
		}
		rule++;
	}
}
/*
 *	docondition evaluates conditionals, which are delimited
 *	by parentheses, and which contain two parts: a very
 *	simple Boolean expression and a rule.  The Boolean
 *	expression can, at the moment, only be a simple comparison.
 *	OR's (if the conditions are exclusive) can be done by
 *	putting multiple conditions in a row, and AND's by
 *	making the rule a conditional.  docondition calls
 *	parsecond (parse conditional) to pick out the various
 *	parts of the conditional, evaluates the comparison,
 *	and calls evalrule with the rule as an argument if the
 *	comparison evaluates to true.
 *
 *	Two additional special characters that are accepted here
 *	are:
 *
 *		L	Current recursion level
 *		#	The number itself
 */
docondition (rule, base, n, level)
	char	*rule;
	unsigned long	n;
	int	base, level;
{
	char	subrule [128];
	unsigned long	leftside, rightside;
	int	truth;
	char	comparator;

/*
 *	This is to check for bad grammars or buggy parser.
 */
	if (!parsecond (rule, base, n, level,
			&leftside, &comparator, &rightside, subrule))
	{
		printf ("Gagged on rule \"%s\"\n", rule);
		return;
	}

	switch (comparator) {
	case '>':	truth = leftside > rightside;	break;
	case '=':	truth = leftside == rightside;	break;
	case '<':	truth = leftside < rightside;	break;
	case '~':	truth = leftside != rightside;	break;
	}

dbg ("docondition (%d, %d, %d %c %d) -> %s\n",
	base, n, leftside, comparator, rightside,
	truth ? subrule : "FAILS");

	if (!truth)
		return;

	evalrule (subrule, base, n, level);
}

/*
 *	parsecond parses the rule according to the base,
 *	and assigns the parts to the variables passed
 *	as arguments.
 */

parsecond (rule, base, n, level, lp, cp, rp, subrule)
	char	*rule, *cp, *subrule;
	unsigned long	*lp, *rp, n;
	int	base, level;
{
	char	*index(), *rindex();
	register char *start, *end;
	char	leftstring[20], rightstring[20];

	if (sscanf (rule, "(%s %c %s", leftstring, cp, rightstring) != 3)
	{

dbg ("parsecond failed sscanf (\"%s\", ...)\n", rule);

		return (0);
	}

	*rp = parsenumber (rightstring, base, n, level);
	*lp = parsenumber (leftstring, base, n, level);

	if (!(start = index (rule, '\t')))
	{

dbg ("parsecond couldn't find a tab in \"%s\"\n", rule);

		return (0);
	}

	end = next (++start, ')');

	while (start < end)
		*subrule++ = *start++;
		
	*subrule = '\0';
	return (1);
}

/*
 *	parsenumber figures out the numerical value of the
 *	string that it is passed, based on the base and the
 *	number n.
 */

unsigned long parsenumber (s, base, n, level)
	unsigned long n;
	char *s;
{
	if (isdigit (s[0]))
		return (atoi (s));

	switch (s[0]) {
	case '/':	return (n / base);
	case '%':	return (n % base);
	case '#':	return (n);
	case 'L':	return (level);
	case 'B':	return (base);
	default:	fprintf (stderr, "bad number string \"%s\"\n", s);
			return (-1);
	}
}

/*
 *	outchar is a slightly clever version of putchar.  It
 *	won't put a space at the beginning of a line, and it
 *	won't put two spaces in a row.
 */

outchar (c)
{
	static	lastspace = 0,
		bol = 1;

	if ((lastspace || bol) && c == ' ')
		return;

	if (c == '\n')
		bol = 1;
	else
		bol = 0;

	if (c == ' ')
		lastspace = 1;
	else
		lastspace = 0;

	putchar (c);
}

/*
 *	Well, see, I had this bug, and I left my debugger in
 *	Chicago, and...
 */

dbg (fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	char *fmt, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9;
{
	int tmpdbgflag = dbgflag;

	if (dbgflag > 0)
	{
		dbgflag = 0;
		fprintf (stderr, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9);
		dbgflag = tmpdbgflag;
	}
}

chkdbg ()
{
	extern char *getenv ();
	register char *cp;

	if ((dbgflag == 0) && (cp = getenv ("DEBUG=")))
		dbgflag = atoi (cp);
}
