/*
 * Exchange two headers.
 */

#include <stdio.h>
#include <sys/types.h>
#include "arthead.h"

hxchg(hp1, hp2)
      struct arthead *hp1, *hp2 ;
      {
      register char *p, *q ;
      register char c ;
      register int i ;

      p = (char *)hp1, q = (char *)hp2 ;
      for (i = sizeof(struct arthead) ; --i >= 0 ; )
            c = *p, *p++ = *q, *q++ = c ;
}
