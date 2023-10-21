/*
 * Routines step through all the elements of a newsgroup.
 * Called by the BNG macro.
 */

#include <stdio.h>
#include "af.h"


DPTR nglnext ;


DPTR
nglfirst(ngnum) {
      return nglnext = readptr(ngchain(ngnum)) ;
}


ARTNO
ngltest(ngnum, a)
      int ngnum ;
      struct artrec *a ;
      {
      register int i ;

      if (nglnext == DNULL)
            return -1 ;
      readrec(nglnext, a) ;
      for (i = 0 ; i < a->a_ngroups ; i++) {
            if (a->a_group[i].a_ngnum == ngnum) {
                  nglnext = a->a_group[i].a_ngchain ;
                  return a->a_group[i].a_artno ;
            }
      }
      xerror("bad newsgroup chain") ;
}
