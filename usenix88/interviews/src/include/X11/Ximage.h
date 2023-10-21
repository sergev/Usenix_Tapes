/*
 * Interface to X image operations.
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


#ifndef Ximage_h
#define Ximage_h

#include <InterViews/Xdefs.h>

/* ImageFormat -- PutImage, GetImage */

#define XYBitmap		0	/* depth 1, XYFormat */
#define XYPixmap		1	/* depth == drawable depth */
#define ZPixmap			2	/* depth == drawable depth */

/*
 * Data structure for "image" data, used by image manipulation routines.
 */
struct XImage {
    int width, height;		/* size of image */
    int xoffset;		/* number of pixels offset in X direction */
    int format;			/* XYBitmap, XYPixmap, ZPixmap */
    char *data;			/* pointer to image data */
    int byte_order;		/* data byte order, LSBFirst, MSBFirst */
    int bitmap_unit;		/* quant. of scanline 8, 16, 32 */
    int bitmap_bit_order;	/* LSBFirst, MSBFirst */
    int bitmap_pad;		/* 8, 16, 32 either XY or ZPixmap */
    int depth;			/* depth of image */
    int bytes_per_line;		/* accelarator to next line */
    int bits_per_pixel;		/* bits per pixel (ZPixmap) */
    unsigned long red_mask;	/* bits in z arrangment */
    unsigned long green_mask;
    unsigned long blue_mask;
    char *obdata;		/* hook for the object routines to hang on */
    struct funcs {		/* image manipulation routines */
	XImage *(*create_image)();
	int (*destroy_image)();
	unsigned long (*get_pixel)();
	int (*put_pixel)();
	struct _XImage *(*sub_image)();
	int (*add_pixel)();
	} f;
};

XImage* XCreateImage(
    XDisplay*, XVisual*, int depth, int format, int offset,
    void* data, int width, int height, int bitpad, int bytes_per_line
);
void XPutImage(
    XDisplay*, XDrawable, Xgc, XImage*, int srcx, int srcy,
    int dstx, int dsty, XCardinal width, XCardinal height
);
void XGetImage(
    XDisplay*, XDrawable, Xgc, XImage*, int srcx, int srcy,
    int dstx, int dsty, XCardinal width, XCardinal height,
    XPlaneMask, int format
);

#endif
