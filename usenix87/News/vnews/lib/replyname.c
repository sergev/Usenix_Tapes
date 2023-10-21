/*
 * Generate an address for replying to this article
 */

#include <stdio.h>
#include "config.h"
#include "defs.h"
#include <sys/types.h>
#include "arthead.h"

char *
replyname(hptr, tbuf)
struct arthead *hptr;
char tbuf[PATHLEN];
{
	register char *ptr;
	extern char FULLSYSNAME[];
	char *getaddr();

#ifndef INTERNET
	strcpy(tbuf, hptr->h_path);
	/*
	 * Play games stripping off multiple berknet
	 * addresses (a!b!c:d:e => a!b!d:e) here.
	 */
	for (ptr=tbuf; *ptr; ptr++)
		if (index(NETCHRS, *ptr) && *ptr == ':' && index(ptr+1, ':'))
			strcpy(ptr, index(ptr+1, ':'));
#else
	ptr = hptr->h_from;
	if (hset(hptr->h_replyto))
		ptr = hptr->h_replyto;
	getaddr(ptr, tbuf);
#endif
	return tbuf;
}
