#include <stdio.h>
#include "af.h"

#define MAXTITLE 256	/* where does this belong? */
#define equal(s1, s2)	(strcmp(s1, s2) == 0)


/*
 * Return a pointer to the hash chain for this title.
 */

DPTR
htitle(title)
      register char *title ;
      {
      char buf[MAXTITLE] ;

      while (isre(title))
            for (title += 3 ; *title == ' ' ; title++) ;
      strcpy(buf, title) ;
      rmnf(buf) ;
      return afhd.af_titletab + hash(buf, afhd.af_ttlen) * sizeof(DPTR) ;
}



/*
 * Determine if a1 is the parent of a2 using titles.
 */

tparent(a1, a2)
      struct artrec *a1, *a2 ;
      {
      char t1[MAXTITLE], t2[MAXTITLE] ;
      char *p ;

      if (a1->a_subtime > a2->a_subtime || !isre(a2->a_title))
            return 0 ;
      strcpy(t1, a1->a_title), strcpy(t2, a2->a_title) ;
      if (rmnf(t1)) {
            if (!rmnf(t2))
                  return 0 ;
      } else {
            rmnf(t2) ;
      }
      for (p = t2 + 3 ; *p == ' ' ; p++) ;	/* skip over "Re:" */
      if (!equal(t1, p))
            return 0 ;
      return 1 ;
}
