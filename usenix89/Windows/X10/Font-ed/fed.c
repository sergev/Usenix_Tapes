/*
#
# $Source: /local/projects/X/fed/RCS/fed.c,v $
# $Header: fed.c,v 1.13 87/04/08 08:49:05 jim Exp $
#
#                     Copyright (c) 1987 Cognition Inc.
#
# Permission to use, copy, modify and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies, and that both
# that copyright notice and this permission notice appear in supporting
# documentation, and that the name of Cognition Inc. not be used in
# advertising or publicity pertaining to distribution of the software
# without specific, written prior permission.  Cognition Inc. makes no
# representations about the suitability of this software for any purpose.
# It is provided "as-is" without express or implied warranty.
#
#							  Jim Fulton
#							  Cognition Inc.
#                                                         900 Tech Park Drive
# uucp:  ...!{mit-eddie,talcott,necntc}!ci-dandelion!jim  Billerica, MA
# arpa:  jim@athena.mit.edu, fulton@eddie.mit.edu         (617) 667-4800
#
*/


/*
 * fed - Font EDitor for DEC standard fonts
 */

#include "font.h"
#include "elem.h"
#include <strings.h>
#include <sys/stat.h>

int quiet = 0;
int verbose = 0;
int low = 0;		/* lowest numbered font element */
int high = 255;		/* highest numbered font element */

#define assignBasename(dstvar,srcstring) { \
	    (dstvar) = rindex (srcstring, '/'); \
	    (dstvar) = ((dstvar) ? (dstvar) + 1 : srcstring); }

#define get_string_arg(msg,var) {		\
		if (argc <= 1) Usage (msg);	\
		var = *++argv;			\
		argc--;				\
		}

#define get_int_arg(msg,var) {		\
		if (argc <= 1) Usage (msg);	\
		var = atoi (*++argv);		\
		argc--;				\
		}

#define get_octal_arg(msg,var) {		\
		if (argc <= 1) Usage (msg);	\
		var = aotoi (*++argv);		\
		argc--;				\
		}

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#define fedNULL			0
#define fedCOMPILE		1
#define fedDECOMPILE		2
#define fedINFO			3
#define fedPRINT		4

Usage (message)
	char *message;
{
		fprintf (stderr, 
"%s:  error with '%s'.\nUsage:\n\t%s  [flags]  inputfile",
			 ProgramName, message, ProgramName);
		fprintf (stderr, 
"\nwhere [flags] are:\n");
		fprintf (stderr, 
"    -c                Compile font inputfile.\n");
		fprintf (stderr,
"    -d                Decompile directory inputfile.\n");
		fprintf (stderr,
"    -i                Information on the font.\n");
		fprintf (stderr,
"    -p                Print font bitmap to stdout rotated -90 degrees.\n");
		fprintf (stderr,
"    -o outputfile     Output into outputfile.\n");
		fprintf (stderr,
"    -[l,L] number     Lowest number to compile or decompile [0].\n");
		fprintf (stderr,
"    -[h,H] number     Highest number to compile or decompile [255].\n");
		fprintf (stderr,
"    -v                Verbose.\n");
		fprintf (stderr,
"    -q                Quiet.\n");
		fprintf (stderr,
"\n");
		fprintf (stderr, 
"The uppercase flags -L and -H take their arguments in octal.\n");
		fprintf (stderr,
"\n");
	exit (1);
}


main (argc, argv)
	int argc;
	char *argv[];
{
	char *inputfile, *outputfile;
	int compile_direction;

	setlinebuf (stderr);
	inputfile = outputfile = NULL;
	compile_direction = NULL;

	assignBasename (ProgramName, argv[0]);

	argc--; argv++;		/* ignore command (argv[0]) */
	for (; argc > 0; argc--, argv++) {	/* while there are args... */
	    if (**argv == '-') {		/* if -flag */
		register char *cp = *argv;
		for (cp++; *cp != '\0'; cp++) {		/* scan flags */
		    switch (*cp) {
			case 'c':
			    compile_direction = fedCOMPILE;
			    break;
			case 'd':
			    compile_direction = fedDECOMPILE;
			    break;
			case 'i':
			    compile_direction = fedINFO;
			    break;
			  case 'p':
			    compile_direction = fedPRINT;
			    break;
			case 'o':
			    get_string_arg ("-o outputfile", outputfile);
			    break;
			case 'l':
			    get_int_arg ("-l number", low);
			    break;
			case 'h':
			    get_int_arg ("-h number", high);
			    break;
			case 'L':
			    get_octal_arg ("-l number", low);
			    break;
			case 'H':
			    get_octal_arg ("-h number", high);
			    break;
			case 'v':
			    verbose++;
			    break;
			case 'q':
			    quiet++;
			    break;
			default:
			    Usage ("unknown flag");
			    break;
		    } 						/*end switch*/
		} 						   /*end for*/
	    } else {	    /* got something beside a -flag on command line */
		if (inputfile) Usage ("extra parameter");
		else inputfile = *argv;
	    } 							    /*end if*/
	} /*end for */

	if (!inputfile) Usage ("no inputfile given");

	switch (compile_direction) {
	    case fedCOMPILE:
		compile (inputfile, outputfile);
		break;
	    case fedDECOMPILE:
		decompile (inputfile, outputfile);
		break;
	    case fedINFO:
		dumpheader (inputfile);
		break;
	    case fedPRINT:
		print (inputfile, outputfile);
		break;
	    default:
		Usage ("you must specify -c, -d, -i, or -p");
	}
	exit (0);
}

print (fontfile, outputfile)
    char *fontfile;
    char *outputfile;
{
    FONT *f;
    FILE *outfp;

    f = GetFont (fontfile);
    if (!f) Error ("getting font in print", fontfile);

    if (!outputfile)
      outfp = stdout;
    else {
	outfp = fopen (outputfile, "w");
	if (!outfp) Error ("opening outputfile", outputfile);
    }

    fontutil_printbitmap (f, outfp, !quiet);
}

dumpheader (fontfile)
	char *fontfile;
{
	FONT *f;

	f = GetFont (fontfile);
	if (!f) Error ("getting font in dumpheader", fontfile);
	fontutil_dumpheader (f);
}	    

compile (fontdirectory, fontfile)
	char *fontdirectory;
	char *fontfile;
{
	FontElementList *elemlist;	/* list of font elements (characters */
	FontElement *elem;		/* a random font element */
	FONT _f;			/* the FONT structure to build */
	FONT *f;			/* pointers are used everywhere else */
	FontElement **elemarray, **elemp;	/* for sorting array */
	int x_hot, y_hot, n;
	char cwd[256], *getwd();
	FILE *elemfile;

	f = &_f;
	assignBasename (f->name, fontdirectory);
	f->first = f->last = f->space = f->height = f->avg_width = 
		f->fixed = f->base = 0;
	f->data = (caddr_t) NULL;

	if (!getwd (cwd)) {
	    Error ("can't get the working directory", "");
	}

	if (chdir (fontdirectory) < 0) {
	    Error ("can't chdir to font directory", fontdirectory);
	}

	read_profile (f);
	elemlist = read_directory (".");    /* since we chdir'ed */
	elemarray = (FontElement **) malloc (elemlist->ncomponents *
						(sizeof (FontElement *)));

		/*
		 * Read in the bitmaps for all the fonts.
		 */

	elemp = elemarray;
	for (elem = elemlist->head; elem; elem = elem->next) {
	    elemfile = fopen (elem->filename, "r");
	    elem->raster = ReadBitmapFile (elemfile,
				&elem->width, &elem->height,
				&x_hot, &y_hot,
				&elem->size);
	    if (!elem->raster) 
		Error ("unable to read in element bitmap", elem->filename);

	    *elemp++ = elem;
	    fclose (elemfile);
	}

		/*
		 * and restore the directory 
		 */


	if (chdir (cwd) < 0) {
	    Error ("can't chdir back to original directory", cwd);
	}

		/*
		 * Sort the element array so that the characters are in
		 */

	n = sizeof (FontElement *);
	qsort ((char *) elemarray, elemlist->ncomponents, n, elemcompar);

		/*
		 * We now have part of the FONT information read in, and we
		 * have a sorted list of the elements.  All we have to do now
		 * is assemble them...
		 */

	if (fontfile == (char *) NULL || *fontfile == '\0') fontfile = f->name;
	PutFont (fontfile, f, elemarray, elemlist->ncomponents);
}

int elemcompar (e1p, e2p)
	register FontElement **e1p, **e2p;
{
	return ((*e1p)->num - (*e2p)->num);
}

decompile (fontfile, fontdirectory)
	char *fontfile;
	char *fontdirectory;
{
	FONT *f;
	char *fontname;
	register int i,j;
	register char *cp;
	int len, lo, hi;
	int removedir = 0;
	struct stat statbuf;

	f = GetFont (fontfile);
	if (!f) Error ("getting font in decompile", fontfile);

	assignBasename (fontname, fontfile);

	cp = rindex (fontname, '.');
	if (cp) *cp = '\0';

			/* can use fontname to make a directory */

	if (fontdirectory == NULL) fontdirectory = fontname;

	if (stat (fontdirectory, &statbuf) < 0) {
	    if (errno != ENOENT)  	/* then barf */
	        Error ("STATing fontdirectory", fontdirectory);
	    if (mkdir (fontdirectory, 0775) < 0)
		Error ("can't make the font directory", fontdirectory);
	    removedir++;
	    goto got_directory;
	}

		/* okay, the file exists; better be a directory... */

	if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {	/* not a directory */
	    Error ("can't decompile into a regular file", fontdirectory);
	}
			/* we have a directory ... */
    got_directory:
	if (chdir (fontdirectory) < 0) {
	    if (removedir) (void) rmdir (fontdirectory);
	    Error ("can't chdir to font directory", fontdirectory);
	}
		/* we are now in the fontdirectory */

	lo = max (low, f->first);
	hi = min (high, f->last);

	write_out_fontdir (f, lo, hi);

	FreeFont (f);

}

