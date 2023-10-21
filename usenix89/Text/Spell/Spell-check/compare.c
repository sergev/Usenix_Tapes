
/* RCS Info: $Revision: 1.3 $ on $Date: 85/10/08 18:35:16 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/compare.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Defined:
 */

#include "spellfix.h"

/* This is the major routine -- it determines how well a misspelled word
 * and a correctly spelled word match. The higher the return value the
 * worse the match is. We use a stack here so that we can backtrack
 * (we don't, but we could...)
 */

struct action {
	int a_type;		/* What we did. */
	char *a_where;		/* A pointer into the bad string. */
	char a_letter;		/* What letter we used, if applicable. */
	int a_cost;		/* How much this cost us. */
} ;

#define A_TRANS		1	/* Transpose a_where and a_where + 1. */
#define A_ADD		2	/* Added a_letter after a_where. */
#define A_DELETE	3	/* Deleted the letter at a_where. */
#define A_CHANGE	4	/* Changed the letter at w_where to a_letter. */

#define NACTIONS	256	/* Too many. */

extern unsigned char adjacent[NLETTERS][NLETTERS];

#define freq(x, y)	(adjacent[(x) - 'a'][(y) - 'a'])

compare(bad, good, maxcost)
	char *bad, *good;
	int maxcost;	/* If the cost gets higher than this, give up. */
{
	register int sp = 0;		/* Action stack pointer. */
	struct action astack[NACTIONS];	/* The action stack. */
	register char *badplace = bad;	/* Where we are now. */
	register char *goodplace = good;
	int nmatched = 0;		/* How many letters we have matched. */
	int roll = 0;			/* How long we've been on a roll. */
	int cost = 0, ntrans = 0, nadds = 0, ndels = 0, nchanges = 0;
	register int tempcost = 0, i, j;

	while (*badplace && *goodplace) {
		/* See what we can do with *badplace and *goodplace. */
		if (*badplace == *goodplace) {
			/* Great, they match. */
			badplace++;
			goodplace++;
			nmatched++;
			roll++;
		} else if ((badplace[0] == goodplace[1]) &&
				(goodplace[0] == badplace[1])) {
			/* Transpose these two. Neither can be NULL here. */
			astack[sp].a_type = A_TRANS;
			astack[sp].a_where = badplace;
			astack[sp].a_letter = 0;
			j = freq(badplace[0], badplace[1]) / 20;
			i = freq(goodplace[0], goodplace[1]) / 20;
			if ((badplace > bad) && (goodplace > good)) {
				j += freq(badplace[-1], badplace[0]) / 40;
				i += freq(goodplace[-1], goodplace[0]) / 40;
			}
			if (badplace[2] && goodplace[2]) {
				j += freq(badplace[1], badplace[2]) / 40;
				i += freq(goodplace[1], goodplace[2]) / 40;
			}
			if (i > 5)
				i = 5;
			if (j > 5)
				j = 5;
			astack[sp].a_cost = ++ntrans * 10 + j - i;
			tempcost += astack[sp].a_cost;
			roll = 0;
			sp++;
			badplace += 2;
			goodplace += 2;
		} else if (goodplace[0] == badplace[1]) {
			/* Delete *badplace. */
			astack[sp].a_type = A_DELETE;
			astack[sp].a_where = badplace;
			astack[sp].a_letter = 0;
			if (badplace > bad) {
				j = (freq(badplace[-1], badplace[0]) + 
					freq(badplace[0], badplace[1]))/ 20;
				i = freq(badplace[-1], badplace[1]) / 10;
				if (i > 10)
					i = 10;
				if (j > 10)
					j = 10;
			} else
				i = j = 0;
			astack[sp].a_cost = ++ndels * 20 + j - i;
			tempcost += astack[sp].a_cost;
			roll = 0;
			sp++;
			badplace++;
		} else if (badplace[0] == goodplace[1]) {
			/* Add *goodplace. */
			astack[sp].a_type = A_ADD;
			astack[sp].a_where = badplace - 1;
			astack[sp].a_letter = goodplace[0];
			if (badplace[1]) {
				i = (freq(badplace[0], goodplace[0]) + 
					freq(goodplace[0], badplace[1]))/20;
				j = freq(badplace[0], badplace[1]) / 10;
				if (i > 10)
					i = 10;
				if (j > 10)
					j = 10;
			} else
				i = j = 0;
			astack[sp].a_cost = ++nadds * 20 + j - i;
			tempcost += astack[sp].a_cost;
			roll = 0;
			sp++;
			goodplace++;
		} else {
			/* Change *badplace to *goodplace. This is a last
			 * resort.
			 */
			astack[sp].a_type = A_CHANGE;
			astack[sp].a_where = badplace;
			astack[sp].a_letter = *goodplace;
			if ((badplace > bad) && badplace[1]) {
				j = (freq(badplace[-1], badplace[0]) + 
					freq(badplace[0], badplace[1]))/10;
				i = (freq(badplace[-1], goodplace[0]) + 
					freq(goodplace[0], badplace[1]))/10;
				if (i > 20)
					i = 20;
				if (j > 20)
					j = 20;
			} else
				i = j = 0;
			astack[sp].a_cost = ++nchanges * 30 - i + j;
			tempcost += astack[sp].a_cost;
			roll = 0;
			sp++;
			badplace++;
			goodplace++;
		}
		if (tempcost > maxcost)
			return (NOCHANCE);
	}
	while (*badplace) {
		/* Delete all these characters. */
		astack[sp].a_type = A_DELETE;
		astack[sp].a_where = badplace;
		astack[sp].a_letter = 0;
		astack[sp].a_cost = ++ndels * 20;
		tempcost += astack[sp].a_cost;
		roll = 0;
		sp++;
		badplace++;
	}
	if (tempcost > maxcost)
		return (NOCHANCE);
	while (*goodplace) {
		/* Add all these characters. */
		astack[sp].a_type = A_ADD;
		astack[sp].a_where = badplace - 1;
		astack[sp].a_letter = goodplace[0];
		astack[sp].a_cost = ++nadds * 20;
		tempcost += astack[sp].a_cost;
		roll = 0;
		sp++;
		goodplace++;
	}
	if (tempcost > maxcost)
		return (NOCHANCE);
	return (tempcost);
}

