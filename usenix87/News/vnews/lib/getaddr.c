/*
 * Get the machine address of a person, stripping off the real name.
 */

#include <stdio.h>
#include "config.h"
#include "defs.h"

char *
getaddr(full, mach)
      char full[] ;
      char mach[NAMELEN] ;
      {
      register char *p, *q, *r ;
      char *index(), *rindex();

      p = full ;
      if ((q = index(p, '(')) == NULL)
            q = p + strlen(p) ;
      if ((p = index(full, '<')) != NULL && p < q && (r = rindex(p, '>')) != NULL) {
            p++ ;
            q = r ;
      } else
            p = full ;
      while (*p == ' ')
            p++ ;
      while (--q > p && *q == ' ') ;
      if (q > p + NAMELEN - 1)
            q = p + NAMELEN - 1 ;
      r = mach ;
      while (p <= q)
            *r++ = *p++ ;
      *r = '\0' ;
      return mach ;
}
