#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int		main ();		/* Report product of file sz and age */
static void	usage ();		/* Print a usage message */
static int	byteyears ();		/* Do it for one file */

enum timefield				/* Time field selection in statbuf */
    {
    ACCESS,
    CREATE,
    MODIFY
    };

#define YEAR	(365.25*86400)		/* Number of seconds in a year */
					/* (86400 is the number in a day) */

extern char *	optarg;			/* Current 'getopt' argument value */
extern int	optind;			/* Current 'getopt' option index */
extern long	time ();		/* Return system time */

int main (argc, argv)			/* Report product of file sz and age */
    register int	argc;		/* Argument count */
    register char *	argv[];		/* Argument vector */
    {
    int			digits = 0;	/* No. of significant digits to use */
    long		now;		/* The time right now */
    int			optch;		/* 'Getopt' argument character */
    int			status = 0;	/* Eventual return status */
    enum timefield	tfield = MODIFY; /* Which time field to use */

    while ((optch = getopt (argc, argv, "acmd:")) != EOF)
	{
	switch (optch)
	    {
	    case 'a':
		tfield = ACCESS;
		break;
	    case 'c':
		tfield = CREATE;
		break;
	    case 'm':
		tfield = MODIFY;
		break;
	    case 'd':
		digits = atoi (optarg);
		break;
	    default:
		usage ();
		return 2;
	    }
	}
    if (optind >= argc)
	{
	usage ();
	return 2;
	}
    now = time ((long *) NULL);
    for (  ;  optind < argc;  optind++)
	status |= byteyears (argv[optind], now, tfield, digits);
    return status;
    }

static void usage ()			/* Print a usage message */
    {
    (void) fprintf (stderr, "Usage:  byteyears -[acm] [-d n] files\n");
    }

/* Produce report for one file */
static int byteyears (name, now, tfield, digits)
    register char *	name;		/* Name of the file to report about */
    long		now;		/* The time right now */
    enum timefield	tfield;		/* Which field to work with */
    int			digits;		/* Number of significant digits to do */
    {
    double		byears;		/* Byte-year product */
    struct stat		statbuf;	/* File status, for size and age */

    if (stat (name, &statbuf) < 0)
	{
	perror (name);
	return 1;
	}
    switch (tfield)
	{
	case ACCESS:
	    now -= statbuf.st_atime;
	    break;
	case CREATE:
	    now -= statbuf.st_ctime;
	    break;
	case MODIFY:
	    now -= statbuf.st_mtime;
	    break;
	}
    byears = (double) statbuf.st_size * (double) now / YEAR;
    (void) printf ("%s\t%.*f\n", name, digits, byears);
    return 0;
    }
