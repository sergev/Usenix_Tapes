/*
#
# $Source: /local/projects/X/fed/RCS/putfont.c,v $
# $Header: putfont.c,v 1.7 87/04/08 08:49:42 jim Exp $
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

extern unsigned int extzv();

#define WordsPerScanLine(w) (((w) + 15) >> 4)
#define BitmapSizeInWords(w,h) (WordsPerScanLine (w) * (h))

int PutFont (fontfilename, f, elemarray, nelems)
	char *fontfilename;		/* name of the file to write out */
	FONT *f;		/* the font structure to use */
	register FontElement *elemarray[];
{
	int i;
	int width, height, size, bad, leftsize, curpos, curelemnum;
	int rastersize, tablesize, wordsperscanline, wpsl;
	short *fontraster;
	short *leftedgearray;
        char **fltable;		/* font line table[i] is start of scanline i */
	FontElement *elemp;
	FontPriv _fp;
	FontPriv *fp = &_fp;
	short *datap;
	FILE *fontfile;
	int dfslen, len;
	char ffname[256];
	long bitmapoffset, leftarrayoffset;
	FontData font;
	
#define charbitmap (* (BitMap *) font.f_characters)

	if (nelems <= 0) {
	    fprintf (stderr, "PutFont: fonts must have at least 1 element.\n");
	    errno = ENOENT;
	    return (-1);
	}

	if (f->fixed < 0) {
	    fprintf (stderr, "PutFont: negative (%d) fixed width font.\n",
		f->fixed);
	    errno = EINVAL;
	    return (-1);
	}

	f->first = elemarray[0]->num;
	f->last = elemarray[nelems-1]->num;

	bad = 0;		/* set if error encountered */
	width = 0;
	height = f->height;
	for (i = 0; i < nelems; i++) {
	    width += elemarray[i]->width;
	    if (f->fixed && elemarray[i]->width != f->fixed) {
		fprintf (stderr,
		    "PutFont: font element %s has width %d instead of %d.\n",
		    elemarray[i]->filename, elemarray[i]->width, f->fixed);
		bad++;
	    }
	    if (elemarray[i]->height != height) {
		fprintf (stderr, 
		    "PutFont: font element %s has height %d instead of %d.\n",
		    elemarray[i]->filename, elemarray[i]->height, height);
		bad++;
	    }
	}
	if (bad) {
	    errno = EMSGSIZE;
	    return (-1);
	}

		/*
		 * if fixed width then do all in between chars
		 */

	leftsize = f->last - f->first + 2;
	if (f->fixed) {
	    width = (f->last - f->first + 1) * f->fixed;
	}

		/* 
		 * allocate space for raster and for left array
		 */

	fontraster = leftedgearray = (short *) NULL;
	fltable = (char **) NULL;

	wordsperscanline = WordsPerScanLine (width);
	rastersize = BitmapSizeInWords (width, height);
	fontraster = (short *) calloc (rastersize, sizeof (short));
	if (!fontraster) {
	    fprintf (stderr,
		"PutFont: unable to allocate %d words for font raster.\n", 
		rastersize);
	    errno = ENOMEM;
	    goto cleanup;
	}
	
	fltable = (char **) calloc (height, sizeof (char *));
	if (!fltable) {
	    fprintf (stderr,
		"PutFont: unable to allocate %d pointers for line table.\n",
		height);
	    errno = ENOMEM;
	    goto cleanup;
	}
	for (i = 0; i < height; i++)
	    fltable[i] = (char *) (fontraster + i * wordsperscanline);

	tablesize = CHARPERFONT + 1;
	leftedgearray = (short *) calloc (tablesize, sizeof (short));
	if (!leftedgearray) {
	    fprintf (stderr, 
		"PutFont: unable to allocate %d words for left edge array.\n",
		tablesize);
	    errno = ENOMEM;
	    goto cleanup;
	}

		/* 
		 * okay, now iterate over elements and fill in data 
		 * 
 		 * The font file the first element of the left edge array is
		 * the start of f->first and the last is the right size of 
		 * f->last.  GetFont reads the data into &leftarea[f->first]
		 * so that the width table can be generated easily.
		 */
	
	curpos = 0;		/* current left edge */
	curelemnum = 0;		/* current element to look at */
	for (i = 0; i <= f->last ; i++) {
	    leftedgearray[i] = curpos;
	    elemp = elemarray[curelemnum];

	    if (i == elemp->num) {	/* if char supplied */
		int scanline, wordw;
			/*
			 * insert the raster for the element at curpos
			 */
		wordw = elemp->width &~ 0xf;	/* round it */
	        wpsl = WordsPerScanLine (elemp->width);
		datap = elemp->raster;

		for (scanline = 0; scanline < height; scanline++) {
		    register char *base = fltable[scanline];
		    register bit;

		    for (bit = 0; bit < wordw; bit += 16) {
			insv (base, curpos + bit, 16, 
			    extzv (datap, bit, 16));
				
		    }
		    if (elemp->width != wordw) {
			insv (base, curpos + wordw, elemp->width - wordw,
				extzv (datap, wordw, elemp->width - wordw));
		    }
		    datap += wpsl;	/* save many multiplies */
		}

		curpos += elemarray[curelemnum]->width;
		curelemnum++;
	    } else if (f->fixed && i > f->first && i < f->last) {
		curpos += f->fixed;
	    }
	}
	leftedgearray[f->last+1] = curpos;    /* fence post */

#ifdef VERBOSE
	printf ("leftedgearray:\n");
	for (i = 0; i < tablesize; i++) {
	    printf ("%5d  ", leftedgearray[i]);
	    if ((i % 10) == 9) putchar ('\n');
	} 
	putchar ('\n');
#endif

	dfslen = strlen (DEFAULT_FONT_SUFFIX);
	len = strlen (fontfilename);
	strcpy (ffname, fontfilename);
	if (len <= dfslen || strcmp (&fontfilename[len-dfslen], 
				     DEFAULT_FONT_SUFFIX) != 0) {
	    strcpy (&ffname[len], DEFAULT_FONT_SUFFIX);
	}
	fontfile = fopen (ffname, "w");
	if (!fontfile) {
	    fprintf (stderr, 
		"PutFont: unable to open font file %s for output.\n",
		ffname);
	    bad = 1;
	    goto cleanup;
	}

	font.f_firstChar = f->first;
	font.f_lastChar = f->last;
	font.f_baseline = f->base;
	font.f_spaceIndex = f->space;
	font.f_fixedWidth = f->fixed;
	charbitmap.bm_width = width;
	charbitmap.bm_height = height;
	charbitmap.bm_bitsPerPixel = 1;

	bitmapoffset = sizeof (a_FontData);
	leftarrayoffset = bitmapoffset + rastersize * sizeof (short);

	bcopy ((char *)&bitmapoffset, (char *) charbitmap.bm_address, 
		sizeof (long));
	bcopy ((char *)&leftarrayoffset, (char *) font.f_leftArray,
		sizeof (long));

	fwrite ((char *) &font, sizeof (FontData), 1, fontfile);
	fwrite ((char *) fontraster, sizeof (short), rastersize, fontfile);
	fwrite ((char *) &leftedgearray[f->first], sizeof (short), 
		leftsize, fontfile);
	fclose (fontfile);

    cleanup:
	i = errno;
	if (leftedgearray) free (leftedgearray);
	if (fontraster) free (fontraster);
	if (fltable) free (fltable);
	errno = i;
	return (bad ? -1 : 0);
}
