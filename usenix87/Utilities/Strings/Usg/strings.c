/*

    NAME
	strings - find the printable strings in an object, or
		  other binary, file

    SYNOPSIS
	strings [-a] [-o] [-l number] [file ...]

    This is a rewrite of the Berkeley strings(1) program. The original was
    very particular to the BSD object format and would have needed
    considerable changes rewriting.  I also changed the way the arguments
    were handled to use getopt. Also, standard input can now be read.

					Tony Hansen
*/

#include <stdio.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>

extern char *optarg;
extern int optind;
extern int getopt();
extern void exit();

static LDFILE *ldptr;			/* holds COFF header info */
static SCNHDR secthead;			/* holds COFF section info */
static int cutoff = 4;			/* minimum length of lines */
static int allflag = 0;			/* print all of binary */
static int proffset = 0;		/* print offsets */
static char *progname;			/* argv[0] */

main(argc, argv)
int argc;
char **argv;
{
    register int c;

    progname = argv[0];
    while ((c = getopt(argc, argv, "aol:")) != EOF)
	switch (c)
	    {
	    case 'a':	allflag++;		break;
	    case 'o':	proffset++;		break;
	    case 'l':	cutoff = atoi(optarg);	break;
	    default:
		(void) fprintf (stderr,
		    "usage: %s [-a] [-o] [-l n] [file ...]\n", progname);
		(void) fprintf (stderr, "\t-a\tlook at all of file\n");
		(void) fprintf (stderr, "\t-o\tprint offsets\n");
		(void) fprintf (stderr, "\t-l n\tminimum length of string\n");
		exit(1);
	    }

    if (cutoff < 1)		cutoff = 0;
    else if (cutoff > BUFSIZ)	cutoff = BUFSIZ - 1;
    else			cutoff--;

    if (optind == argc)
	find(-1L, stdin);
    else
	for ( ; optind < argc; optind++)
	    if (strcmp(argv[optind], "-") == 0)
		find(-1L, stdin);
	    else
		lookinfile(argv[optind]);

    return 0;
}

/*
    Look at the entire binary for strings
    or open up the file using the COFF routines to sift through
    the object module.
*/

lookinfile(filename)
register char *filename;
{
    int firsttime = 1;

    ldptr = NULL;

    if (allflag)
	{
	FILE *fnptr = fopen(filename, "r");
	if (fnptr == NULL)
	    (void) fprintf (stderr, "%s: cannot open %s.\n",
		progname, filename);
	else
	    {
	    find(-1L, fnptr);
	    (void) fclose(fnptr);
	    }
	}
    else
	for (;;)
	    {
	    if ((ldptr = ldopen(filename, ldptr)) == NULL)
		{
		if (firsttime)
		    (void) fprintf (stderr, "%s: cannot open %s.\n",
			progname, filename);
		break;
		}
	    if (processobject(filename) == FAILURE)
		{
		if (firsttime)
		    {
		    (void) fprintf (stderr,
			"%s: %s is not in Common Object File Format\n",
			progname, filename);
		    (void) FSEEK(ldptr, (long) 0, BEGINNING);
		    find(-1L, IOPTR(ldptr));
		    }
		break;
		}
	    firsttime = 0;
	    if (ldclose(ldptr) == SUCCESS)
		break;
	    }
}

/*
    Find the data sections of the COFF file and process them.
*/

processobject(filename)
register char *filename;
{
    register int i, nscns;

    switch (HEADER(ldptr).f_magic)
	{
	case N3BMAGIC:
	case FBOMAGIC:
	case RBOMAGIC:
	case VAXWRMAGIC:
	case VAXROMAGIC:
	case U370WRMAGIC:
	case U370ROMAGIC:
	    nscns = HEADER(ldptr).f_nscns;
	    for (i = 1;
		 i <= nscns && ldshread(ldptr, i, &secthead) == SUCCESS;
		 i++)
		if (secthead.s_flags & STYP_DATA)
		    lookatdata(filename);
	    return SUCCESS;
	default:
	    return FAILURE;
	}
}

/*
    Look at the data section for printables.
*/
lookatdata(filename)
char *filename;
{
    if (FSEEK(ldptr, secthead.s_scnptr, BEGINNING))
	{
	(void) fprintf(stderr,
	    "%s: fseek to data section failed in file %s!\n",
	    progname, filename);
	return;
	}
    find (secthead.s_size, IOPTR(ldptr));
}

/*
    Find groups of printable and non-printable characters.
*/
find(count, fptr)
register long count;
FILE *fptr;
{
    char buf[BUFSIZ];
    register char *bufptr = buf;
    register int c;
    
    while (count-- != 0)
        {
	c = fgetc(fptr);
	if ((c == '\n') ||
	    ((c < ' ') && (c != '\f') && (c != '\t') && (c != '\b')) ||
	    (c >= '\177') || (c == EOF))
	    {
	    if (bufptr > buf && bufptr[-1] == '\n')
	        bufptr--;
	    *bufptr++ = '\0';
	    if (bufptr > &buf[cutoff])
	        {
		if (proffset)
		    (void) printf ("%7ld ", ftell(fptr) - (bufptr - buf));
		(void) printf ("%s\n", buf);
		}
	    bufptr = buf;
	    }
	else
	    {
	    if (bufptr < &buf[sizeof(buf)-2])
	        *bufptr++ = c;
	    }
	if (c == EOF)
	    break;
	}
}
