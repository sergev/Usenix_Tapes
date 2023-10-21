/*******************************************************
 *
 *	      Calendar Generation Program
 *	    Copyright 1986 David H. Brierley
 *
 * Permission is granted to anyone to use this software for any
 * purpose on any computer system, and to redistribute it freely,
 * subject to the following restrictions:
 * 1. The author is not responsible for the consequences of use of
 *	this software, no matter how awful, even if they arise
 *	from defects in it.
 * 2. The origin of this software must not be misrepresented, either
 *	by explicit claim or by omission.
 * 3. Altered versions must be plainly marked as such, and must not
 *	be misrepresented as being the original software.
 *
 * David H. Brierley
 * Portsmouth, RI
 * {allegra,ihnp4,linus}!rayssd!dhb
 *
 ********************************************************/

/*
 * pic_m2h - calndr program support module
 *
 * This module takes the internal machine format data file and
 * converts it to human readable format.  Carriage returns are
 * added to each line (actually \n) and the picture data is
 * optionally transformed into the compressed format defined
 * by the pic_h2m program.
 *
 */

#include <stdio.h>

main (argc, argv)
int     argc;
char   *argv[];
{
    char    line[134];
    char   *p1;
    char    moname[81];
    char    nums[6];
    int     points[13];
    int     picdata;
    int     compress;
    int     count;
    int     n;

    compress = 0;
    picdata = -1;
    for (n = 1; n < argc; n++) {
	if (strcmp (argv[n], "-c") == 0) {
	    compress = 1;
	    continue;
	}
	picdata = open (argv[n], 0);
	if (picdata == -1) {
	    perror ("calndr");
	    fprintf (stderr, "Unable to open file '%s'\n", argv[n]);
	    exit (1);
	}
    }
    if (picdata == -1) {
	fprintf (stderr, "No input file specified\n");
	exit (1);
    }

    for (n = 0; n < 12 * 7; n++) {
	if (read (picdata, moname, 81) != 81) {
	    fprintf (stderr, "I/O error on file\n");
	    exit (1);
	}
	printf ("%s\n", moname);
    }
    for (n = 0; n < 10 * 5; n++) {
	if (read (picdata, nums, 6) != 6) {
	    fprintf (stderr, "I/O error on file\n");
	    exit (1);
	}
	printf ("%s\n", nums);
    }
    if (read (picdata, (char *) points, sizeof points) != sizeof points) {
	fprintf (stderr, "I/O errron file\n");
	exit (1);
    }
    for (n = 0; ; n++) {
	if (read (picdata, line, 134) != 134) {
	    break;
	}
	for (p1 = line; *p1 != '\0'; p1++) {
	    printf ("%c", *p1);
	    if (compress == 1) {
		if (*(p1 + 1) != *p1) continue;
		for (count = 1; *(p1 + count) == *p1; count++) ;
		count--;
		p1 += count;
		if (count > 5) {
		    printf ("[%d]", count);
		}
		else {
		    while (count-- > 0) {
			printf ("%c", *p1);
		    }
		}
	    }
	}
	printf ("\n");
    }

    (void) close (picdata);

}
