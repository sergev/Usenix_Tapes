#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#include "defs.h"
#include "arthead.h"
#include "af.h"


#define equal(s1, s2)	(strcmp(s1, s2) == 0)
char *rindex(), *savestr() ;
char *getaddr() ;
time_t cgtdate() ;
DPTR addrec() ;

extern int newgroup ;


/*
 * Add an article to the data base.
 * Glitches:
 *	Ignore the message id <>, since it is clearly wrong (rn bug).
 *	Ignore message ids without a trailing '>' since probably truncated.
 */

addart(h, fname, groups)
      struct arthead *h ;
      char *fname ;
      char *groups ;
      {
      struct artrec a ;
      char buf[128] ;
      char *parlist[32], **pp ;
      struct artgroup *gp ;
      register char *p, *q ;

      h->h_subtime = cgtdate(h->h_subdate) ;
      h->h_rectime = cgtdate(h->h_recdate) ;
      if (hset(h->h_expdate))
            h->h_exptime = cgtdate(h->h_expdate) ;
      else
            h->h_exptime = 0L ;

      p = groups; gp = a.a_group ;
      while (*p && gp < &a.a_group[MAXNG]) {
            q = buf ;
            while (*p == ' ')
                  p++ ;
            while (*p != '/') {
                  if (*p == '\0')
                        xerror("bad newsgroup list") ;
                  *q++ = *p++ ;
            }
            *q = '\0' ;
            if ((gp->a_ngnum = ngnum(buf)) <= 0) {
                  gp->a_ngnum = newng(buf) ;
                  newgroup++ ;
            }
            p++ ;
            gp->a_artno = atoi(p) ;
            while (*p != ' ' && *p)
                  p++ ;
            gp++ ;
      }
      a.a_ngroups = gp - a.a_group ;

      pp = parlist + 31 ; parlist[31] = NULL ;
      p = h->h_references ;
      while (p && *p) {
            if (pp <= parlist)
                  xerror("Too many references") ;
            q = buf ;
            while (*p && *p != ',' && *p != ' ')
                  *q++ = *p++ ;
            *q = '\0' ;
            tomsgid(buf) ;
            if (! equal(buf, "<>"))
                  *--pp = savestr(buf) ;
            while (*p == ' ' || *p == ',')
                  p++ ;
      }
      while (*pp && index(*pp, '>') == 0)
            pp++ ;
      if (strlen(h->h_title) > 127)
            h->h_title[127] = '\0' ;
      getaddr(h->h_from, buf) ;

      a.a_subtime = h->h_subtime ;
      a.a_rectime = h->h_rectime ;
      a.a_exptime = h->h_exptime ;
      a.a_parent = DNULL ;
      a.a_children = DNULL ;
      a.a_childchain = DNULL ;
      a.a_nlines = h->h_intnumlines ;
      a.a_flags = 0 ;
      a.a_ident = h->h_ident ;
      a.a_title = h->h_title ;
      a.a_from = buf ;
      a.a_file = fname ;
      a.a_nkwords = 0 ;		/* fix this eventually */
      addrec(&a, pp) ;

      while (*pp)
            free(*pp++) ;
}
