/*
#
# $Source: /local/projects/X/fed/RCS/err.c,v $
# $Header: err.c,v 1.6 87/04/08 08:49:00 jim Exp $
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


#include <stdio.h>
#include <errno.h>

extern char *ProgramName;
extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

char *SysError ()
{
    return ((errno > 0 && errno < sys_nerr) ? 
	    sys_errlist [errno] : "unknown error code");
}

MessageHeader (t, m1, m2)
    char *t;				/* message type */
    char *m1, *m2;
{
    fprintf (stderr, "%s:  ", ProgramName);
    if (t) fprintf (stderr, "%s ", t);
    if (m1) fprintf (stderr, "%s ", m1);
    if (m2) fprintf (stderr, "%s ", m2);
}


Error (m1, m2)
    char *m1, *m2;
{
    MessageHeader ("error", m1, m2);
    fprintf (stderr, "(errno %d, %s).\n", errno, SysError());
    exit (1);
}

Warning (m1, m2)
	char *m1, *m2;
{
    MessageHeader ("warning", m1, m2);
    if (errno != 0) fprintf (stderr, "(errno %d, %s)", errno, SysError());
    fprintf (stderr, ".\n");
}

