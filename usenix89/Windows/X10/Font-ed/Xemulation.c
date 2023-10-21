/*
#
# $Source: /local/projects/X/fed/RCS/Xemulation.c,v $
# $Header: Xemulation.c,v 1.3 87/04/08 08:48:42 jim Exp $
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
 * Routines to emulate X utility procedures used in X internal routines.
 */

#include <stdio.h>
#include <errno.h>

extern int errno;

char *Xalloc (amount)
	register int amount;
{
	register char *ptr;

	if (amount < 0) goto bad;
	if (ptr = (char *) malloc ((unsigned) amount)) return (ptr);

    bad:
	errno = ENOMEM;
	fprintf (stderr, "Xalloc:  unable to allocate %d characters.\n",
		amount);
	exit (1);
}


DeviceError (msg)
	char *msg;
{
	fprintf (stderr, "DeviceError:  %s\n", msg);
	exit (1);
}
