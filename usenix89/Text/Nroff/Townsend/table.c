/*
 *
 *     TABLE - A PROGRAM TO PREPARE NROFF DRIVER TABLES
 *     copyright (c) 1985 by Bruce Townsend and Bell-Northern Research.
 *     Permission hereby granted to use, distribute, modify, or copy
 *     except for profit, providing this disclaimer is included.
 *
 *     The contributions of Ian Darwin (the Makefile, cleaning up the
 *     code, etc) are gratefully acknowledged.
 *
 *     Version 1.2 -
 *        * The program has been substantially modified from version 1.1
 *          The major bug in version 1.1 was that zero-width characters
 *          were not handled properly. This has been fixed.
 *
 *     Please send bug reports to Bruce Townsend (utcs!bnr-vpa!bruce)
 *
 *     Usage:
 *           1) Build a file tabXXX.c, where XXX is some reasonable
 *              acronym for the terminal or printer you want to drive
 *              (peruse existing names to see what's reasonable!).
 *              Make this file by hacking up one of the existing
 *              table entries.
 *           2) If the amount of char data that the structure
 *              references is very large, you may want to redefine
 *              C_SIZE
 *           3) check to see whether <sgtty.h> or <termio.h> should
 *              be #included in the tabXXX.c file. One of these include
 *              files may be necessary to provide settings for t.bset,
 *              t.breset. This is instead of the previous tendency to code
 *              octal magic numbers in these fields.
 *           4) Enter your new table into the Makefile; just add the
 *              name "tabXXX" to the TABFILES definition.
 *           5) Type "make tabXXX" where tabXXX is the same name
 *              you chose before.
 *           6) Locate the tabfile in the proper place (on our system
 *              the directory is /usr/lib/term
 *
 *
 */

#define C_SIZE	10000	/* The maximum amount of character data allowed
			   in the initialized structure t - increase if
			   necessary */

#include <stdio.h>
#include "term.h"	/* This file contains the definition of the
			   the structures t and t_stor */

/************* DO NOT ALTER ANYTHING AFTER THIS POINT ***************/

extern struct t t;
struct t_stor t_stor;

char	c_data[C_SIZE];
char	*c_pointer[256];
int	c_length[256];
char	*c_end = c_data;
int	n_strings, c_size;

main (argc, argv)
int	argc;
char	*argv[];
{
	FILE	*table;
	int	i, j, i_len, j_len;
	char	*tail, *start, *char_pointer, *strcpy();

	if (argc != 2) {	/* Need a file name argument */
	    fprintf (stderr, "Usage: table \"file\"\n");
	    exit (1);
	    }

	/* Open the file */
	if ((table = fopen (argv[1], "w")) == NULL) {
	    fprintf (stderr, "File %s not opened for writing\n", argv[1]);
	    exit (1);
	    }

	/* Copy the integer values from the initialized structure
	   to the storage structure */
	t_stor.bset = t.bset;
	t_stor.breset = t.breset;
	t_stor.Hor = t.Hor;
	t_stor.Vert = t.Vert;
	t_stor.Newline = t.Newline;
	t_stor.Char = t.Char;
	t_stor.Em = t.Em;
	t_stor.Halfline = t.Halfline;
	t_stor.Adj = t.Adj;

	/* Add the character strings into a character array */
	addstring (t.twinit);
	addstring (t.twrest);
	addstring (t.twnl);
	addstring (t.hlr);
	addstring (t.hlf);
	addstring (t.flr);
	addstring (t.bdon);
	addstring (t.bdoff);
	addstring (t.iton);
	addstring (t.itoff);
	addstring (t.ploton);
	addstring (t.plotoff);
	addstring (t.up);
	addstring (t.down);
	addstring (t.right);
	addstring (t.left);
	for (i = 0; i < 256 - 32; i++) addchar (t.codetab[i]);

	/* eliminate strings which are tails of other strings */
	for (i = 0; i < n_strings; i++) {
	    if (! c_pointer[i]) continue;	/* String cleared out */
	    i_len = c_length[i];
	    for (j = 0; j < n_strings; j++) {
		if (i == j || ! c_pointer[j]) continue;
		j_len = c_length[j];
		if (i_len <= j_len) {	/* string i could be tail of string j */
		    tail = c_pointer[j] + j_len - i_len;
		    if (! char_comp (c_pointer[i], tail, i_len)) {
			c_pointer[i] = 0;
			break;
			}
		    }
		}
	    }

	/* Compress the c_data array */
	char_pointer = c_data;
	for (i = j = 0; i < n_strings; i++) {
	    if (! (start = c_pointer[i])) continue;
	    c_pointer[j] = char_pointer;
	    c_length[j++] = c_length[i];
	    for (i_len = c_length[i]; i_len--;) *char_pointer++ = *start++;
	    *char_pointer++ = 0;
	    }
	n_strings = j;
	c_size = char_pointer - c_data;

	/* Now find each string in this table and provide an index to it */
	t_stor.twinit = findstring (t.twinit);
	t_stor.twrest = findstring (t.twrest);
	t_stor.twnl = findstring (t.twnl);
	t_stor.hlr = findstring (t.hlr);
	t_stor.hlf = findstring (t.hlf);
	t_stor.flr = findstring (t.flr);
	t_stor.bdon = findstring (t.bdon);
	t_stor.bdoff = findstring (t.bdoff);
	t_stor.iton = findstring (t.iton);
	t_stor.itoff = findstring (t.itoff);
	t_stor.ploton = findstring (t.ploton);
	t_stor.plotoff = findstring (t.plotoff);
	t_stor.up = findstring (t.up);
	t_stor.down = findstring (t.down);
	t_stor.right = findstring (t.right);
	t_stor.left = findstring (t.left);
	for (i = 0; i < 256 - 32; i++) {
	    t_stor.codetab[i] = findchar (t.codetab[i]);
	    }
	t_stor.zzz = 0;

	/* Write the character storage block size */
	if (fwrite (&c_size, sizeof (c_size), 1, table) != 1)
	    write_err ();

	if (fwrite (&t_stor, sizeof (t_stor), 1, table) != 1)
	    write_err ();

	if (fwrite (c_data, sizeof (*c_data), c_size, table) != c_size)
	    write_err ();

	if (fclose (table)) {
	    fprintf (stderr, "File %s not closed properly\n", argv[1]);
	    exit (1);
	    }
	}

addstring (string)
char	*string;
{
	c_pointer[n_strings] = c_end;
	c_end += (c_length[n_strings] = strlen (string)) + 1;
	if (c_end >= c_data + C_SIZE) {
	    fprintf (stderr, "Table size too small, increase it!\n");
	    exit(1);
	    }
	strcpy (c_pointer[n_strings++], string);
}

addchar (string)
char	*string;
{
	c_pointer[n_strings] = c_end;
	c_end += (c_length[n_strings] = strlen (string + 2) + 2) + 1;
	if (c_end >= c_data + C_SIZE) {
	    fprintf (stderr, "Table size too small, increase it!\n");
	    exit(1);
	    }
	*c_pointer[n_strings] = *string++;	/* Copy in first two bytes */
	*(c_pointer[n_strings]+1) = *string++;
	strcpy (c_pointer[n_strings++] + 2, string); /* Copy the rest */
}

char_comp (str1, str2, len)
char	*str1, *str2;
int	len;
{
	while (len--) {
	    if (*str1++ != *str2++) return (1);
	    }
	return (0);
	}

findstring (string)
char	*string;
{
	int	c_len, s_len, i;

	for (i = 0; i < n_strings; i++) {
	    if ((c_len = c_length[i]) >= (s_len = strlen (string))) {
		if (!char_comp (string, c_pointer[i] + c_len - s_len, s_len))
		    return (c_pointer[i] + c_len - s_len - c_data);
		}
	    }
	fprintf (stderr, "Serious bug! string not found in table\n");
	exit(1);
	/* NOTREACHED */
	}

findchar (string)
char	*string;
{
	int	c_len, s_len, i;

	for (i = 0; i < n_strings; i++) {
	    if ((c_len = c_length[i]) >= (s_len = strlen (string+2) + 2)) {
		if (!char_comp (string, c_pointer[i] + c_len - s_len, s_len))
		    return (c_pointer[i] + c_len - s_len - c_data);
		}
	    }
	fprintf (stderr, "Serious bug! character not found in table\n");
	exit(1);
	/* NOTREACHED */
	}

write_err () {
	fprintf (stderr, "Write to file failed\n");
	exit (1);
	}
