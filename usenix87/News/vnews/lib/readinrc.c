/*
 * Read in the .newsrc file and the newsgroups file.  If rcfp is NULL,
 * only the newsgroups file is read.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "defs.h"
#include "artfile.h"
#include "ng.h"
#include "newsrc.h"
#include "libextern.h"
#include "str.h"

static int rcmode;		/* file modes for .newsrc file */
int rcreadok;			/* .newsrc file has been read */
int rcsaved;			/* .newsrc file has been backed up */
char *rcjunk[MAXRCJUNK];	/* comment lines in .newsrc file */
char **nextrcjunk = rcjunk;	/* first unused entry in rcjunk */
struct ngentry ngtable[MAXGROUPS];	/* newsgroup table */
int ndunsub;			/* number of discussions unsubscribed to */
DPTR dunsub[MAXUNSUB];		/* discussions unsubscribed to */
struct ngentry *firstng, *lastng;	/* articles in .newsrc file */
struct ngentry *curng;		/* current newsgroup */

struct ngentry **nghashtab, *hashng();


readinrc(rcfp)
	FILE *rcfp;
	{
	register struct ngentry *ngp;
	register char *p;
	char unsub;
	int i;
	DPTR dp;
	struct artrec a;
	struct ngrec g;
	struct stat statb;

	gfopen();
	if (maxng >= MAXGROUPS)
		xerror("Too many groups, increase MAXGROUPS");
	nghashtab = ckmalloc(i = (MAXGROUPS * 2 + 1) * sizeof *nghashtab);
	bzero((char *)nghashtab, i);
	ALL_GROUPS(g) {
		ngtable[g.g_num].ng_name = nsavestr(g.g_name);
		hashng(g.g_name, &ngtable[g.g_num]);
	}
	gfclose();
	if (rcfp == NULL)
		goto out;
	if (fstat(fileno(rcfp), &statb) >= 0)
		rcmode = statb.st_mode;
	while (fgets(bfr, LBUFLEN, rcfp) != NULL) {
		if (!nstrip(bfr))
			xerror(".newsrc line too long");
		if (bfr[0] == '<') {
			char ident[NAMELEN];
			scopyn(bfr, ident, NAMELEN);
			if (ndunsub >= MAXUNSUB)
				xerror("Unsubscribed to too many discussions");
			if ((dp = lookart(ident, &a)) != DNULL)
				dunsub[ndunsub++] = dp;
		} else if (islower(bfr[0])) {		/* newsgroup line? */
			for (p = bfr ; *p != ':' && *p != '!' ; p++)
				if (*p == '\0' || *p == ' ' || *p == '\t')
					goto junk;
			unsub = (*p == '!');
			*p++ = '\0';
			if (*p == ' ')
				p++;
			ngp = hashng(bfr, (struct ngentry *)0);
			if (ngp != NULL) {
				if (ngp->ng_bits != NULL) {
					free(ngp->ng_bits);
				} else {
					addrc(ngp);
				}
				ngp->ng_bits = savestr(p);
				ngp->ng_unsub = unsub;
			}
		} else {
junk:			if (nextrcjunk >= &rcjunk[MAXRCJUNK])
				xerror("Too many lines in .newsrc");
			*nextrcjunk++ = savestr(bfr);
		}
	}
	free(nghashtab);
	rcreadok = 1;

out:
	/* add new groups to the .newsrc file */
	for (ngp = ngtable ; ngp < ngtable + MAXGROUPS ; ngp++) {
		if (ngp->ng_name != NULL && ngp->ng_bits == NULL && wewant(ngp->ng_name))
			addrc(ngp);
	}
}



/*
 * Write out the .newsrc file.
 */
writeoutrc()
{
	FILE *rcfp;
	register int i;
	char **jp;
	struct ngentry *ngp;
	char newstmp[FPATHLEN];
	char newsbak[FPATHLEN];
	struct artrec a;
	FILE *ckfopen();

	if (!rcreadok)
		return;
	updaterc();
	sprintf(newstmp, "%s.tmp", newsrc);
	rcfp = ckfopen(newstmp, "w");
	if (rcmode)
		chmod(newstmp, rcmode);

	/* Write out options line, continuations, and comments. */
	for (jp = rcjunk ; jp < nextrcjunk ; jp++)
		fprintf(rcfp, "%s\n", *jp);

	/* Write out the newsgroup lines. */
	for (ngp = firstng ; ngp != NULL ; ngp = ngp->ng_next) {
		fprintf(rcfp, "%s%c", ngp->ng_name, ngp->ng_unsub? '!' : ':');
		if (ngp->ng_bits != NULL && *ngp->ng_bits != '\0')
			fprintf(rcfp, " %s", ngp->ng_bits);
		putc('\n', rcfp);
	}
	/* write out a list of discussions we have unsubscribed to */
	for (i = 0 ; i < ndunsub ; i++) {
		readrec(dunsub[i], &a);
		fprintf(rcfp, "%s\n", a.a_ident);
	}
	if (ferror(rcfp) || fclose(rcfp) == EOF)
		xerror("write error on .newsrc.tmp");
	sprintf(newsbak, "%s.bak", newsrc);
	if (! rcsaved) {
		rename(newsrc, newsbak);
		rcsaved++;
	}
	if (rename(newstmp, newsrc) < 0)
		xerror("rename .newsrc.tmp to .newsrc failed");
}


struct ngentry *
hashng(name, entry)
      char *name;
      struct ngentry *entry;
      {
      register char *p;
      register int i;
      register struct ngentry *ngp;

      i = 0;
      for (p = name ; *p ; i += *p++);
      if (p > name + 2)
            i += (p[-2] << 2) + (p[-1] << 1);
      i &= 077777;			/* protect against negetive chars */
      for (i = i % (MAXGROUPS * 2 + 1) ; (ngp = nghashtab[i]) != NULL ; ) {
            if (strcmp(ngp->ng_name, name) == 0)
                  return ngp;
            if (++i >= (MAXGROUPS * 2 + 1))
                  i = 0;
      }
      if (entry != NULL)
            nghashtab[i] = entry;
      return ngp;
}
