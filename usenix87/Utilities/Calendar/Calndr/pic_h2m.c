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
 * pic_h2m - calndr program support module
 *
 * This program converts the picture data file from human readable
 * format to the internal machine format that the calndr program
 * is expecting to see.  The purpose for doing this is to speed up
 * execution of the calndr program.  If you are concerned about the
 * extra disk space required by the internal format of the data file
 * you may use the '-r' flag to the calndr program.
 */

/*
 * Format of human readable version of the data file.
 * 1. The first 84 lines of the file contain the names of the months.
 *    Each month name is 7 lines long and should be centered in
 *    eighty columns.  You should probably not touch this data.
 * 2. The next 50 lines contain the numbers from 0 to 9, five lines
 *    each, five characters wide.  Again, dont touch.
 * 3. The last section of the file contains the pictures.  This section
 *    may be modified to give different pictures.  Since the original
 *    program was in FORTRAN, the picture data uses fortran carriage
 *    control characters in the first column.  The beginning of each
 *    months picture is denoted by a top of form control ("1").  There
 *    should be exactly twelve pictures.  The first picture is always
 *    used for January, the second for February, etc.  As a general
 *    rule, the picture data is straight ascii text.  The one exception
 *    to this is that a character may be followed by a count enclosed
 *    within squares brackets.  The purpose of this is to allow the
 *    data file to be compressed but still be, for the most part, human
 *    readable.  One implication of this is that the data file cannot
 *    contain an open bracket followed by a string of digits followed
 *    by a close bracket.  The current data file does not even contain
 *    any open brackets, never mind all the rest of the crap.
 */

#include <stdio.h>
#include <ctype.h>

main (argc, argv)
int     argc;
char   *argv[];
{
    char    line[140];
    char    enil[140];
    char   *p1;
    char   *p2;
    char   *p3;
    char    moname[85];
    char    nums[10];
    int     points[13];
    int     psize;
    FILE   *picdata;
    int     newdata;
    int     n;
    int     len;
    int     locator;
    long    pos;
    long    lseek ();

    if (argc != 3) {
	fprintf (stderr, "Usage: %s infile outfile\n", argv[0]);
	exit (1);
    }
    picdata = fopen (argv[1], "r");
    if (picdata == NULL) {
	perror ("calndr");
	fprintf (stderr, "Unable to open file '%s'\n", argv[1]);
	exit (1);
    }
    newdata = creat (argv[2], 0666);
    if (newdata == -1) {
	perror ("calndr");
	fprintf (stderr, "Unable to open file '%s'\n", argv[2]);
    }

    for (n = 0; n < 12 * 7; n++) {
	if (fgets (moname, 85, picdata) == NULL) {
	    fprintf (stderr, "I/O error on file\n");
	    exit (1);
	}
	len = strlen (moname);
	for (--len; len < 81; len++) {
	    moname[len] = '\0';
	}
	if (write (newdata, moname, 81) != 81) {
	    perror ("calndr");
	    fprintf (stderr, "I/O error writing output file\n");
	    exit (1);
	}
    }
    for (n = 0; n < 10 * 5; n++) {
	if (fgets (nums, 10, picdata) == NULL) {
	    fprintf (stderr, "I/O error on file\n");
	    exit (1);
	}
	len = strlen (nums);
	for (--len; len < 6; len++) {
	    nums[len] = '\0';
	}
	if (write (newdata, nums, 6) != 6) {
	    perror ("calndr");
	    fprintf (stderr, "I/O error writing output file\n");
	    exit (1);
	}
    }
    pos = lseek (newdata, 0L, 1);
    if (pos == -1) {
	perror ("calndr");
	fprintf (stderr, "Unable to determine position in file\n");
	exit (1);
    }
    psize = 13 * sizeof (int);
    if (write (newdata, (char *) points, psize) != psize) {
	perror ("calndr");
	fprintf (stderr, "I/O error writing output file\n");
	exit (1);
    }
    locator = 0;
    for (n = 0;; n++) {
	if (fgets (enil, 140, picdata) == NULL) {
	    break;
	}
	if (enil[0] == '1') {
	    if (locator == 12) {
		break; /* already have 12 pictures */
	    }
	    points[locator] = n;
	    locator++;
	}
	p1 = enil;
	p2 = line;
	while (*p1 != '\n') {
	    if (*p1 != '[') {
		*p2++ = *p1++;
		continue;
	    }
            for (p3 = p1 + 1; isdigit (*p3); p3++) ; /* find close bracket */
	    if (*p3 == ']') {
		p3 = p1 - 1;
		len = atoi (++p1);
		while (len-- > 0) {
		    *p2++ = *p3;
		}
		while (*p1 != ']') p1++;
		p1++;
		continue;
	    }
	    *p2++ = *p1++;
	}
	for (; p2 < line + 134; p2++) {
	    *p2 = '\0';
	}
	if (write (newdata, line, 134) != 134) {
	    perror ("calndr");
	    fprintf (stderr, "I/O error writing output file\n");
	    exit (1);
	}
    }
    points[locator] = n;
    if (lseek (newdata, pos, 0) == -1) {
	perror ("calndr");
	fprintf (stderr, "Unable to seek backwards in file\n");
	exit (1);
    }
    if (write (newdata, (char *) points, psize) != psize) {
	perror ("calndr");
	fprintf (stderr, "I/O error writing output file\n");
	exit (1);
    }

    (void) fclose (picdata);
    (void) close (newdata);

}
