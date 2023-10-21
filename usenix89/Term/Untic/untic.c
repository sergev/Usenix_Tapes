/*
 * Untic.c -- Uncompile a terminfo file
 *
 * Usage:
 *	untic terminal_name . . .
 *
 * This program finds the terminal description in
 *	/usr/lib/terminfo/?/terminal_name
 * It then converts the information into an ASCII file suitable for
 * running into "tic".  The resulting file is written to standard output.
 *
 * Compile by:
 *	cc -O -o untic untic.c
 *
 * It is probably a good idea to ensure that the file produced will compile
 * to the original file before trusting "untic".
 *
 * Structure of terminfo file:
 *	short	magic number
 *	short	length of terminal names + NUL
 *	short	length of booleans
 *	short	length of numerics
 *	short	length of strings
 *	short	length of string table
 *	chars	NUL terminated terminal name
 *	chars	boolean flags for each of the possible booleans
 *	shorts	values for each of the possible numerics
 *	shorts	offsets (from start of strings) for each of the
 *		possible strings
 *	chars	NUL terminated strings for each of the defined strings
 *
 * Most of the variables are in the order that the documentation lists
 * them.  This is important, as the information is stored in the file
 * based upon the ordinal position of the variable.  Some of the string
 * variables are not in order.  Presumably, if they add more variables,
 * it will be to the end of the list, and not in the middle.
 *
 * This has been tested on
 *	Plexus P20 (M68010), System 5 Release 2 (I think)
 *
 * Bugs:
 *	The longest string capability is limited to 4096 bytes.  If a longer
 *	string is encountered, the program will do unpredicatable things.
 *	(Who uses strings that long anyway?)  The longest that the terminfo
 *	file can be is 4096 bytes anyway, so this isn't too big a problem.
 *
 * Credits:
 *	Written by Dave Regan
 *	orstcs!regan
 *	16 May 86
 *
 *	I disclaim that this program does anything useful.  It might also
 *	do accidental damage.  Backup your original terminfo files.
 *
 *	This program is public domain.  That means you can do anything
 *	you want with it.
 */


#include <stdio.h>
#ifdef QC
#define	OPEN_MODE	"rb"	/* Mode used to open terminfo files	*/
#define	void	int		/* Sigh . . .				*/
#else		/* QC */
#include <ctype.h>
#define	OPEN_MODE	"r"	/* Mode used to open terminfo files	*/
#endif		/* QC */
#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif		/* TRUE */

#define	DEBUG	FALSE		/* TRUE/FALSE to enable debugging output */

#ifdef BSD
#define	TERMINFO_DIR	"/usr/static/sys5r2v2/usr/lib/terminfo"
#endif		/* BSD */
#ifndef TERMINFO_DIR
#define	TERMINFO_DIR	"/usr/lib/terminfo"
#endif		/* TERMINFO_DIR */


#define	MAGIC	0x011A		/* Terminfo magic number		*/
#define	MAXLINE	65		/* Longest emited line			*/
#define	MAX_CAP	4096		/* Longest single capability		*/

extern	char *strcpy();

int	line_len;		/* Current length of line		*/

main(argc, argv)
  int	argc;			/* Number of paramters			*/
  char	*argv[];		/* The parameters themselves		*/
    {
    char subdir[2];		/* Subdirectory name			*/
    FILE *file;
    extern FILE *fopen();

#ifdef CPM
    wildexp(&argc, &argv);
#endif		/* CPM */

    /* Change directory to the working directory			*/
    (void)chdir(TERMINFO_DIR);

    /* Go through the arguments						*/
    subdir[1] = '\0';
    if (argc == 1)
	convert(stdin, "stdin");
    else
	{
	while (--argc)
	    {
	    ++argv;
	    subdir[0] = argv[0][0];
	    (void)chdir(subdir);
	    if ((file = fopen(*argv, OPEN_MODE)) == NULL)
		{
		perror(*argv);
		}
	    else
		{
		convert(file, *argv);
		(void)fclose(file);
		}
	    (void)chdir("..");
	    }
	}
    }


/*
 * Addchar -- Add a character
 */
char *
addchar(cptr, ch)
  char	*cptr;
  int	ch;
    {
    char *addstr(), *addoctal();

    if (ch == 0x1B)
	return (addstr(cptr, "\\E"));
    if (ch == '\n')
	return (addstr(cptr, "\\n"));
    if (ch == '\r')
	return (addstr(cptr, "\\r"));
    if (ch == '\t')
	return (addstr(cptr, "\\t"));
    if (ch == '\b')
	return (addstr(cptr, "\\b"));
    if (ch == '\f')
	return (addstr(cptr, "\\f"));
    if (ch == ' ')
	return (addstr(cptr, "\\s"));
    if (ch == '^')
	return (addstr(cptr, "\\^"));
    if (ch == '\\')
	return (addstr(cptr, "\\\\"));
    if (ch == ',')
	return (addstr(cptr, "\\,"));
    if (ch == ':')
	return (addstr(cptr, "\\:"));
    if (ch >= ('A' - '@') && ch <= ('Z' - '@'))
	{
	*cptr++ = '^';
	*cptr++ = ch + '@';
	return (cptr);
	}
#ifdef notdef
    /*
     * Did you know that \r \n \f \t are NOT control characters
     * as defined by "iscntrl" under BSD 4.2?  I find that
     * rather odd.
     */
#endif		/* notdef */
    if (iscntrl(ch) || isspace(ch) || ch > 0x7F)
    	return (addoctal(cptr, ch));
    *cptr++ = ch;
    return (cptr);
    }


/*
 * Addoctal -- Add an octal character
 *
 * Use sprintf just in case "0" through "7" are not contiguous.  Some
 * machines are weird.
 */
char *
addoctal(cptr, ch)
  char	*cptr;
  int	ch;
    {
    char *addstr();

    ch &= 0xFF;

    if (ch == 0x80)
	return (addstr(cptr, "\\0"));
    (void)sprintf(cptr, "\\%03o", ch);
    while (*cptr != '\0')
	cptr++;
    return (cptr);
    }


/*
 * Addstr -- Add a string to the capability
 */
char *
addstr(cptr, str)
  char	*cptr, *str;
    {
    while (*str)
	*cptr++ = *str++;
    return (cptr);
    }


/*
 * Convert -- Do the actual conversion
 */
convert(file, name)
  FILE	*file;		/* The file with the compiled information	*/
  char	*name;		/* Printable version of the filename		*/
    {
    int	ch, val, i, j, fail;
    int name_length, bool_length, num_length, str_length, s_length;
    char capability[MAX_CAP+1], *cptr, *addchar();

    static char *booleans[] =	/* Names of boolean variables, in order	*/
	{ "bw", "am", "xsb", "xhp", "xenl", "eo", "gn", "hc", "km",
	  "hs", "in", "da", "db", "mir", "msgr", "os", "eslok", "xt",
	  "hz", "ul", "xon" };
    static char *numerics[] =	/* Names of numeric variables, in order	*/
	{ "cols", "it", "lines", "lm", "xmc", "pb", "vt", "wsl" };
    static char *strings[] =	/* Names of string variables, not in strict
				   order.  Makes things a little harder	*/
	{ "cbt", "bel", "cr", "csr", "tbc", "clear", "el", "ed", "hpa",
	  "cmdch", "cup", "cud1", "home", "civis", "cub1", "mrcup",
	  "cnorm", "cuf1", "ll", "cuu1", "cvvis", "dch1", "dl1", "dsl",
	  "hd", "smacs", "blink", "bold", "smcup", "smdc", "dim", "smir",
	  "invis", "prot", "rev", "smso", "smul", "ech", "rmacs", "sgr0", 
	  "rmcup", "rmdc", "rmir", "rmso", "rmul", "flash", "ff", "fsl",
	  "is1", "is2", "is3", "if", "ich1", "il1", "ip", "kbs", "ktbc",
	  "kclr", "kctab", "kdch1", "kdl1", "kcud1", "krmir", "kel", "ked",
	  "kf0", "kf1", "kf10", "kf2", "kf3", "kf4", "kf5", "kf6", "kf7",
	  "kf8", "kf9", "khome", "kich1", "kil1", "kcub1", "kll", "knp",
	  "kpp", "kcuf1", "kind", "kri", "khts", "kcuu1", "rmkx", "smkx", 
	  "lf0", "lf1", "lf10", "lf2", "lf3", "lf4", "lf5", "lf6", "lf7",
	  "lf8", "lf9", "rmm", "smm", "nel", "pad", "dch", "dl", "cud",
	  "ich", "indn", "il", "cub", "cuf", "rin", "cuu", "pfkey", "pfloc",
	  "pfx", "mc0", "mc4", "mc5", "rep", "rs1", "rs2", "rs3", "rf",
	  "rc", "vpa", "sc", "ind", "ri", "sgr", "hts", "wind", "ht", "tsl",
	  "uc", "hu", "iprog", "ka1", "ka3", "kb2", "kc1", "kc3", "mc5p"
#ifdef HPUX5
	  , "meml", "memu"
#endif
	  };
    int str_cap[sizeof(strings) / sizeof(char *)];

    /* Check the magic number out					*/
    if (get2(file) != MAGIC)
	{
	fprintf(stderr, "\"%s\" is not a terminfo file\n", name);
	return;
	}

    /* Get the rest of the header information				*/
    name_length = get2(file);	/* Get the length of the terminal names	*/
    bool_length = get2(file);	/* Get the length of the booleans	*/
    num_length = get2(file);	/* Get the length of the numerics	*/
    str_length = get2(file);	/* Get the length of the strings	*/
    s_length = get2(file);	/* Get the length of the string tables	*/

    /* Check for too many data items					*/
    fail = FALSE;
    if (bool_length > (sizeof(booleans) / sizeof(char *)))
	{
	fprintf(stderr, "Boolean variables have been added to terminfo.\n");
	fail = TRUE;
	}
    if (num_length > (sizeof(numerics) / sizeof(char *)))
	{
	fprintf(stderr, "Numeric variables have been added to terminfo.\n");
	fail = TRUE;
	}
    if (str_length > (sizeof(strings) / sizeof(char *)))
	{
	fprintf(stderr, "String variables have been added to terminfo.\n");
	fail = TRUE;
	}
    if (fail)
	{
	fprintf(stderr,
"Update the \"untic\" program.  Use \"xxx\" if needed.  Good luck.\n");
	return;
	}

    /* Time to get real information					*/
    while ((ch = getc(file)) != '\0' && ch != EOF)
	putchar(ch);
    printf(",\n\t");

    /* Send out the non-null boolean variables				*/
    line_len = 0;
    for (i = 0; i < bool_length; i++)
	{
	if ((ch = getc(file)) != 0)
	    emit(booleans[i]);
	}

    /* The rest of the file is on a 16 bit boundary, so adjust the file	*/
    if ((name_length + bool_length) & 0x01)
	(void)getc(file);

    /* Get the numeric variables					*/
    for (i = 0; i < num_length; i++)
	{
	if ((val = get2(file)) != 0xFFFF)
	    {
	    (void)sprintf(capability, "%s#%d", numerics[i], val);
	    emit(capability);
	    }
	}

    /* Get the string variables offsets					*/
    for (i = 0; i < str_length; i++)
	str_cap[i] = get2(file);

    /* Get the string variables themselves				*/
    for (i = 0; i < s_length; i++)
	{
	for (j = 0; j < str_length; j++)	/* Find the name	*/
	    if (str_cap[j] == i)
		break;
#if DEBUG
	if (j >= str_length)
	    fprintf(stderr, "Cannot find address %d\n", i);
#endif		/* DEBUG */
	(void)strcpy(capability, strings[j]);
	cptr = &capability[strlen(capability)];
	*cptr++ = '=';
	for (; (ch = getc(file)) != '\0' && ch != EOF; i++)
	    cptr = addchar(cptr, ch);
	*cptr = '\0';
	emit(capability);
	}

    printf("\n\n");
    }


/*
 * Emit -- Emit the string
 *
 * Emit the given string, and append a comma.  If the line gets too long,
 * send out a newline and a tab.
 */
emit(str)
  char	*str;		/* String to emit				*/
    {
    if ((line_len += strlen(str) + 2) > MAXLINE)
	{
	line_len = strlen(str) + 2;
	printf("\n\t");
	}
    printf("%s, ", str);
    }


/*
 * Get2 -- Get a two byte number
 */
get2(file)
  FILE *file;		/* The file with the compiled information	*/
    {
    short temp;

    temp = getc(file) & 0xFF;
    return (temp + (getc(file) << 8));
    }


#ifdef CPM
chdir(str)
  char	*str;
    {
    }

perror(str)
  char	*str;
    {
    fprintf(stderr, "Cannot open \"%s\"\n", str);
    }
#endif		/* CPM */
