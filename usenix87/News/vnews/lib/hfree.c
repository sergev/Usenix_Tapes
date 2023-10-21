/*
 * Free up the storage used by a header.
 */

#include <stdio.h>
#include <sys/types.h>
#include "arthead.h"


hfree(hp)
      struct arthead *hp ;
      {
      register int i ;
      register char **p ;

      for (i = 8, p = hp->h_space ; --i >= 0 ; p++) {
            if (*p != NULL) {
                  free(*p) ;
                  *p = NULL ;
            }
      }
}
