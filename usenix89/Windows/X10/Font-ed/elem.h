/*
#
# $Source: /local/projects/X/fed/RCS/elem.h,v $
# $Header: elem.h,v 1.4 87/04/08 08:49:53 jim Exp $
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
 * Useful routines for manipulating lists - counts on calloc to zero out space.
 */

typedef struct _FontElement {		/* a font element */
	char *filename;		/* filename with this elements definition */
	int len;		/* length of this name */
	int num;		/* numeric value associated with this name */
	short *raster;		/* data for this font element */
	int size;		/* number of shorts in raster */
	int width;		/* width of this font element */
	int height;		/* height of this font element */
	struct _FontElement *prev, *next;	/* linked list */
} FontElement;

typedef struct _FontElementList {
	int ncomponents;
	struct _FontElement *head, *tail;
} FontElementList;

#define nullFontElement ((FontElement *) NULL)
#define nullFontElementList ((FontElementList *) NULL)

#define FontElementSize (sizeof (FontElement))
#define allocFontElement() ((FontElement *) calloc (1, FontElementSize))
#define allocFontElementList() \
		((FontElementList *) calloc (1, FontElementSize))

#define llAppend(_thing,_list) {                        \
        _list->ncomponents++;                                           \
        if (_list->tail) _list->tail->next = _thing;                    \
        _thing->prev = _list->tail;                                     \
        if (!_list->head) _list->head = _thing;                         \
        _list->tail = _thing;                                           \
        }

FontElementList *read_directory();
