/*
#
# $Source: /local/projects/X/fed/RCS/ba.c,v $
# $Header: ba.c,v 1.7 87/04/08 08:48:47 jim Exp $
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
 * bitmaptoascii - convert a bitmap to an ascii (i.e. human readable) version
 * and back again.
 */

#include <stdio.h>
#include <strings.h>

char *ProgramName;

#define assignBasename(dstvar,srcstring) { \
	    (dstvar) = rindex (srcstring, "/"); \
	    (dstvar) = ((dstvar) ? (dstvar)++ : srcstring); }

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

Usage (message)
	char *message;
{
		fprintf (stderr, 
"%s:  error with '%s'.\nUsage:\n\t%s  [flags]  [inputfile]",
			 ProgramName, message, ProgramName);
		fprintf (stderr, 
"\nwhere [flags] are:\n");
		fprintf (stderr,
"    -a                Ascii to bitmap mode.\n");
		fprintf (stderr,
"    -o outputfile     Output to file instead of stdout.\n");
		fprintf (stderr,
"    -h                Help.  Print this message.\n");
		fprintf (stderr,
"\n");
	exit (1);
}

#define baBITMAP	1
#define baASCII		2

main (argc, argv)
	int argc;
	char *argv[];
{
	char *inputfile, *outputfile;
	int mode;

	setlinebuf (stderr);
	inputfile = outputfile = "";
	mode = baBITMAP;

	assignBasename (ProgramName, argv[0]);

	argc--; argv++;		/* ignore command (argv[0]) */
	for (; argc > 0; argc--, argv++) {	/* while there are args... */
	    if (**argv == '-') {		/* if -flag */
		register char *cp = *argv;
		for (cp++; *cp != '\0'; cp++) {		/* scan flags */
		    switch (*cp) {
			case 'a':
			    mode = baASCII;
			    break;
			case 'o':
			    get_string_arg ("-o outputfile", outputfile);
			    break;
			case 'h':
			    Usage ("having no manual to read");
			    break;
			default:
			    Usage ("unknown flag");
			    break;
		    } 						/*end switch*/
		} 						   /*end for*/
	    } else {	    /* got something beside a -flag on command line */
		if (*inputfile) Usage ("extra parameter");
		else inputfile = *argv;
	    } 							    /*end if*/
	} /*end for */

	if (*inputfile) 
	    if (!freopen (inputfile, "r", stdin))
		Error ("unable to open inputfile", inputfile);

	if (*outputfile) 
	    if (!freopen (outputfile, "w", stdout))
		Error ("unable to open outputfile", outputfile);

	switch (mode) {
	    case NULL:
	    case baBITMAP:
		bitmap2ascii (inputfile, outputfile);
		break;
	    case baASCII:
		ascii2bitmap (inputfile, outputfile);
		break;
	}
	exit (0);
}

/*
 * filter "bitmap" format files into ascii readable versions or other way
 * around.  input is from stdin and output to stdout; filenames are for
 * diagnostics only.
 */

short *ReadBitmapFile();
extern unsigned int extzv();

bitmap2ascii (inputfilename, outputfilename)
    char *inputfilename;		/* for error messages */
    char *outputfilename;		/* for error messages */
{
    short *data;
    int width, height, x_hot, y_hot, rastersize;
    register int scanline, bit, w;
    register short *base;
    
    data = ReadBitmapFile (stdin, &width, &height, &x_hot, &y_hot, 
			   &rastersize);
    
    if (!data) 
      Error ("unable to read in bitmap file", inputfilename);
    
    fprintf (stdout, "#define width %d\n", width);
    fprintf (stdout, "#define height %d\n", height);
    putc ('\n', stdout);
    w = width;
    base = data;
    for (scanline = 0; scanline < height; scanline++) {
	for (bit = 0; bit < w; bit++) {
	    if (extzv (base, bit, 1))
	      putc ('#', stdout);
	    else 
	      putc ('-', stdout);
	}
	base = &base[((width + 15) >> 4)];	/* round up words to skip */
	putc ('\n', stdout);
    }
}

ascii2bitmap (inputfilename, outputfilename)
    char *inputfilename, *outputfilename;
{
    unsigned short *data, *ReadAsciiBitmapFile();
    int width, height, dummyxhot, dummyyhot, rastersize;
    int n;

    data = ReadAsciiBitmapFile (stdin, &width, &height, &dummyxhot, &dummyyhot,
				&rastersize);
    if (!data) 
      Error ("unable to read in ascii bitmap file", inputfilename);

    fprintf (stdout, "#define width %d\n", width);
    fprintf (stdout, "#define height %d\n", height);
    fprintf (stdout, "\n");
    fprintf (stdout, "static short bits[] = {");

#define mod4(n) ((n) & 3)

    for (n = 0; n < rastersize; n++) {
	if (n != 0) 
	    fprintf (stdout, ", "); /* comma between */
	if (mod4 (n) == 0)
	    fprintf (stdout, "\n    ");
	fprintf (stdout, "0x%04x", (unsigned int) data [n]);
    }
    fprintf (stdout, "\n};");
}
