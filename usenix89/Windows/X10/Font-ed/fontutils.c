/*
#
# $Source: /local/projects/X/fed/RCS/fontutils.c,v $
# $Header: fontutils.c,v 1.14 87/04/08 08:49:22 jim Exp $
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


#include "font.h"
#include "elem.h"
#include <ctype.h>
#include <sys/dir.h>

extern unsigned int extzv();

fontutil_printbitmap (f, outstream, printboundaries)
    FONT *f;				/* font to write */
    FILE *outstream;			/* where to write */
    int printboundaries;		/* bool */
{
    FontPriv *fp;
    int scanline, column;
    BITMAP *strikebm;
    char *procname = "printbitmap";
    char *fontname;
    int leftedgeindex, leftedge;
    short *leftarray;

    if (!f) {
	fprintf (stderr, "%s:  null FONT\n", procname);
	return;
    }

    if (!(fontname = f->name)) fontname = "?unknown?";

    fp = FDATA (f);
    if (!fp) {
	fprintf (stderr, "%s:  null FontPriv for font %s.\n",
		 procname, fontname);
	return;
    }

    leftarray = fp->leftarray;
    if (!leftarray) {
	fprintf (stderr, "%s:  null leftarray for font %s.\n",
		 procname, fontname);
	return;
    }

    strikebm = fp->strike;
    if (!strikebm) {
	fprintf (stderr, "%s:  null strike bitmap for font %s.\n",
		 procname, fontname);
	return;
    }

    leftedgeindex = 0;
    for (column = 0; column < strikebm->width; column++) {
	for (; (leftedge = leftarray[leftedgeindex]) <= column;
	     leftedgeindex++) {
	    if (printboundaries) {
		printf ("  Font %s, char %d ('%c'), starts at %d, width %d:\n",
			fontname, leftedgeindex,
			isprint (leftedgeindex) ? leftedgeindex : '?',
			leftedge, leftarray[leftedgeindex+1] - leftedge);
	    }
	}
	for (scanline = strikebm->height - 1; scanline >= 0; scanline--) {
	    char *base = fp->fltable[scanline];
	    int byteno = column >> 3;
	    int bitno = column & 7;

	    putc (((base[byteno] & (1 << bitno)) ? '#' : '-'), outstream);
	}
	putc ('\n', outstream);
    }

    return;
}


fontutil_dumpheader (f)
	FONT *f;
{
	FontPriv *fp;
	int leftsize;
	register int j;

	if (!f) {
	    fprintf (stderr, "dumpheader:  null FONT\n");
	    return;
	}

	fp = FDATA (f);

	printf ("FONT:\n");
	printf ("name = %s, [first,last] = [%d,%d], space = %d,\n",
		f->name, f->first, f->last, f->space);
	printf ("avg_width = %d, height = %d, fixed = %d, base = %d\n",
		f->avg_width, f->height, f->fixed, f->base);

	printf ("FontPriv:\n");
	printf ("    maxwidth = %d, wpitch = %d\n", fp->maxwidth, fp->wpitch);

	leftsize = f->last - f->first + 2;
	printf ("    leftedge array size [%d,%d] = %d, leftarray table:\n", 
			fp->leftarray[f->first],fp->leftarray[f->last+1], 
			leftsize);
	for (j = 0; j <= f->last + 1; j++) {
	    if (j) {
		if ((j % 10) == 0) printf ("\n");
		else printf ("  ");
	    } 
	    printf ("%4d", (j < f->first) ? 0 : fp->leftarray[j]);
	}

	printf ("\n\n    width table size = %d, width table:\n", 
			f->last - f->first + 1);
	for (j = 0; j <= f->last; j++) {
	    if (j) {
		if ((j % 10) == 0) printf ("\n");
		else printf ("  ");
	    }
	    printf ("%4d", (j < f->first) ? 0 : fp->widths[j]);
	}
	printf ("\n\n");
	return;
}


fontutil_print_char_bitmapfile (f, i, outstream)
	FONT *f;
	int i;
	FILE *outstream;
{
	FontPriv *fp;
	int scanline;
	register bit;
	register w, wordw;
	register offset;

	fp = FDATA (f);

	w = fp->widths[i];
	offset = fp->leftarray[i];

 	fprintf (outstream, "#define char_width %d\n", w);
	fprintf (outstream, "#define char_height %d\n", f->height);
	fprintf (outstream, "static short char_bits[] = {\n");

	wordw = w & ~0xf;		/* get rid of low 4 bits */
	for (scanline = 0; scanline < f->height; scanline++) {
	    register char *base = fp->fltable[scanline];
	    for (bit = 0; bit < wordw; bit += 16) {
		fprintf (outstream, "0x%04x, ", extzv (base, offset + bit, 16));
	    }
	    if (w != wordw) 
		fprintf (outstream, "0x%04x, \n", 
			extzv (base, offset + wordw, w - wordw));
	}
	fprintf (outstream, "};\n");
}

write_profile (f)
	register FONT *f;
{
	FILE *profile;

	profile = fopen (profilename, "w");
	if (!profile) Error ("unable to write profile ", profilename);

	fprintf (profile, "space: 0%o\n", (int) f->space);
	fprintf (profile, "height: %d\n", (int) f->height);
	fprintf (profile, "fixed: %d\n", (int) f->fixed ? f->avg_width : 0);
	fprintf (profile, "base: %d\n", (int) f->base);
	fclose (profile);
}

#define isblank(c) ((c) == ' ' || (c) == '\t' || (c) == ':')
#define skipspace(cp) for (; isblank(*cp); cp++)
#define skipword(cp) for (; isalnum(*cp); cp++)

extern char *profilename;

read_profile (f)
	register FONT *f;
{
	FILE *profile;
	int value;
	char *variable, line[81], *cp, *input;

	profile = fopen (profilename, "r");
	if (!profile) Error ("unable to read profile ", profilename);

	while (fgets (line, sizeof (line), profile)) { 
	    input = line;
	    skipspace (input);
	    if (*input == '#' || !*input) continue; /* skip comments */

	    variable = input;
	    skipword (input);
	    *input++ = '\0';	/* variable is now valid */

	    skipspace (input);
	    if (!*input) {
		Warning ("no value in profile field: ", line);
		continue;
	    }

	    if (*input == '0') value = aotoi (input);	/* get octal num */
	    else value = atoi (input);			/* get decimal num */

	    if (streq (variable, "space")) f->space = value; else
	    if (streq (variable, "height")) f->height = value; else
	    if (streq (variable, "fixed")) f->fixed = value; else
	    if (streq (variable, "width")) f->fixed = value; else
	    if (streq (variable, "base")) f->base = value; else
	    Error ("unknown field in profile", variable);
	}
	fclose (profile);
}

FontElementList *read_directory (directoryname)
	char *directoryname;
{
	DIR *dirp;
	struct direct *dp;
	FontElementList *felist;
	FontElement *fe;

	if (!directoryname) {
	    Warning ("read_directory called with a null dirname", NULL);
	    directoryname = ".";
	}

	dirp = opendir (directoryname);
	if (!dirp) 
	    Error ("unable to open font directory", directoryname);

	felist = allocFontElementList ();
	if (!felist)
	    Error ("unable to alloc the Font Element list");

	for (dp = readdir (dirp); dp; dp = readdir (dirp)) {
	    if (!allodigits (dp->d_name)) {
		if (alldigits (dp->d_name)) {
		    errno = 0;
		    MessageHeader (NULL, NULL, NULL);
		    fprintf (stderr, "non-octal file %s in font dir %s/.\n",
			     dp->d_name, directoryname);
		}
		continue;	/* is not font char */
	    }
	    fe = allocFontElement ();
	    fe->filename = copystr (dp->d_name);
	    fe->len = dp->d_namlen;
	    fe->num = aotoi (fe->filename);
	    llAppend (fe, felist);
	}

	closedir (dirp);
	return (felist);
}	    

int allodigits (name)
	register char *name;
{
	while (*name) {
	    if (*name < '0' || *name > '7') return (0);
	    name++;
	}
	return (1);
}

int alldigits (name)
	register char *name;
{
	while (*name) {
	    if (*name < '0' || *name > '7') return (0);
	    name++;
	}
	return (1);
}

int aotoi (s)
	register char *s;
{
	register char *cp;
	register int accumulator, base;

	if (s == (char *) NULL || *s == '\0') return (0);	/* bad? */

	for (cp = s; *cp >= '0' && *cp <= '7' ; cp++) ;	/* find end */

	base = 1;
	accumulator = 0;
	for (cp--; cp >= s; cp--) {	/* back off illegal char */
	    accumulator += (*cp - '0') * base;
	    base *= 8;
	}
	return (accumulator);
}

char *copystr (s)
	register char *s;
{
	register char *cp;
	register int len;

		/* get length of string */
	for (len = 0, cp = s; *cp; len++, cp++) ;
		/* space for string */
	cp = (char *) malloc (len + 1);

	strcpy (cp, s);		/* this should be one instruction... */

	return (cp);
}


/*
 * write out the font into the current directory
 */

write_out_fontdir (f, lo, hi)
    FONT *f;
    int lo;
    int hi;
{
    int i;
    FILE *charbitmapfile;
    char charbitmapfilename[4];		/* xxx\0 */

    write_profile (f);

    for (i = lo; i <= hi; i++) {
	charbitmapfilename[0] = charbitmapfilename[1] = 
	  charbitmapfilename[2] = charbitmapfilename[3] = '\0';

	sprintf (charbitmapfilename, "%03o", i);

	charbitmapfile = fopen (charbitmapfilename, "w");
	if (!charbitmapfile)
	  Error ("unable to create char bitmap file",charbitmapfilename);
	else 
	  fontutil_print_char_bitmapfile (f, i, charbitmapfile);
	(void) fclose (charbitmapfile);
    }
}



#ifndef INLINE
/*
 * The following emulates the "routines" that are replaced by inline.
 */

static unsigned int keeptable[9] = {0x00,0x01,0x03,0x07,
				    0x0f,0x1f,0x3f,0x7f, 0xff};


unsigned int extzv (base, startbit, nbits)
    unsigned char *base;		/* base address */
    int startbit;		/* where to start, 0 is first bit */
    int nbits;			/* number of bits to handle <= 32 */
{
    unsigned int retval = 0;
    int byteoff = startbit >> 3;
    int bitoff = startbit & 7;
    int havebits = 0;

    base += byteoff;
    if (nbits > 32) nbits = 32;

    while (nbits > 0) {
	unsigned int val, maxbits, getbits;

	maxbits = (8 - bitoff);		/* how many bits left in this byte */
	getbits = nbits;
	if (getbits > maxbits) getbits = maxbits;    /* how many to get */

	val = ((unsigned int) *base);
	val >>= bitoff;			/* put it to the start of the byte */
	val &= keeptable [getbits];	/* keep the right number of bits */
	retval |= (val << havebits);	/* and put them into the result */
	havebits += getbits;		/* record how many we have for next */
	nbits -= getbits;		/* and debit */
	bitoff = 0;			/* later iterations start on bytes */
	base++;				/* go to the next byte */
    }

    return (retval);
}

unsigned int insv (base, startbit, nbits, src)
    unsigned char *base;		/* base address */
    int startbit;			/* where to start, 0 is first bit */
    int nbits;				/* number of bits to handle <= 32 */
    unsigned int src;			/* where to get bits from */
{
    unsigned int origsrc = src;
    int byteoff = startbit >> 3;
    int bitoff = startbit & 7;
    int havebits = 0;

    base += byteoff;
    if (nbits > 32) nbits = 32;

    while (nbits > 0) {
	unsigned int val, mask, putbits, retval;
	int maxbits;

	maxbits = (8 - bitoff);		/* how many can we do this time? */
	putbits = nbits;
	if (putbits > maxbits) putbits = maxbits;

	/* going to stuff putbits worth starting at bit offset bitoff */
	/* source is from bit 0 of src since we shift it each time */

	retval = (unsigned int) *base;

	/* first, mask out the destination bits */
	mask = keeptable [putbits];	/* for getting only the correct bits */
	retval &= ~(mask << bitoff);	/* clear them */

	/* next, get the src bits, no shifting since src is shifted below */
	val = (src & mask);		/* get from bottom of src */
	val <<= bitoff;			/* and move to destination place */

	/* and stuff them in */
	retval |= val;
	*base = (unsigned char) retval;

	/* finally we need to do bookkeeping */
	nbits -= putbits;
	src >>= putbits;		/* src better be declared unsigned */
	bitoff = 0;			/* starting on byte next time */
	base++;
    }
    return (origsrc);
}
#endif !INLINE
