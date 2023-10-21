#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "defs.h"
#include "newsrc.h"
#include "artfile.h"

char *savestr(), *ckmalloc();

/*
 * Set up the bit map, curng, ngsize, and groupdir for this newsgroup.
 * If no newsrc was read in then we don't bother with the bit map.
 */


char *bitmap;			/* which articles have been read */
ARTNO minartno;			/* first unread article */
ARTNO maxartno;			/* last article */


setupgrp(ngp, max)
	struct ngentry *ngp;
	ARTNO max;
	{
	int mapsize = (max >> 3) + 1;

	if (curng != NULL) {
		updaterc();
		free(bitmap);
	}
	curng = ngp;
	maxartno = max;
	bitmap = ckmalloc(mapsize);

/*
 * Set up the bit map.
 *
 * The key to understanding this piece of code is that a bit is set iff
 * that article has NOT been read.  Thus, we fill in the holes when
 * commas are found (e.g. 1-20,30-35 will result in filling in the 21-29
 * hold), and so we assume the newsrc file is properly ordered, the way
 * we write it out. 
 */
    {
	register char	*p, punct = ',';
	register ARTNO	cur = 0;
	ARTNO	next;

	bzero(bitmap, mapsize);

	/* check for first unread article */
	p = ngp->ng_bits;
	if (p == NULL)
		p = "";
	if (p[0] == '1' && p[1] == '-') {
		p += 2;
		cur = atoi(p);
		while (isdigit(*p))
			p++;
	}
	minartno = cur + 1;

	/* process rest of bit string */
	while (*p) {
		while (!isdigit(*p) && *p)
			p++;
		if (!*p)
			break;
		next = atoi(p);
		if (next > maxartno)		/* "Can't happen" */
			next = maxartno + 1;
		if (punct == ',') {
			while (++cur < next) {
				setunread(cur);
			}
		}
		cur = next;
		while (!ispunct(*p) && *p)
			p++;
		punct = *p;
	}
	while (++cur <= maxartno)
		setunread(cur);
	return 0;
    }
}



updaterc()
{
	register int	cur, next = 1;
	register char	*ptr;
	extern	int rcreadok;
	extern	char bfr[];

	if (!rcreadok || !curng)
		return;
	bfr[0] = bfr[1] = '\0';
	for (;;) {
		ptr = &bfr[strlen(bfr)];
		while (next <= maxartno && isunread(next))
			next++;
		if (next > maxartno)
			break;
		cur = next;
		while (next <= maxartno && ! isunread(next))
			next++;
		if (cur + 1 == next)
			sprintf(ptr, ",%d", cur);
		else
			sprintf(ptr, ",%d-%d", cur, next - 1);
	}
	if (curng->ng_bits != NULL)
		free(curng->ng_bits);
	curng->ng_bits = savestr(bfr + 1);	/* the +1 skips the leading comma */
}
