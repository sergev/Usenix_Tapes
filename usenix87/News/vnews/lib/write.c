#include <stdio.h>
#include "af.h"
#include "libextern.h"

long lseek();


/*
 * Write a record to the data base.  We can't use stdio because fseek
 * doesn't work under 4.1 BSD.
 */

writerec(a, fd)
      struct artrec *a ;
      int fd ;
      {
      int i ;
      register char *p, *q ;
      char **pp ;

      bfr[0] = A_PREFIX ;
      bcopy((char *)a, bfr + 1, A_WRTLEN) ;
      bcopy((char *)a->a_group, bfr + 1 + A_WRTLEN, i = a->a_ngroups * sizeof(*a->a_group)) ;
      p = bfr + 1 + A_WRTLEN + i ;
      pp = &a->a_ident ;
      i = (a->a_flags & A_DUMMY)? 1 : 4 + a->a_nkwords ;
      while (--i >= 0) {
            for (q = *pp++ ; *p++ = *q++ ; ) ;
      }
      if (write(fd, bfr, p - bfr) != p - bfr)
            xerror("Write error in writerec") ;
}



writeptr(p, dp)
      DPTR p, dp ;
      {
      if (lseek(affd, (long)dp, 0) < 0)
            xerror("seek %ld failed, writeptr %ld", dp, p) ;
      if (write(affd, (char *)&p, sizeof p) != sizeof p)
            xerror("write failed, writeptr(%ld, %ld)", p, dp) ;
}
