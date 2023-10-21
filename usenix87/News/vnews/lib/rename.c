#include <errno.h>
#include "config.h"

#if BSDREL < 42

int
rename(from, to)
      char *from, *to ;
      {
      extern int errno ;

      if (link(from, to) < 0) {
            if (errno != EEXIST)
                  return -1 ;
            if (unlink(to) < 0 || link(from, to) < 0)
                  return -1 ;
      }
      return unlink(from) ;
}

#endif
