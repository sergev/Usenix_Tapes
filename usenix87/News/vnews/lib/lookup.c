#include <stdio.h>
#include "af.h"


/*
 * Look up an article using the article id.
 */

DPTR
lookart(id, a)
      char *id ;
      struct artrec *a ;
      {
      DPTR dp ;

      dp = readptr(hashid(id)) ;
      while (dp != DNULL) {
            readrec(dp, a) ;
            if (equal(id, a->a_ident)) {
                  /*printf("lookart %s succeeded\n", id) ;	/*DEBUG*/
                  return dp ;
            }
            dp = a->a_idchain ;
      }
      /*printf("lookart %s failed\n", id) ;			/*DEBUG*/
      return DNULL ;
}
