/*
#
# $Source: /local/projects/X/fed/RCS/mapfile.c,v $
# $Header: mapfile.c,v 1.7 87/04/08 08:49:38 jim Exp $
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
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mapfile.h"

extern int errno;
char *malloc(), *calloc();

int mapfilein (filename, maplistp)
    char *filename;			/* name of file to read in */
    struct mapfile **maplistp;		/* where to set addr of bufs */
{
    struct stat statbuf;		/* buffer to receive stat info */
    int fd;				/* file descriptor of file */
    int filesize;			/* number of characters n file */
    char *readbuf;			/* buffer to receive contents */
    char *prevline;
    register char *cp;			/* work variable */
    register int i;			/* work variable */
    int nlines;				/* number of lines in file */
    int len;
    register struct mapfile *list;


					/* stat the file, abort on error */
    if (stat (filename, &statbuf) != 0) goto punt;

    filesize = (int) statbuf.st_size;
    if (filesize == 0) {		/* optomize empty file case */
	*maplistp = NULL;
	return (0);
    }
	
    errno = 0;
    fd = open (filename, O_RDONLY, 0777);
    if (errno != 0) goto punt;

    readbuf = (char *) malloc(filesize+2);	/* 1 for \n, 1 for for \0 */
    if (readbuf == (char *) NULL) {		/* if unable to get space */
	close (fd);
	goto punt;
    }

    i = read (fd, readbuf, filesize);	/* read file all at once */
    if (i != filesize) {		/* if we didn't get all of the file */
	free (readbuf);
	close (fd);
	goto punt;
    }
    close (fd);

    readbuf[filesize] = '\0';
    readbuf[filesize+1] = '\0';	/* 2nd null in case 1st becomes \n */

    cp = readbuf;
    nlines = 0;
    for (i = 0; i < filesize; cp++, i++) {    /* count \n */
	if (*cp == '\n')
	  nlines++;
    }
    if (*--cp != '\n') {		/* fake \n terminated */
	*++cp = '\n';
	i++;
    }

				/* allocate list + 1 extra for terminating 0 */

    list = (struct mapfile *) calloc ((unsigned) nlines,
				      (unsigned) sizeof (struct mapfile));
    if (!list) goto punt;		/* no more space */

    for (prevline = cp = readbuf, len = i = 0; i < nlines; cp++) {
	if (*cp != '\n') {
	    len++;
	    continue;
	}
	/* *cp = '\0'; */		/* replace \n with \0 */
	list[i].len = len + 1;		/* include \n in count */
	list[i].line = prevline;
	i++;				/* start on next line */
	len = 0;
	prevline = cp + 1;
    }

    *maplistp = list;
    return (nlines);

  punt:
    *maplistp = NULL;
    return (-1);
}


mapfilefree (list)
    struct mapfile *list;
{
    free (list[0].line);		/* free readbuf */
    free ((char *) list);		/* free the list itself */
}
