/* abbreviation related routines 
 * Written & hacked by Stephen Uitti, PUCC staff, ach@pucc-j, 1985
 * maketd is copyright (C) Purdue University, 1985
 *
 * Permission is hereby given for its free reproduction and
 * modification for non-commercial purposes, provided that this
 * notice and all embedded copyright notices be retained.
 * Commercial organisations may give away copies as part of their
 * systems provided that they do so without charge, and that they
 * acknowledge the source of the software.
 */

#ifdef BSD2_9
#include <sys/types.h>
#endif
#include <stdio.h>
#include <ctype.h>
extern char *malloc();

#include "srtunq.h"
#include "abrv.h"
#include "maketd.h"

struct srtent abrv;			/* include file abrevs */
char   *abrvtbl[MXABR];			/* translation table strings */
int	abrvlen[MXABR];			/* string lengths (for speed) */

/* lngsrt - string length more important than lexicographical compare.
 * return > 0 if b is longer than a.
 * return < 0 if b is shorter than a.
 * if a & b are the same length, return strcmp(a, b), which means
 * that 0 is returned if the strings are THE SAME,
 * if b > a: return > 0
 * if b < a: return < 0
 */
int
lngsrt(a, b)
char *a, *b;
{
    register i;

    if ((i = strlen(b) - strlen(a)) != 0)
	return i;
    else
	return strcmp(a, b);
}

/* hincl - include header optimizer:
 * Compress multiple leading /'s to just one.
 * Remove leading "./".
 * Doesn't change date, just returns pointer into begining of path.
 */
char *
hincl(p)
register char *p;
{
    if (*p == '/')		/* compress multiple leading /'s */
	while (p[1] == '/')	/* to just one */
	    p++;
    if (strncmp("./", p, 2) == 0) {
	p += 2;			/* leading "./" can confuse make */
	while (*p == '/')	/* don't change ".//a.h" to "/a.h" */
	    p++;
    }
    return p;
}

/* srchincl - search line for make defines of A-Z
 * Put entries into abrvtbl.
 */
void
srchincl(p)
register char *p;
{
    register char *q, *r;
    register i;

    if (shortincl && *p != '\0') {
	while (*p == SPC || *p == '\t')
	    p++;		/* ignore white space */
	q = p;			/* what letter */
	if (isupper(*p)) {	/* A-Z, for now */
	    p++;
	    while (*p == SPC || *p == '\t')
		p++;		/* ignore white space */
	    if (*p++ == '=') {
		while (*p == SPC || *p == '\t')
		    p++;
		if ((i = strlen(p)) != 0) {
		    if ((r = malloc((unsigned)(i + 1))) == NULL)
			err("Out of memory in define search");
		    if (abrvtbl[*q - 'A'])
			fprintf(stderr, "%s: warning - %c redefined in %s\n",
			    prgnm, *q, makename);
		    abrvtbl[*q - 'A'] = r;
		    while (*p != '\0' && *p != '#' && *p != '\n')
			*r++ = *p++;
		    *r = '\0';	/* null terminate result */
		}		/* if non-null string */
	    }			/* if = */
	}			/* if A-Z */
    }				/* if shortinclude & string */
}

/* abrvsetup - set up abrev table, spit out the abrevs.	 Use
 * any A-Z definitions found in Makefile, no duplicates.  Add
 * /usr/include & /usr/include/sys if "all" dependencies are
 * being generated (including /usr/include based files).
 * Try to use I=/usr/include and S=/usr/include/sys, but don't
 * make a stink about it.
 */
void
abrvsetup()
{
    register i;				/* temp */
    register char *p;			/* temp */
    register slot;			/* slot search point */
    register j;				/* temp */
    static abrdone = FALSE;		/* do this only once */
    register flushi = FALSE;		/* print I=.. */
    register flushs = FALSE;		/* print S=.. */
    static char *istring = "I=/usr/include";
    static char *sstring = "S=/usr/include/sys";

    if (abrdone)
	return;
    if (shortincl) {
	abrdone = TRUE;			/* we've done this */
	/* add /usr/include/sys, /usr/include if not already there */
	if (alldep) {
	    if (abrvtbl['S'-'A'] == NULL) {
		flushs = TRUE;
		srchincl(sstring);
	    } else if ((p = srtin(&abrv, &sstring[2], lngsrt)) != NULL)
		fprintf(stderr, "%s: %s - %s\n", prgnm, p, &sstring[2]);
	    if (abrvtbl['I'-'A'] == NULL) {
		flushi = TRUE;
		srchincl(istring);
	    } else if ((p = srtin(&abrv, &istring[2], lngsrt)) != NULL)
		fprintf(stderr, "%s: %s - %s\n", prgnm, p, &istring[2]);
	}
	if (!replace) {			/* no new defines with replace */
	    srtgti(&abrv);		/* init tree traversal */
	    slot = 0;			/* start at bgn */
	    while ((p = srtgets(&abrv)) != NULL) {
		j = strlen(p);
		for (i = 0; i < MXABR; i++) { /* look for this definition */
		    if (abrvtbl[i] == NULL)
			continue;	/* check non-null entries */
		    if (*abrvtbl[i] == '\0')
			continue;	/* check non-null entries */
		    if (strcmp(p, abrvtbl[i]) == 0)
			break;		/* exact match found */
		    else if (strlen(abrvtbl[i]) == j + 1 &&
			strncmp(p, abrvtbl[i], j) == 0 &&
			    abrvtbl[i][j] == '/')
			break;		/* match of "p/" found */
		}
		if (i == MXABR) {	/* not found */
		    for (i = slot; i < MXABR; i++) /* find free slot */
			if (abrvtbl[i] == NULL)
			    break;
		    if (i < MXABR) {	/* free slot found */
		/* 	if (!usestdout && !replace) */
			    fprintf(makefd, "%c=%s\n", 'A' + i, p);
			abrvtbl[i++] = p;
			slot = i;	/* reduce free slot search time */
		    }
		}			/* if not found */
	    }				/* while */
	}				/* if !replace */
	if (flushi && !usestdout && !replace)
	    fprintf(makefd, "%s\n", istring);
	if (flushs && !usestdout && !replace)
	    fprintf(makefd, "%s\n", sstring);
	for (i = 0; i < MXABR; i++) {	/* set up string lengths */
	    if (abrvtbl[i] == NULL) {
		abrvlen[i] = 0;
	    } else {
		abrvlen[i] = strlen(abrvtbl[i]);
		if (verbose)
		    fprintf(stderr, "%s: %c='%s'\n",
			prgnm, i + 'A', abrvtbl[i]);
	    }				/* if */
	}				/* for */
    }					/* if */
}

/* findabr - find an abbreviation in abrvtbl for string p (if any).
 * if multiple abbreations work, use longest.
 * (ie: /usr/include & /usr/include/sys; use /usr/include/sys)
 * if found, return index
 * else: MXABR
 * Do not match abbreviations of less than 3 characters.
 */
int
findabr(p)
register char *p;			/* string pointer */
{
    register i;				/* for index */
    register j;				/* found index */

    for (i = 0, j = MXABR; i < MXABR; i++)
	if (abrvlen[i] > 2)		/* changing "." to $A is evil */
	    if (strncmp(abrvtbl[i], p, abrvlen[i]) == 0)
		if (j == MXABR || (abrvlen[i] > abrvlen[j]))
		    j = i;
    return j;
}
