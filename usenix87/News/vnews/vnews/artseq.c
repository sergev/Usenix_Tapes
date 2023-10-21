#include "vnews.h"

#ifdef SMALL			/* try to avoid running out of core */
#define MAXNUMARTS	100	/* max arts we can read in one newsgroup */
#endif

static struct svartlist alstack[10];	/* stack of article lists */
struct svartlist *alptr = alstack;	/* stack pointer for above */
static int readmode = NEXT;		/* currently unused */
static struct ngentry *ngindex;		/* current location in .newsrc file */


/*
 * set or clear the unread article flag
 */
setnew(on) {
	if (alptr->al_type != SVNEWSGROUP)
		return;
	if (on)
		setunread(curart->i_artnum);
	else
		clrunread(curart->i_artnum);
}


/*
 * Go to a particular article number
 */

setartno(num)
int num;
{
	if (num <= 0) {
		msg("Bad article no.");
		return;
	}
	if (num > numthisng) {
		msg("Not that many articles.");
		return;
	}
	curart = &thisng[num - 1];
	setupart();
}


/*
 * Advance the counter to the next unread article.
 */
nextart(count, zapthem) 
{
	if (count <= 0)
		return FALSE;
	while (--count >= 0) {
		if (zapthem)
			setnew(0);
		if (readmode == SPEC || xflag) {
			curart++, curind++;
		} else {
			while (++curart, ++curind <= numthisng && !isunread(curart->i_artnum))
				;
		}
		if (curind > numthisng)
			return getnxtng(FORWARD);
	}
	setupart();
	return FALSE;
}


/*
 * Vnews article list stack.  Pushal(type) pushes the current article list
 * onto the stack.  Type is the type of the replacement article list.  Popal
 * restores an article list from the stack.  Setupart should be called after
 * popal.
 */

struct alsave {
	int a_numthisng;
	int a_curind;
	struct ngentry *a_curng;
	int a_iwartlin;
	int a_indexpg;
	int a_maxartno;
};


pushal(type) {
	struct alsave als;

	if (alptr >= alstack + 9) {
		msg("newsgroups nested too deeply");
		alptr->al_type = type;
		return;
	}
	alptr->al_tfoffset = tfoffset;
	als.a_numthisng = numthisng;
	als.a_curind = curind;
	als.a_curng = curng;
	als.a_iwartlin = indexwin.w_artlin;
	als.a_indexpg = indexpg;
	als.a_maxartno = maxartno;
	fseek(tfp, (long)tfoffset, 0);
	fwrite((char *)&als, sizeof als, 1, tfp);
	fwrite((char *)thisng, sizeof *thisng, numthisng, tfp);
	tfoffset = ftell(tfp);
	tflinno = -1;
	(++alptr)->al_type = type;
}


popal() {
	struct alsave als;

	tfoffset = (--alptr)->al_tfoffset;
	fseek(tfp, (long)tfoffset, 0);
	fread((char *)&als, sizeof als, 1, tfp);
	numthisng = als.a_numthisng;
	curind = als.a_curind;
	indexwin.w_artlin = als.a_iwartlin;
	indexpg = als.a_indexpg;
	fread((char *)thisng, sizeof *thisng, numthisng, tfp);
	indexwin.w_force = 2;
	tflinno = -1;
	if ((alptr+1)->al_type == SVNEWSGROUP)
		setupgrp(als.a_curng, als.a_maxartno);
	setartno(curind);
}



switchng(ngp)		/* go to specific newsgroup */
	struct ngentry *ngp;
	{
	if (alptr >= alstack + 8) {
		msg("Nesting too deep.");
		return;
	}
	if (alptr->al_type == SVARTICLE)
		popal();
	pushal(SVNEWSGROUP);
	readinng(ngp, 1);
	if (numthisng <= 0) {
		popal();
		msg("group? %s: empty", ngp->ng_name);
	} else {
		ngrp++;
	}
	setupart();
}


spclgrp(dp, a)
	DPTR dp;
	struct artrec *a;
	{
	if (alptr->al_type != SVARTICLE)
		pushal(SVARTICLE);
	scopyn(a->a_title, thisng[0].i_title, 36);
	scopyn(a->a_from, thisng[0].i_from, 29);
	thisng[0].i_nlines = a->a_nlines;
	thisng[0].i_artnum = a->a_group[0].a_artno;
	thisng[0].i_dptr = dp;
	numthisng = 1;
	curart = thisng;
	setupart();
	sprintf(filename, "%s/%s", SPOOL, a->a_file);
	indexpg = 0;
}


getnxtng(direct) {
	if (direct == FORWARD && alptr > alstack) {	/* al stack stuff */
		popal();
		setupart();
		/* advance to next article if current one read */
		if ((alptr+1)->al_type == SVNEWSGROUP
		 && isunread(curart->i_artnum) == 0)
			return nextart(1, 0);
		return FALSE;
	}
	while (alptr > alstack)		/* for backwards, clear al stack */
		popal();

	for (;;) {
		if (direct == FORWARD) {
			if ((ngindex = nextgrp(ngindex)) == NULL) {
				quitflg++;
				return TRUE;
			}
		} else {
			if ((ngindex = prevgrp(ngindex)) == NULL) {
				msg("Can't back up.");
				direct = FORWARD;
				continue;
			}
		}
		if (ngindex->ng_unsub || !wewant(ngindex->ng_name))
			continue;
		readinng(ngindex, 0);
		if (numthisng > 0)
			break;
	}
	ngrp++;
	setupart();
	return FALSE;
}


setupart() {
	if (ngrp) {
		pngsize = maxartno;
		if (Aflag)
			indexpg = 1;
		if (indexpg)
			ngrp = 0;
	}
	curind = curart - thisng + 1;
#ifdef DEBUG
	fprintf(stderr, "getnextart settles on #%d, bit %d\n", curind, curart->i_artnum);
#endif
	dirname(curng->ng_name, filename);
	sprintf(filename + strlen(filename), "/%d", curart->i_artnum);
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	news = TRUE;
	indexwin.w_force |= 1;
	erased = 0;
}

/*
 * Read in the newsgroup.
 */
readinng(ngp, all)
struct ngentry *ngp;
{
	register struct artinfo *ip;
	struct artrec a, a2;
	DPTR dp, dp2;
	int ngnum = ng_num(ngp);
	ARTNO bit, oldbit;
	int first;
	int i;
	static int thisngsize = 20;
	int cmppart();
	char *ckmalloc(), *realloc();

	if (news) {
		curflag = CURHOME;
#ifdef CURSES
		move(0, 0);	/* let user know we are thinking */
		refresh();
#else
		_amove(0, 0);	/* let user know we are thinking */
		vflush();
#endif
	}
	if (thisng == NULL) {
		thisng = ckmalloc(20 * sizeof thisng[0]);
	}
	numthisng = 0;
	oldbit = 0;
	first = 1;
	BKWD_GROUP(ngnum, bit, dp, a) {
		if (first) {
			setupgrp(ngp, bit);
			first = 0;
		}
		while (--oldbit > bit)		/* clear intermediate bits */
			clrunread(oldbit);
		oldbit = bit;
		if (bit < minartno && !all)
			break;
		if (!(all || isunread(bit)))
			continue;
		/* later:  rethink the handling of all */
		if ((a.a_flags & A_NOFILE) || !aselect(&a, 0) || haveunsub(dp)) {
skipit:			clrunread(bit);
			continue;
		}
		a2.a_subtime = a.a_subtime;
		for (dp2 = a.a_parent, i = 0 ; dp2 != DNULL && ++i < 100 ; dp2 = a2.a_parent) {
			if (haveunsub(dp2))
				goto skipit;
			readrec(dp2, &a2);
		}
		if (i == 100)
			fprintf(stderr, "Parent loop %s %s \r\n", a.a_ident, a2.a_ident);	/*DEBUG*/
		if (numthisng >= thisngsize) {
			thisngsize += 4;
			if ((thisng = realloc((char *)thisng, thisngsize * sizeof thisng[0])) == NULL)
				xerror("Out of space");
		}
		ip = &thisng[numthisng];
		ip->i_artnum = bit;
		ip->i_subtime = a.a_subtime;
		ip->i_basetime = a2.a_subtime;
		scopyn(a.a_title, ip->i_title, 37);
		scopyn(a.a_from, ip->i_from, 29);
		ip->i_dptr = dp;
		ip->i_nlines = a.a_nlines;
#ifndef DEBUG
		if (debugging > 1)
#endif
			fprintf(stderr, "storing %d in %d, subtime %d\n", bit, numthisng, ip->i_subtime);
		numthisng++;
#ifdef MAXNUMARTS
		if (numthisng >= MAXNUMARTS) {
			printf("Warning - more than %d new articles, missing some.\n", MAXNUMARTS);
			goto nomore;		/* exit loop */
		}
#endif
	}
	while (--oldbit >= minartno)
		clrunread(oldbit);
nomore:
	if (numthisng > 0) {
		qsort((char *)thisng, numthisng, sizeof thisng[0], cmppart);
	}
	curart = thisng;		/* go to start of group */
	indexwin.w_artlin = 0;
	indexwin.w_force = 2;
}


/*
 * Check whether we ahve unsubscribed to the discussion.
 */
haveunsub(dp)
	DPTR dp;
	{
	register DPTR *p;
	register int i;

	for (i = ndunsub, p = dunsub ; --i >= 0 ; p++)
		if (*p == dp)
			return 1;
	return 0;
}


/*
 * Compare two articles, and determine if they are in the same discussion.
 * If not, we sort by time of base article.
 */
cmppart(n1, n2)
register struct artinfo *n1, *n2;
{
	if (n1->i_basetime > n2->i_basetime)
		return 1;			/* different discussions */
	else if (n1->i_basetime < n2->i_basetime)
		return -1;			/* different discussions */
	else if (n1->i_subtime > n2->i_subtime)
		return 1;			/* same discussion */
	else if (n1->i_subtime < n2->i_subtime)
		return -1;			/* same discussion */
	else
		return 0;			/* same discussion */
}



/*
 * Return TRUE if the user has not ruled out this article.
 */
aselect(a, insist)
register struct artrec *a;
int	insist;
{
	extern char *titles[];

	if (insist)
		return TRUE;
	if (tflag && !titmat(a->a_title, titles))
		return FALSE;
	if (aflag && a->a_rectime < atime)
		return FALSE;
	if (a->a_ngroups > 1 && !rightgroup(a))
		return FALSE;
	if (fflag && a->a_parent != DNULL)
		return FALSE;
	return TRUE;
}



/*
 * Code to avoid showing multiple articles for vnews.
 * Works even if you exit vnews.
 * Returns nonzero if we should show this article.
 */
rightgroup(a)
	struct artrec *a;
	{
	int i, flag;
	struct ngentry *ngp;

	flag = 1;
	for (i = 0 ; i < a->a_ngroups ; i++) {
		ngp = ngtable + a->a_group[i].a_ngnum;
		if (ngp == curng)
			return flag;
		if (wewant(ngp->ng_name) && ngp->ng_unsub == 0) {
			flag = 0;
		}
	}
	/* "Can't happen" */
	return 1;
}



/*
 * Return true if the newsgroup was specified in the -n option.
 */

wewant(name)
	char *name;
	{
	return ngmatch(name, sublist);
}
