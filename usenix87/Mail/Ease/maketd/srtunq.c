/* Sorting function that inserts strings one at a time into memory.
 * Strings are null terminated.
 * Only uniq strings are stored (no count is kept of how many)
 * Any memory used is freed on init (or re-init).
 *
 * Author: Stephen Uitti, PUCC, 1985
 * libsrtunq is copyright (C) Purdue University, 1985
 *
 * Permission is hereby given for its free reproduction and
 * modification for non-commercial purposes, provided that this
 * notice and all embedded copyright notices be retained.
 * Commercial organisations may give away copies as part of their
 * systems provided that they do so without charge, and that they
 * acknowledge the source of the software.
 */

#ifndef lint
static char *rcsid =
    "$Id: srtunq.c,v 1.4 86/08/30 21:35:36 ksb Exp Locker: ksb $";
#endif

#ifdef BSD2_9
#include <sys/types.h>
#endif
#ifdef notdef			/* a comment that allows comments. */
 * use:
 * #include <local/srtunq.h>			/* defines, etc */
 * struct srtent d;				/* data structure foothold */
 * char *p;					/* temp */
 * srtinit(&d);					/* init the structure */
 * if ((p = srtin(&d, "foo", NULL)) != NULL) err(p);	/* add the data */
 *	...
 * srtgti(&d);					/* init for get */
 * p = srtgets(&d);				/* get next entry */
 *	...
 * srtfree(&d);					/* delete the structure */
#endif

#include <stdio.h>			/* for null */
#include <sys/types.h>			/* for void, in V7 */
#include "srtunq.h"			/* for srtunq structs & functions */
/* #include <local/srtunq.h>		/* for srtunq structs & functions */

/* libc functions */
char   *malloc();			/* need more memory */
char   *strcpy();			/* copy string */

/* srtgti - init get string function */
void
srtgti(ent)
struct srtent *ent;
{
    ent->srt_next = ent->srt_top;
    if (ent->srt_next == NULL)
	return;
    while (ent->srt_next->srt_less != NULL)
	ent->srt_next = ent->srt_next->srt_less;
}

/* srtgets - get next string from sorted list, NULL if none more. */
/* The least shall be first... and the greatest shall be last */
char *
srtgets(ent)
struct srtent *ent;
{
    register struct srtbl *s;		/* tmp */
    register char *p;			/* ret val */

    if ((s = ent->srt_next) == NULL)
	return NULL;
    p = s->srt_str;			/* ret val */
    if (s->srt_more != NULL) {
	s = s->srt_more;		/* go one more */
	while (s->srt_less != NULL)	/* then all the way less */
	    s = s->srt_less;
	ent->srt_next = s;
	return p;
    }
    while (s != ent->srt_top && s->srt_prev->srt_more == s)
	s = s->srt_prev;		/* back down any more's */
    if (s == ent->srt_top)
	s = NULL;			/* no more */
    else
	s = s->srt_prev;
    ent->srt_next = s;
    return p;
}

/* srtinit - init the database tag */
void
srtinit(ent)
struct srtent *ent;
{
    ent->srt_top = NULL;		/* erase any knowledge of it */
    ent->srt_next = NULL;		/* extractions at the begining */
}

/* srtfree - delete all the data, init the tag */
void
srtfree(ent)
struct srtent *ent;
{
    if (ent->srt_top == NULL)		/* is the structure empty? */
	return;				/* yes - exit */
    srtdtree(ent->srt_top);
    free((char *)ent->srt_top);		/* clean up last struct */
    srtinit(ent);			/* init the tag */
}

/* srtdtree - recursive tree delete
 * frees all less & more entries pointed to by the srt struct */
void
srtdtree(srt)
register struct srtbl *srt;
{
    if (srt->srt_less != NULL) {
	srtdtree(srt->srt_less);
	free((char *)srt->srt_less);
	srt->srt_less = NULL;		/* for consistency */
    }
    if (srt->srt_more != NULL) {
	srtdtree(srt->srt_more);
	free((char *)srt->srt_more);
	srt->srt_more = NULL;
    }
}

/* srtin - insert string sorted & unique.
 * returns NULL if good, else error message string. */
char *
srtin(ent, str, compar)
struct srtent *ent;
char *str;
int (*compar)();			/* if NULL: use strcmp */
{
    register char *p;			/* temp string pointer */
    register i;				/* string compare result */
    register struct srtbl *s;		/* temp */
    register struct srtbl *srtold;	/* old temp */

    s = srtold = ent->srt_top;
    while (s != NULL) {
    	if ((i = (compar == NULL ?
		strcmp(str, s->srt_str) : (*compar)(str, s->srt_str))) == 0)
	    return NULL;		/* found: do nothing - "good" */
	srtold = s;
	if (i > 0)
	    s = s->srt_more;
	else
	    s = s->srt_less;
    }
    if ((p = malloc((unsigned)(strlen(str) + sizeof(struct srtbl)))) == NULL)
	return "srtin: warning - out of memory"; /* soft error - caller \n */
    s = (struct srtbl *)p;
    if (srtold == NULL) {		/* empty list */
	ent->srt_top = s;
	srtold = ent->srt_top;
    } else {
	if (i > 0)
	    srtold->srt_more = s;
	else
	    srtold->srt_less = s;
    }
    s->srt_prev = srtold;
    s->srt_less = NULL;
    s->srt_more = NULL;
    (void) strcpy(s->srt_str, str);
    return NULL;
}
