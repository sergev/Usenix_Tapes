/*
 * This software is Copyright (c) 1986 by Rick Adams.
 *
 * Permission is hereby granted to copy, reproduce, redistribute or
 * otherwise use this software as long as: there is no monetary
 * profit gained specifically from the use or reproduction or this
 * software, it is not sold, rented, traded or otherwise marketed, and
 * this copyright notice is included prominently in any copy
 * made.
 *
 * The author make no claims as to the fitness or correctness of
 * this software for any use whatsoever, and it is provided as is. 
 * Any use of this software is at the user's own risk.
 *
 * funcs - functions used by many programs
 */

#ifdef SCCSID
static char	*SccsId = "@(#)funcs.c	2.31	1/17/86";
#endif /* SCCSID */

/*LINTLIBRARY*/

#include <stdio.h>
#include <errno.h>
#if defined(USG) || defined(BSD4_2) || defined(BSD4_1C)
#include <fcntl.h>
#endif /* !v7 */

#if !defined(BSD4_2) && !defined(BSD4_1C)
/*
 * make a directory. Also make sure that the directory is owned
 * by the right userid
 */
mkdir(path, perm)
char *path;
int perm;
{
	int pid, status;

	if (pid=fork()) {
		while (wait(&status) != pid);
	} else {
		(void) execlp("mkdir", "mkdir", path, (char *)NULL);
		perror(path);
		_exit(1);
	}
	return status;
}
#endif /* !BSD4_2 && ! BSD4_1C */
