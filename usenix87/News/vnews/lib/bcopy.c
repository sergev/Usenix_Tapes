#include "config.h"

#if BSDREL < 42

bcopy(from, to, n)
      register char *from, *to ;
      {
      register int i ;

      if ((i = n) != 0) {
            do *to++ = *from++ ;
            while (--i != 0) ;
      }
}

#endif
