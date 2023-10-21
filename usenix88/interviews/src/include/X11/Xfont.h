/*
 * Interface to X font/text operations.
 */

/* 
 * Copyright 1985, 1986, 1987 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 */


#ifndef Xfont_h
#define Xfont_h

#include <InterViews/Xdefs.h>

/* used in QueryFont -- draw direction */

#define FontLeftToRight		0
#define FontRightToLeft		1

#define FontChange		255

/*
 * per character font metric information.
 */
typedef struct {
    short	lbearing;	/* origin to left edge of raster */
    short	rbearing;	/* origin to right edge of raster */
    short	width;		/* advance to next char's origin */
    short	ascent;		/* baseline to top edge of raster */
    short	descent;	/* baseline to bottom edge of raster */
    unsigned short attributes;	/* per char flags (not predefined) */
} XCharStruct;

/*
 * To allow arbitrary information with fonts, there are additional properties
 * returned.
 */
typedef struct {
    XAtom name;
    unsigned long card32;
} XFontProp;

typedef struct {
    XExtData	*ext_data;	/* hook for extension to hang data */
    XFont        fid;            /* Font id for this font */
    unsigned	direction;	/* hint about direction the font is painted */
    unsigned	min_char_or_byte2;/* first character */
    unsigned	max_char_or_byte2;/* last character */
    unsigned	min_byte1;	/* first row that exists */
    unsigned	max_byte1;	/* last row that exists */
    XBool	all_chars_exist;/* flag if all characters have non-zero size*/
    unsigned	default_char;	/* char to print for undefined character */
    int         n_properties;   /* how many properties there are */
    XFontProp	*properties;	/* pointer to array of additional properties*/
    XCharStruct	min_bounds;	/* minimum bounds over all existing char*/
    XCharStruct	max_bounds;	/* minimum bounds over all existing char*/
    XCharStruct	*per_char;	/* first_char to last_char information */
    int		ascent;		/* log. extent above baseline for spacing */
    int		descent;	/* log. descent below baseline for spacing */
} XFontStruct;

/*
 * PolyText routines take these as arguments.
 */
typedef struct {
    char *chars;		/* pointer to string */
    int nchars;			/* number of characters */
    int delta;			/* delta between strings */
    XFont font;			/* font to print it in, None don't change */
} XTextItem;

typedef struct {		/* normal 16 bit characters are two bytes */
    unsigned char byte1;
    unsigned char byte2;
} XChar2b;

typedef struct {
    XChar2b *chars;		/* two byte characters */
    int nchars;			/* number of characters */
    int delta;			/* delta between strings */
    XFont font;			/* font to print it in, None don't change */
} XTextItem16;

XFont XLoadFont(XDisplay*, const char*);
XFontStruct* XQueryFont(XDisplay*, XFont);
char** XListFontsWithInfo(
    XDisplay*, char*, int maxnames, int count, XFontStruct**
);
XFontStruct* XLoadQueryFont(XDisplay*, const char*);
void XFreeFont(XDisplay*, XFontStruct*);
XBool XGetFontProperty(XFontStruct*, XAtom, unsigned long&);
char** XListFonts(XDisplay*, char*, int maxnames, int& count);
void XFreeFontNames(char**);

void XSetFontPath(XDisplay*, char**, int);
char** XGetFontPath(XDisplay*, int&);
void XFreeFontPath(char**);

int XTextWidth(XFontStruct*, const char*, int);
int XTextWidth16(XFontStruct*, const unsigned short*, int);
void XTextExtents(
    XFontStruct*, const char*, int,
    int& direction, int& ascent, int& descent, XCharStruct& overall
);
void XTextExtents16(
    XFontStruct*, const unsigned short*, int, int& direction,
    int& ascent, int& descent, XCharStruct& overall
);
void XQueryTextExtents(
    XDisplay*, XFont, const char*, int,
    int& direction, int& ascent, int& descent, XCharStruct& overall
);
void XQueryTextExtents16(
    XDisplay*, XFont, const XChar2b*, int,
    int& direction, int& ascent, int& descent, XCharStruct& overall
);

void XDrawText(XDisplay*, XDrawable, Xgc, int x, int y, XTextItem[], int);
void XDrawText16(XDisplay*, XDrawable, Xgc, int x, int y, XTextItem16[], int);
void XDrawString16(
    XDisplay*, XDrawable, Xgc, int x, int y, const XChar2b*, int
);
void XDrawImageString16(
    XDisplay*, XDrawable, Xgc, int x, int y, const XChar2b*, int
);

#endif
