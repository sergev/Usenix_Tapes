/*T ctags - generate VI tags file from C sources */
/*S globals and stuff */
#include	<stdio.h>
#include	<ctype.h>

#define EQ ==
#define NE !=
#define GT >
#define GE >=
#define LT <
#define LE <=
#define OR ||
#define AND &&

#define TRUE 1
#define FALSE 0

#define SPACE ' '
#define NUL '\0'

typedef short   BOOL;

char	FunctionName[40];

char    *ReservedWord[]  = { 
     "BOOL", "auto", "bool", "break", "case", "char", "continue",
     "default", "do", "double", "else", "entry", "enum",
     "extern", "float", "for", "goto", "if",
     "int", "long", "register", "return", "short",
     "sizeof", "static", "struct", "switch",
     "typedef", "union", "unsigned", "void", "while",
     NULL };

char	*pgm;
char	inbuf[512];

extern char *strchr ();			/* index on BSD						*/
extern char *strrchr ();		/* rindex on BSD					*/
extern char *strpbrk ();		/* ???								*/

int 	skiptoend ();			/* skip to end of function			*/
char	ckfunc ();				/* check for function declaration	*/
/*Page Eject*/
main (ac, av)
int		ac;
char	*av[];
{
	register FILE *in;			/* input file stream				*/
	register FILE *tagfile;		/* `tags' file being created		*/

	register char *cp;			/* convenient character pointers	*/
	register char *ep;			/*									*/

	register int i;					/* loop index and such			*/
	register int index = FALSE;		/* if true, produce a functions	*/
									/* index instead of tags file	*/
	register int line_number = 0;	/* line number of declaration	*/
	register int optind = 1;		/* like getopt - command arg	*/

	char	*funcname;				/* function name				*/

	char	filename[15];			/* like it says					*/

	if (pgm = strrchr (av[0], '/')) pgm++;
	else 							pgm = av[0];

	/* ------------------------------------------------------------ */
	/* Create the tags file                                         */
	/* ------------------------------------------------------------ */

	tagfile = fopen ("tags", "w");
	if (!tagfile)
	{
		perror ("tags");
		fprintf (stderr,
			"%s: cannot create file tags\n", pgm);
		exit (1);
	}

	/* ------------------------------------------------------------ */
	/* If selected, write a function index to stdout instead        */
	/* ------------------------------------------------------------ */

	if (!strncmp (av[1], "-x", 2))
	{
		index = TRUE;
		optind++;
	}

	/* ------------------------------------------------------------ */
	/* Process each file specified on the command line              */
	/* ------------------------------------------------------------ */

	for (i = optind; i LT ac; i++)
	{
		if (!(in = fopen (av[i], "r")))
		{
			perror (av[i]);
			fprintf (stderr,
				"%s: unable to read file %s\n", pgm, av[i]);
		}
		else
		{
			while (fgets (inbuf, sizeof inbuf, in))
			{
				line_number++;
				if (ckfunc (inbuf))
				{
	/* ------------------------------------------------------------ */
	/* Found a function declaration                                 */
	/* ------------------------------------------------------------ */

					inbuf[strlen(inbuf)-1] = NUL;
					if (index)
					{
						printf ("%-20.20s  %6d  %s\t%s\n",
							FunctionName, line_number, av[i], inbuf);
					}
	/* ------------------------------------------------------------ */
	/* Special case function name `main'                            */
	/* ------------------------------------------------------------ */
					else if (!strcmp (FunctionName, "main"))
					{
						cp = strrchr (av[i], '/');
						if (cp) cp++;
						else cp = av[i];
						strcpy (filename, cp);
						ep = strrchr (filename, '.');
						if (ep) *ep = NUL;
						fprintf (tagfile, "M%s\t%s\t?^%s$?\n",
							filename, av[i], inbuf);
					}
					else
					{
						fprintf (tagfile, "%s\t%s\t?^%s$?\n",
							FunctionName, av[i], inbuf);
					}
					line_number += skiptoend (in);
				}
			}
			fclose (in);
		}
	}
	fclose (tagfile);

	/* ------------------------------------------------------------ */
	/* Tags file must be sorted alphabetically on tag               */
	/* ------------------------------------------------------------ */

	system ("sort -u -t'\t' +0 -1 -o tags tags");

	exit (0);
}
/*S ckfunc - check line for function declaration */
/*H ckfunc */
/*E*/

#define isidchr(c) (isalnum(c) OR (c EQ '_'))

char
ckfunc (s)
register char   *s;				/* string to check for declaration	*/
{
    register char *p;			/* convenient character pointer		*/
    register int  i;			/* useful loop index 				*/
    register int  result;		/* used to terminate search loop	*/
    register char found = FALSE;	/* true if declaration found	*/

    static char *_fnm = "ckfunc";

	/* ------------------------------------------------------------ */
	/* There's GOT to be a left paren for there to be a declaration */
	/* ------------------------------------------------------------ */

	if (strchr (s, '('))
	{
		found = TRUE;

		while (found)
		{
			found = FALSE;
			p = FunctionName;
			for (s; isascii (*s) AND isspace (*s) AND *s; s++);
			if( *s EQ '*' )
			{
				for (++s; isascii (*s) AND isspace (*s) AND *s; s++);
			}

			if ((*s EQ '_') OR isalpha(*s))
			{
				while (isidchr (*s)) *p++ = *s++;

				*p = NUL;

				for (found = FALSE, i = 0;
					 !found AND ReservedWord[i]; i++)
				{
					if (!(result = strcmp (FunctionName, ReservedWord[i])))
						found = TRUE;
					else if  (result LT 0) break;
				}
			}
		}
	}
	
	for (s; isascii (*s) AND isspace (*s) AND *s; s++);

	if (*s EQ '(')
	{
		for (found = FALSE; *s AND !found; s++)
			found = *s EQ ')';
		
		if (found)
		{
			for (; *s AND isspace (*s); s++);

			found = *s NE ';';
		}
	}

    return found;
}
/* ------------------------------------------------------------ */
/* Skiptoend is called after a function declaration is found    */
/* to skip to the end of the function.                          */
/* ------------------------------------------------------------ */
int
skiptoend (in)
register FILE *in;			/* file being processed					*/
{
	register int nest = 0;		/* nesting level - zero means end	*/
	register int c;				/* character being checked			*/
	register int line_number = 0;	/* like it says					*/

	/* ------------------------------------------------------------ */
	/* Simpleminded search for the starting brace                   */
	/* ------------------------------------------------------------ */

	while (((c = getc (in)) NE EOF) AND c NE '{')
		if (c EQ '\n') line_number++;

	nest++;

	/* ------------------------------------------------------------ */
	/* Keep going until the paired brace is found                   */
	/* ------------------------------------------------------------ */

	while (nest AND ((c = getc (in)) NE EOF))
	{
		switch (c)
		{
			case	'{':	nest++; break;
			case	'}':	nest--; break;
			case	'\n':	line_number++;
		}
	}

	return line_number;
}
