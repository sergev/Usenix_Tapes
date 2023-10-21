/* geneal - manipulate family trees, etc. */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 08:17:08 by jimmc (Jim McBeath) */
/* last edit 10-Sept-85 07:16:00 by tlr (Terry L. Ridder) */

#include "geneal.h"
#include <ctype.h>

#define HELP_EXIT 2

extern char *getData(), *rindex();

char *Progname;				/* name of the program 		     */
extern int dataDebug;			/* flag for debugging data routines  */
extern int indexDebug;			/* flag for debugging index routines */

struct dpoint *initDataFile();
struct dpoint *gendp;			/* data pointer for file 	     */
char *gendatfile="genealogy.dat";	/* filename for data file 	     */
int indexes=0;				/* set means output index numbers    */

main(argc, argv)
int 	argc;
char 	*argv[];
{
int i, j;
int nval = 1;
int nsac = 0;		/* non-switch argument count */
int fampage = 0;	/* set means user wants a family info page */
int sflag = 0;		/* set means use short form */
int iflag = 0;		/* set means produce individual data page */
int tflag = 0;		/* set means produce a tree */
int t;			/* for status values */

    Progname = rindex(argv[0], '/');	/* program name */
    if (Progname == 0)
    {
  	 Progname = argv[0];
    }
    else
    {
   	 Progname++;
    }
    nval = 0;		/* stays 0 if the user never asks for a number */
    for (i = 1; i < argc; i++)		/* scan switches and args */
    {
	if (argv[i][0] == '-') for (j = 1; j > 0 && argv[i][j]; j++)
		/* check switches */
	switch (argv[i][j])
	{
	case 'f':			/* produce family info page */
	    fampage++;
	    break;
	case 'h':			/* give help */
	    genhelp();
	    exit(HELP_EXIT);		/* don't continue */
	case 'i':			/* individual information page */
	    iflag++;
	    break;
	case 's':			/* short form */
	    sflag++;
	    break;
	case 't':			/* produce a tree */
	    tflag++;
	    break;
	case 'D':			/* debug data routines */
	    dataDebug++;
	    break;
	case 'F':
	    if (argv[i][j+1])
	    {
		 gendatfile = argv[i]+(j+1);
	    }
	    else if (argv[++i])
	    {
		 gendatfile = argv[i];
	    } 
	    else
	    {
		 fatalerr("no argument for -F switch");
	    }
	    j = -2;			/* flag this string is used up */
	    break;
	case 'I':			/* debug index routines */
	    indexDebug++;
	    break;
	case 'N':			/* output index numbers */
	    indexes++;
	    break;
	default:
	    fatalerr("unknown switch %c in %s", argv[i][j], argv[i]);
	}
	else				/* non-switch args */
	{
	    switch (nsac++)		/* non-switch arg count */
	    {
	    case 0:			/* first arg is number to use */
		if (isdigit(argv[i][0]))
		{
		    nval = atoi(argv[i]);
		}
		else
		{
		    fatalerr("bad format for arg0 (must be a number)");
		}
		break;
	    default:			/* too many */
		fatalerr("unknown argument %s", argv[i]);
	    }
	}
    }
    if (fampage + iflag + tflag > 1)
    {
	fatalerr("only one of -f, -i or -t may be selected");
    }
    gendp = initDataFile(gendatfile);	/* get data file */
    if (gendp == 0) 
    {
        fatalerr("can't open data file %s", gendatfile);
    }
/* check for combinations which are not yet implemented */
    if (fampage && sflag) 
    {
        warning("short form for family is not implemented");
    }
    if (iflag && !sflag) 
    {
        warning("only short form available for individuals");
    }
    if (fampage)                    	/* produce a family page if requested */
    {
	t = family(nval);
    }
    else if (iflag)                    	/* produce info about individual */
    {
	t = indivs(nval);
    }
    else if (tflag)                   	/* produce a family tree */
    {
	t = famtree(nval);
    }
/* if no specific action was requested, do the default */
    else               			/* go browsing */
    {
	t = gbrowse();
    }
    if (t == 0)				/* if no errors, exit w/o errors */
    {
	exit(0);
    }
    else
    {
	exit(1);			/* errors */
    }
}

/*........../

/* the help table */
char *helptab[] = {
"arg0 is the ID number of interest",
"Switches:",
"-f   produce a family information page",
"-h   give this help message",
"-i   produce an individual (person) information page",
"-s   use a short form for the specified information",
"-t   produce a tree (this function is quite incomplete!)",
"-D   debug data routines",
"-F file   use the specified file as datafile",
"-I   debug index routines",
"-N   output internal index numbers with all names",
"If no switches are given, the program enters browse mode.",
0
};

genhelp()		/* give help */
{
int i;

    for (i = 0; helptab[i]; i++)
    {
	printf("%s\n", helptab[i]);		/* print out the help table */
    }
    return;
}

/* end */
