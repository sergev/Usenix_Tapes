/*
** cshar.c
*/

/*
Shar puts readable text files together in a package
from which they are easy to extract.  The original version
was a shell script posted to the net, shown below:
        #Date: Mon Oct 18 11:08:34 1982
        #From: decvax!microsof!uw-beave!jim (James Gosling at CMU)
        AR=$1
        shift
        for i do
                echo a - $i
                echo "echo x - $i" >>$AR
                echo "cat >$i <<'!Funky!Stuff!'" >>$AR
                cat $i >>$AR
                echo "!Funky!Stuff!" >>$AR
        done
I rewrote this version in C to provide better diagnostics
and to run faster.  The main difference is that my version
does not affect any files because it prints to the standard
output.  Mine also has a -v (verbose) option.
*/
/*
 *      I have made several mods to this program:
 *
 *      1) the -----cut Here-----... now preceds the script.
 *      2) the cat has been changed to a sed which removes a prefix
 *      character from the beginning of each line of the extracted
 *      file, this prefix character is added to each line of the archived
 *      files and is not the same as the first character of the
 *      file delimeter.
 *      3) added several options:
 *              -c      - add the -----cut Here-----... line.
 *              -d'del' - change the file delimeter to del.
 *
 *                              Michael A. Thompson
 *                              Dalhousie University
 *                              Halifax, N.S., Canada.
 */

/*
 *      I fixed this for iRMX86 by removing all *IX type calls,
 *      in particular, this version cannot generate checksums.
 *      I've also added HI based command line parsing and overwrite
 *      checking.
 *
 *      Peter Kendell STC July 1986
 */

#include <stdio.h>
#include <udi.h>

#define TRUE            1
#define FALSE           0

#define TO              1
#define OVER            2
#define AFTER           3

#define DELIM           "SHAR_EOF"/* put after each file */
#define PREFIX1         'X'     /* goes in front of each line */
#define PREFIX2         'Y'     /* goes in front of each line if delim
                                    starts with PREFIX1 */
#define PREFIX          (delim[0] == PREFIX1 ? PREFIX2 : PREFIX1)
#define INPATHS         64      /* Max no of files to shar */

unsigned status;                /* result of RMX call */

static char delim[16],          /* option to provide alternate delimiter */
        progname[64];           /* This program's name */

char   *program;                /* The name used in error messages */

int     cut = FALSE,            /* option to provide -cut here- line */
        verbose = FALSE;        /* option to provide append/extract feedback */

FILE   *outfile;                /* Where the shared stuff goes */

extern char     *malloc (),
                rq$c$get$parameter (),
                rq$c$get$output$pathname ();

extern void     rq$c$get$input$pathname (),
                rq$c$get$command$name (),
                rq$s$delete$connection (),
                usage ();

extern connection rq$c$get$output$connection ();


void    dummy_exception_handler ()

/*
 *      Just to stop RMX barfing when a system call doesn't return E$OK
 */

{}


int     main ()

{
    int     result = FALSE,
            argc;

    char   *argv[INPATHS],
          **av,
            indexp,
            preposition;

    static char inpath[256],
            outpath[64],
            name[16],
            val[16];

    connection conn;

/*
 *      Kill the exception handler and get the name of this program.
 */

    dq$trap$exception (dummy_exception_handler, &status);
    rq$c$get$command$name (progname, (sizeof progname) - 1, &status);

    for (
            program = progname + strlen (progname);
            program >= progname;
            program--
        )
        if (*program == ':' || *program == '/')
        {
            program++;
            break;
        }

/*
 *      Get the inpathlist
 */

    rq$c$get$input$pathname (inpath, (sizeof inpath) - 1, &status);

    if (!strlen (inpath))   /* No input list */
        usage ();

/*
 *      Get and open the output file, checking a la RMX.
 */

    preposition = rq$c$get$output$pathname (outpath, (sizeof outpath) - 1,
                                                    "TO SHAR.OUT", &status);
    conn = rq$c$get$output$connection (outpath, 0, &status);

    if (status != E$OK)
        exit (1);

    rq$s$delete$connection (conn, &status);

    switch (preposition)
    {
        case TO:
        case OVER:
            if ((outfile = fopen (outpath, "w")) == NULL)
            {
                sprintf (stderr, "%s: fatal, can't write %s %s\n", program,
                         (preposition == TO) ? "to" : "over", outpath);
                exit (1);
            }
            break;

        case AFTER:
            if ((outfile = fopen (outpath, "a")) == NULL)
            {
                sprintf (stderr, "%s: fatal, can't write after %s\n",
                                                        program, outpath);
                exit (1);
            }
            break;

        default:
            break;
    }

/*
 *      Get the parameters
 */

    strcpy (delim, DELIM);

    while (rq$c$get$parameter (name, (sizeof name) - 1, val, (sizeof val) - 1,
                                                &indexp, (char *) 0, &status))
        if (strlen (name))
            if (*name == 'D' || *name == 'd')
                strcpy (delim, val + 1);
            else
                usage ();
        else
            if (*(val + 1) == 'c' || *(val + 1) == 'C')
                cut = TRUE;
            else
                if (*(val + 1) == 'v' || *(val + 1) == 'V')
                    verbose = TRUE;
                else
                    usage ();

/*
 *      Build the array of input file names
 */

    for (
            argc = 0;
            argc < INPATHS;
            rq$c$get$input$pathname (inpath, (sizeof inpath) - 1, &status),
            argc++
        )
    {
        if (!strlen (inpath))
            break;

        argv[argc] = malloc (strlen (inpath));
        strcpy (argv[argc], inpath);
    }

    if (header (argc, argv))
        exit (1);

    av = argv;

    while (argc--)
        result += shar (*av++);

    fputs ("exit\n", outfile);
    exit (result);
}


int     header (argc, argv)
int     argc;
char  **argv;

{
    int     i,
            problems = FALSE;

    for (i = 0; i < argc; i++)

        if (access (argv[i]))
        {
            fprintf (stderr, "%s: error, can't read %s\n", program, argv[i]);
            problems++;
        }

    if (problems)
        return (problems);

    if (cut)
        fputs ("-----Cut Here-----Cut Here-----Cut Here-----Cut Here-----\n",
                                                                    outfile);

    fputs ("#!/bin/sh\n", outfile);
    fprintf (outfile, "#\t%s: Shell Archiver\n", program);
    fputs ("#\tRun the following text with /bin/sh to create:\n#\n", outfile);

    for (i = 0; i < argc; i++)
        fprintf (outfile, "#\t    %s\n", argv[i]);

    return (0);
}


int     shar (file)
char   *file;

{
    char    line[BUFSIZ];
    FILE   *ioptr;

    if (ioptr = fopen (file, "r"))
    {
        if (verbose)
        {
            fprintf (stderr, "%s: appending %s\n", program, file);
            fprintf (outfile, "echo x - extracting %s\n", file);
        }

        fprintf (outfile, "sed 's/^%c//' << '%s' > %s\n", PREFIX, delim, file);

        while (fgets (line, BUFSIZ, ioptr))
        {
            fputc (PREFIX, outfile);
            fputs (line, outfile);
        }

        fclose (ioptr);
        fputs (delim, outfile);
        fputc ('\n', outfile);
        return (0);
    }
    else
    {
        fprintf (stderr, "%s: error, can't open %s\n", program, file);
        return (1);
    }
}


access (filename)
char   *filename;

/*
 *      All that's needed to work under RMX
 */

{
    FILE        *test;

    if ((test = fopen (filename, "r")) == NULL)
        return -1;
    else
    {
        fclose (test);
        return 0;
    }
}

void    usage ()

{
    fprintf (stderr,
"Usage: %s filelist [TO || OVER || AFTER outputfile] [D=delimiter], [V], [C]\n",
        program);
    exit (1);
}
