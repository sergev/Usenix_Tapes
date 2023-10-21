/*
 * Print a list of numbers.
 * See the manual entry for details.
 */

#include <stdio.h>


/*********************************************************************
 * MISCELLANEOUS GLOBAL VALUES:
 */

#define	PROC				/* null; easy to find procs */
#define	FALSE	0
#define	TRUE	1
#define	CHNULL	('\0')
#define	CPNULL	((char *) NULL)
#define	REG	register

char *usage[] = {
    "usage: %s [-lr] [-s size] [-w width] value [value2]",
    "-l print all one one line (default: one number per line)",
    "-r print numbers in random order",
    "-s set step size (increment, default = 1)",
    "-w print numbers in fields of given width (default: no padding)",
    "Prints numbers from 1 to value, or value to value2",
    CPNULL,
};

char	*myname;			/* how program was invoked	*/

int	lflag = FALSE;			/* -l (all one line) option	*/
int	rflag = FALSE;			/* -r (randomize) option	*/

int	step  = 1;			/* value from -s option		*/
int	width = 0;			/* value from -w option		*/


/************************************************************************
 * M A I N
 */

PROC main (argc, argv)
	int	argc;
	char	**argv;
{
extern	int	optind;			/* from getopt()	*/
extern	char	*optarg;		/* from getopt()	*/
REG	int	option;			/* option "letter"	*/
REG	int	value1 = 1;		/* first value		*/
REG	int	value2;			/* second value		*/
	char	format [20];		/* for printing numbers	*/
REG	char	sep;			/* leading separator	*/

/*
 * PARSE ARGUMENTS:
 */

	myname = *argv;

	while ((option = getopt (argc, argv, "lrs:w:")) != EOF)
	{
	    switch (option)
	    {
	    case 'l':	lflag = TRUE;	break;
	    case 'r':	rflag = TRUE;	break;

	    case 's':	step  = atoi (optarg);  break;
	    case 'w':	width = atoi (optarg);  break;

	    default:	Usage();
	    }
	}

	argc -= optind;			/* skip options	*/
	argv += optind;

/*
 * MORE ARGUMENT CHECKS:
 */

	if (argc == 1)
	{
	    value2 = atoi (argv [0]);
	}
	else if (argc == 2)
	{
	    value1 = atoi (argv [0]);
	    value2 = atoi (argv [1]);
	}
	else
	{
	    Usage();
	}

	if (step == 0)
	    Error ("step size cannot be zero");

/*
 * FINISH INITIALIZING:
 */

	if (((step < 0) && (value1 < value2))
	 || ((step > 0) && (value1 > value2)))	/* harmless if values equal */
	{
	    step = -step;			/* invert */
	}

	sprintf (format, (width ? "%%%dd" : "%%d"), width);
	sep = lflag ? ' ' : '\n';

/*
 * PRINT NUMBERS:
 */

	if (rflag)
	    PrintRandom (value1, value2, format, sep);
	else
	{
	    REG int forward = (step > 0);

	    printf (format, value1);

	    while (forward ? ((value1 += step) <= value2)
			   : ((value1 += step) >= value2))
	    {
		putchar (sep);
		printf (format, value1);
	    }
	}

	putchar ('\n');		/* finish last line */
	exit (0);

} /* main */


/************************************************************************
 * P R I N T   R A N D O M
 *
 * Given value limits, print format, leading separator, and global step size
 * (positive or negative matching value limits), print numbers in random
 * order.  Don't print the trailing newline for the last line.
 */

PROC PrintRandom (value1, value2, format, sep)
REG	int	value1;		/* base limit	*/
	int	value2;		/* end limit	*/
REG	char	*format;	/* how to print	*/
REG	char	sep;		/* leading char	*/
{
REG	int	remain = (value2 - value1) / step;
REG	int	count  = remain + 1;	/* total to print	*/
REG	int	*done;			/* this number printed?	*/
REG	int	which;			/* index in done[]	*/

/*
 * INITIALIZE:
 */

	if ((done = (int *) calloc (count, sizeof (int))) == (int *) NULL)
	    Error ("calloc %d integers failed", count);

	srand48 (time ((int *) 0) + getpid() + getuid() + getgid());

	done [which = (lrand48() % count)] = TRUE;
	printf (format, value1 + (which * step));	/* first one */

/*
 * PRINT RANDOM NUMBERS:
 */

	while (remain-- > 0)			/* more to do */
	{
	    which = lrand48() % count;		/* not *precisely* random */

	    while (done [which])		/* already taken */
	    {
		if (--which < 0)		/* wrap around */
		    which = count - 1;
	    }

	    done [which] = TRUE;
	    putchar (sep);
	    printf (format, value1 + (which * step));
	}

} /* PrintRandom */


/************************************************************************
 * U S A G E
 *
 * Print usage messages (char *usage[]) to stderr and exit nonzero.
 * Each message is followed by a newline.
 */

PROC Usage()
{
REG	int	which = 0;		/* current line */

	while (usage [which] != CPNULL)
	{
	    fprintf (stderr, usage [which++], myname);
	    putc ('\n', stderr);
	}

	exit (1);

} /* Usage */


/************************************************************************
 * E R R O R
 *
 * Print an error message to stderr and exit nonzero.  Message is preceded
 * by "<myname>: " using global char *myname, and followed by a newline.
 */

/* VARARGS */
PROC Error (message, arg1, arg2, arg3, arg4)
	char	*message;
	long	arg1, arg2, arg3, arg4;
{
	fprintf (stderr, "%s: ", myname);
	fprintf (stderr, message, arg1, arg2, arg3, arg4);
	putc ('\n', stderr);

	exit (1);

} /* Error */
