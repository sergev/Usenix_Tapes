/*
#
# $Source: /local/projects/X/fed/RCS/font.h,v $
# $Header: font.h,v 1.7 87/04/08 08:49:56 jim Exp $
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

#ifndef _TYPES_
#include <sys/types.h>
#endif _TYPES_

#ifndef FILE
#include <stdio.h>
#endif FILE

#include <X/vsinput.h>

#ifdef _DONT_INCLUDE_X_
#undef CURSOR
#endif _DONT_INCLUDE_X_

#include <X/Xdev.h>

#ifndef _DONT_INCLUDE_X_
#include <X/X.h>
#endif _DONT_INCLUDE_X_

#include <X/vssite.h>
#include <errno.h>
#include <strings.h>
#include "vs100.h"

#define CHARSPERFONT 256

#define streq(a,b) (strcmp ((a), (b)) == 0)

extern char *ProgramName;
extern char *profilename;

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];
char *SysError();

char *malloc(), *realloc(), *calloc();
FONT *GetFont();
char *copystr();
int elemcompar();	/* used by qsort */
short *ReadBitmapFile(), *ReadAsciiBitmapFile();

/*
 * From the Cognition LEX90 implementation of X
 */

#define CHARPERFONT 256

typedef struct _fontpriv {
	short *widths;		/* width table (in pixels)	   */
	int maxwidth;		/* maximum width found in the font */
	int wpitch;		/* number of bytes/line in strike  */
	short *leftarray;	/* leftarray			   */
	BITMAP *strike;		/* the font stike bitmap itself	   */
	short *chrs;		/* chars in independent bitmaps	   */
	char **fltable;		/* beginning of each strike line   */
} FontPriv;

#define FDATA(x) ((FontPriv *) ((x)->data))



