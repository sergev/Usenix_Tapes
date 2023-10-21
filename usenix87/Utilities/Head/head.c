/* head - replace head command */
/*
 * call: head [ +n ] [ -n ] [ file ... ]
 *
 *       +n = number of lines to skip before starting display
 *            default = 0
 *       -n = number of lines to display, starting with top of file
 *            default = 10
 *       file = any number of files may be present
 *				if name is `-', read from standard input
 */
#include    <stdio.h>

#define FALSE 0
#define TRUE 1

void dofile ();

main (ac, av)
int     ac;
char    **av;
{
    register FILE *in = stdin;          /* default is standard input    */

    register int header = FALSE;        /* TRUE if multiple files       */

    register char *cp;                  /* general purpose pointer      */

    register long nlines = 10;          /* number of lines to display   */
    register long skip = 0;             /* number of lines to skip      */

    register char *pgm;                 /* program name                 */

    if (pgm = strrchr (av[0], '/')) pgm++;
    else                            pgm = av[0];

    /* ------------------------------------------------------------ */
    /* Adjust args to skip program name                             */
    /* ------------------------------------------------------------ */

    av++; ac--;

    /* ------------------------------------------------------------ */
    /* process arguments/options                                    */
    /*   it is assumed that the options will precede all file names */
    /*   on the command line                                        */
    /* ------------------------------------------------------------ */

    while (ac)
    {
        if (*av[0] == '+')              /* lines to skip            */
        {
            skip = atoi (&av[0][1]);
            ac--; av++;
        }
        else if (*av[0] == '-')         /* lines to display         */
        {
            nlines = atoi (&av[0][1]);
            ac--; av++;
        }
        else break;
    }

    /* ------------------------------------------------------------ */
    /* If any files, then process each                              */
    /* ------------------------------------------------------------ */

    if (ac)
    {
    /* ------------------------------------------------------------ */
    /* If more than one file name specified, display headers        */
    /* ------------------------------------------------------------ */

        header = ac > 1;
        while (ac--)
        {
            if (!strcmp (*av, "-"))
            {
                dofile (stdin, "standard input", skip, nlines, header);
            }
            else if (!(in = fopen (*av, "r")))
            {
                fprintf (stderr,
                    "head: cannot read file %s - skipping\n", *av);
            }
            else
            {
                dofile (in, *av, skip, nlines, header);
                fclose (in);
            }
            av++;
        }
    }
    else dofile (stdin, "standard input", skip, nlines, FALSE);

    exit (0);
}
/* ------------------------------------------------------------ */
/* Process a file, skipping and displaying as necessary         */
/* ------------------------------------------------------------ */
void
dofile (in, inname, skip, nlines, dohead)
register FILE *in;          /* input stream                         */
register char *inname;      /* input file name                      */
register long skip;         /* number of lines to skip              */
register long nlines;       /* number of lines to display           */
         int dohead;        /* TRUE if header is to be displayed    */
{
    register char *c;           /* convenient local pointer         */
    register long lineno = 0;   /* line number in file              */

    char    inbuf[1024];        /* line buffer                      */

    /* ------------------------------------------------------------ */
    /* Print header if necessary                                    */
    /* ------------------------------------------------------------ */

    if (dohead)
    {
        printf ("\
+-----------------------------------------------------------------+\n\
+                %-40.40s         +\n\
+-----------------------------------------------------------------+\n",
            inname);
    }

    /* ------------------------------------------------------------ */
    /* Skip lines                                                   */
    /* ------------------------------------------------------------ */

    while (lineno++ < skip && fgets (inbuf, sizeof inbuf, in));

    /* ------------------------------------------------------------ */
    /* Display lines                                                */
    /* ------------------------------------------------------------ */

    lineno = 0;

    while (lineno++ < nlines && fgets (inbuf, sizeof inbuf, in))
        fputs (inbuf, stdout);
    
    return;
}
