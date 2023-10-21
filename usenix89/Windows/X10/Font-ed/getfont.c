/*
#
# $Source: /local/projects/X/fed/RCS/getfont.c,v $
# $Header: getfont.c,v 1.9 87/04/08 08:49:29 jim Exp $
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


/* font.c	Reads a font from a file and stores it on the workstation
 *
 *	GetFont		Takes a font name and stores it
 *	FreeFont	Frees the storage taken by a font
 *
 * From the Cognition LEX90 implementation (grubbed form qvss and vs100).
 */

#include "font.h"
#include <ctype.h>

char *Xalloc(), *strcpy(), *strcat();
long lseek();

#define chars ((BitMap *) font.f_characters)

#ifdef COMMENT
/*
 * GetFont - reads in a DEC standard font file and creates an X FONT
 * structure.  Useful things that can be extracted from it are:
 *
 *	FONT *font;
 *	FontPriv *fpriv;
 *	BITMAP *strikebitmap;
 *	int c, charwidth, cind;
 *
 *	fpriv = (FontPriv *) font->data;
 *	strikebitmap = fpriv->strike;
 *
 *	c = (int) ascii_character ();
 *	if (c < font->first || c > font->last) punt();	/* bounds */
 *
 *	charwidth = fpriv->widths[c];	/* width of character */
 *	if (charwidth == 0 && c == font->space) /* then c is space */
 *	
 *	fpriv->leftarray 
 *	fpriv->fltable[0..height-1] = address of scan line in font bitmap
 *	cind = c - font->first;		/* index into char bitmaps */
 *		
 */
#endif COMMENT

FONT *GetFont (name)
	char *name;
{
	char fontname[256];
	int fontfile;
	FontData font;
	int fontsize, leftsize, width, i, j;
	char *fontarea;
	register short *leftarea, *leftarray;
	register FONT *fd;
	register FontPriv *fpriv;
	int tablesize = (CHARPERFONT+1) * sizeof(short);
	int dfdlen;

	dfdlen = strlen (DEFAULT_FONT_DIRECTORY);
	strcpy (fontname, DEFAULT_FONT_DIRECTORY);
	strcat (fontname, name);
	strcat (fontname, DEFAULT_FONT_SUFFIX);

	fontfile = open (&fontname[dfdlen], 0);		/* try name.suffix */

	if (fontfile < 0) fontfile = open (fontname, 0);     /* add prefix */
	else goto gotit;

	if (fontfile < 0) fontfile = open (name, 0);	      /* just name */
	else goto gotit;

	if (fontfile < 0) {
	    errno = EINVAL;
	    return (NULL);
	}
    gotit:
	if (read (fontfile, (caddr_t) &font, sizeof (FontData)) != sizeof (FontData)) {
	    close (fontfile);
	    errno = EINVAL;
	    return (NULL);
	}

	fontsize = BitmapSize(chars->bm_width, chars->bm_height);
	fontarea = (char *) Xalloc (fontsize);
	lseek (fontfile, (long) font.f_characters[0], 0);
	if (read (fontfile, fontarea, fontsize) != fontsize) {
	    close (fontfile);
	    free (fontarea);
	    errno = EINVAL;
	    return (NULL);
	}

	leftarea  = (short *) Xalloc (tablesize);
	bzero(leftarea, tablesize);
	leftarray = (short *) Xalloc (tablesize);
	if (font.f_fixedWidth == 0) {
	    leftsize = (font.f_lastChar - font.f_firstChar + 2) * sizeof (short);
	    lseek (fontfile, (long) font.f_leftArray[0], 0);
	    if (read (fontfile, & leftarea[font.f_firstChar], leftsize) 
	    		!= leftsize) {
		close (fontfile);
		free (fontarea);
		free ((caddr_t) leftarea);
		free ((caddr_t) leftarray);
		errno = EINVAL;
		return (NULL);
	    }
	} else { /* if fixed width font, generate leftarray for use later */
	    j = 0;
	    for (i = font.f_firstChar; i <= font.f_lastChar + 1; i++) {
		leftarea[i] = j;
		j += font.f_fixedWidth;
	    }
	}
	bcopy(leftarea, leftarray, tablesize);	/* copy for later hacking */

	close (fontfile);


	fd = (FONT *) Xalloc (sizeof (FONT));

	fd->height = chars->bm_height;		/* FONT height */
	fd->first = font.f_firstChar;		/* FONT first */
	fd->last = font.f_lastChar;		/* FONT last */

	fd->base = font.f_baseline;		/* FONT base */
	fd->space = font.f_spaceIndex;		/* FONT space */
	fd->space += fd->first;
	fpriv = (FontPriv *) Xalloc (sizeof (FontPriv));
	if (fd->avg_width = font.f_fixedWidth) {	/* FONT avg_width */
	    fd->fixed = 1;			/* FONT fixed */
	    fpriv->maxwidth = fd->avg_width;	/* FontPriv maxwidth */
	    }
	else
	    fd->fixed = 0;

	fd->refcnt = 1;				/* FONT refcnt */
	fd->data = (caddr_t) fpriv;		/* FONT fpriv */
	fpriv->widths = leftarea;		/* FontPriv widths */
	fpriv->leftarray = leftarray;		/* FontPriv leftarray */

	if ((fpriv->strike = (BITMAP *) Xalloc(sizeof(BITMAP))) == NULL) {
	    free (fontarea);
	    free ((caddr_t) leftarea);
	    free ((caddr_t) leftarray);
	    free ((caddr_t) fd);
	    free ((caddr_t) fpriv);
	    return (NULL);
	}
	fpriv->wpitch = (((chars->bm_width + 15) >> 3) & ~1); /* bytes/sl */
	fpriv->strike->width = chars->bm_width;	    /* width of font bitmap */
	fpriv->strike->height = chars->bm_height;    /* height of font */
	fpriv->strike->refcnt = 1;
	fpriv->strike->data = (caddr_t) fontarea;    /* bitmap for font */
	/*
	 * compute line table for font to eliminate multiply to find beginning
	 * of line.
	 */
	fpriv->fltable = (char **)Xalloc(chars->bm_height * sizeof(caddr_t));
	for (i = 0; i < chars->bm_height; i++) 
		fpriv->fltable[i] = ((caddr_t) fontarea) + i * fpriv->wpitch;

	fd->name = (char *) Xalloc (strlen (name) + 1);	/* FONT name */
	strcpy (fd->name, name);

	fpriv->maxwidth = 0;

				/* FontPriv widths, FontPriv maxwidth */
			/* convert the leftarray to the width table */
	for (i = fd->first; i <= fd->last; i++) {
	    width = fpriv->leftarray[i + 1] - fpriv->leftarray[i];
	    if (width > fpriv->maxwidth) {
		fpriv->maxwidth = width;
	    }
	    if (width < 0) {
		fprintf (stdout, 
		    "GetFont char %d (bounds [%d,%d]) width = %d\n",
			i, fd->first, fd->last, width);
		width = 0;	/* font sanity check */
/*		DeviceError ("Bad font leftarray!\n");		*/
		}
	    fpriv->widths[i] = width;
	}
		
				/* FONT avg_width */
        fd->avg_width = ((fpriv->leftarray[fd->last+1]  - 
		fpriv->leftarray[fd->first]) / (fd->last - fd->first + 1));

	return (fd);
#undef chars
}

FreeFont (font)
	register FONT *font;
{
	register FontPriv *data;

	data = FDATA(font);
	if (data->widths) free ((caddr_t) data->widths);
	free (data->fltable);
	FreeBitmap(data->strike);
	free ((caddr_t) data);
	free (font->name);
	free ((caddr_t) font);
}

#ifndef XSERVER
FreeBitmap (bm)
	register BITMAP *bm;
{
	free ((caddr_t) bm->data);
	free ((caddr_t) bm);
	return;
}
#endif
