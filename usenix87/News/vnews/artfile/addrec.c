#include <stdio.h>
#include "af.h"


static int nchildren ;			/* number of orphaned children */
static DPTR child[200] ;		/* list of orphans */

long lseek() ;


/*
 * Add a record to the data base.
 */

DPTR
addrec(a, parlist)
      struct artrec *a ;
      char **parlist ;
      {
      DPTR pos ;
      DPTR idhash, thash ;
      DPTR ngptr[MAXNG] ;
      DPTR dp ;
      int i, j ;
      static char nuls[60] ;
      DPTR htitle();

      dorelatives(a, parlist) ;
      idhash = hashid(a->a_ident) ;
      a->a_idchain = readptr(idhash) ;
      if ((a->a_flags & A_DUMMY) == 0) {
            thash = htitle(a->a_title) ;
            a->a_titlechain = readptr(thash) ;
      }
      for (i = 0 ; i < a->a_ngroups ; i++) {
            struct artrec a2 ;
            int ng = a->a_group[i].a_ngnum ;

            /* insert record in proper place in chain */
            dp = readptr(ngptr[i] = ngchain(ng)) ;
            while (dp != DNULL) {
                  readrec(dp, &a2) ;
                  for (j = 0 ; ; j++) {
                        if (j >= a2.a_ngroups)
                              xerror("bad newsgroup chain for %d", ng) ;
                        if (a2.a_group[j].a_ngnum == ng)
                              break ;
                  }
                  if (a2.a_group[j].a_artno <= a->a_group[i].a_artno)
                        break ;
                  ngptr[i] = dp + GROUPOFF + stroff(a_ngchain, artgroup) + j * sizeof(struct artgroup) ;
                  dp = a2.a_group[j].a_ngchain ;
            }
            a->a_group[i].a_ngchain = dp ;
      }
      pos = afhd.af_free ;
      lseek(affd, (long)pos, 0) ;
      writerec(a, affd) ;
      afhd.af_free = lseek(affd, 0L, 1) ;
      write(affd, nuls, 60) ;		/* readrec tends to overrun */
      lseek(affd, 0L, 0) ;
      write(affd, (char *)&afhd, sizeof afhd) ;
      writeptr(pos, idhash) ;
      if ((a->a_flags & A_DUMMY) == 0)
            writeptr(pos, thash) ;
      if (a->a_parent)
            writeptr(pos, a->a_parent + CHILDRENOFF) ;
      for (i = 0 ; i < nchildren ; i++) {
            writeptr(pos, child[i] + PARENTOFF) ;
      }
      for (i = 0 ; i < a->a_ngroups ; i++) {
            writeptr(pos, ngptr[i]) ;
      }
      return pos ;
}



/*
 * Find the parents and children of the article.
 * If the references line is present,
 *    create a dummy entry for parent if necessary.
 * If no references line, guess at parent using title.
 * Look for articles without references lines whose titles indicate that
 *    this article might be the parent.
 */

dorelatives(a, parlist)
      register struct artrec *a ;
      char **parlist ;
      {
      struct artrec a2 ;
      DPTR dp, dp2, dummy ;
      int i ;

      /* first look for parent */
      if (parlist && parlist[0]) {
            if ((a->a_parent = lookart(parlist[0], &a2)) == DNULL) {
                  /* make dummy parent */
                  bzero((char *) &a2, sizeof a2) ;
                  a2.a_ident = parlist[0] ;
                  a2.a_flags = A_DUMMY ;
                  a2.a_rectime = a->a_rectime;
                  a2.a_subtime = a->a_subtime;
                  a->a_parent = addrec(&a2, parlist + 1) ;
            }
      } else if (a->a_title && isre(a->a_title)) {
            for (dp = readptr(htitle(a->a_title)) ; dp != NULL ; dp = a2.a_titlechain) {
                  readrec(dp, &a2) ;
                  if (tparent(&a2, a)) {
                        a->a_parent = dp ;
                        break ;
                  }
            }
      }
      /* check for loops in parent relation */
      if ((dummy = lookart(a->a_ident, &a2)) != DNULL) {
            /* we are replacing a dummy entry with a real one */
            i = 100 ;
            for (dp2 = a->a_parent ; dp2 != DNULL ; dp2 = a2.a_parent) {
                  readrec(dp2, &a2) ;
                  if (dp2 == dummy) {
                        a->a_parent = a2.a_parent ;
                        printf("loop in parent chain for %s found\n", a->a_ident) ;
                        break ;
                  }
                  if (--i < 0) {
                        printf("loop containing %s discovered while updating %s\n",
                              a2.a_ident, a->a_ident) ;
                        break ;
                  }
            }
      }

      /* now look for orphans */
      nchildren = 0 ;
      if (dummy != DNULL) {
            /* we are replacing a dummy entry with a real one */
            readrec(dummy, &a2) ;
            if (a2.a_parent != DNULL)
                  rmchain(a2.a_parent + CHILDRENOFF, CHILDCHOFF, dummy) ;
            for (dp = a2.a_children ; dp != DNULL ; dp = a2.a_childchain) {
                  child[nchildren++] = dp ;
                  readrec(dp, &a2) ;
            }
            writeptr(DNULL, dp + CHILDRENOFF) ;
      }
      if (a->a_parent != DNULL) {
            readrec(a->a_parent, &a2);
            a->a_childchain = a2.a_children ;
      } else
            a->a_childchain = DNULL ;
      if ((a->a_flags & A_DUMMY) == 0)
         for (dp = readptr(htitle(a->a_title)) ; dp != DNULL ; dp = a2.a_titlechain) {
            readrec(dp, &a2) ;
            if (a2.a_parent == DNULL && tparent(a, &a2))
                  child[nchildren++] = dp ;
      }

      /* link the orhpans together into the child chain */
      child[nchildren] = DNULL ;
      for (i = 0 ; i < nchildren ; i++) {
            DPTR pardp = readptr(child[i] + PARENTOFF) ;	/*DEBUG*/
            if (pardp != DNULL && pardp != dummy)
                  printf("bad orphan %ld: par %ld, dummy %ld\n", child[i], pardp, dummy) ;
            writeptr(child[i + 1], child[i] + CHILDCHOFF) ;
      }
      a->a_children = child[0] ;
}



/* remove an entry from a linked list */

rmchain(chain, offset, rec)
      DPTR chain ;
      DPTR rec ;
      {
      DPTR dp ;
      int nremoved = 0 ;	/*DEBUG*/
      DPTR svchain = chain ;	/*DEBUG*/

      dp = readptr(chain) ;
      while (dp != DNULL) {
            if (dp == rec) {
                  writeptr(readptr(dp + offset), chain) ;
                  nremoved++;
            }
            chain = dp + offset ;
            dp = readptr(chain) ;
      }
      if (nremoved == 0)
            printf("rmchain(%ld, %d, %ld) failed\n", svchain, offset, rec) ;
}
