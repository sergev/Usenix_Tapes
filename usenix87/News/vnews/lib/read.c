#include <stdio.h>
#include "af.h"
#include "libextern.h"		/* defines bfr */

long lseek() ;


/*
 * Read in an article record.
 */

readrec(dp, a)
      DPTR dp ;
      struct artrec *a ;
      {
      register int len ;
      int i ;
      register char *p ;
      register char *q ;
      char **pp ;

      /*printf("readrec %ld\n", (long)dp) ;		/*DEBUG*/
      if (lseek(affd, (long)dp, 0) < 0)
            xerror("seek %ld failed, readrec\n", dp) ;
      len = BSIZE - ((int)dp & (BSIZE - 1)) ;
      if (len <= 1 + A_WRTLEN + MAXNG * sizeof(struct artgroup))
#if BSIZE == 512
            len += BSIZE ;
#else
            len = 1 + sizeof(struct artrec) ;
#endif
      if ((len = read(affd, bfr, len)) < 1 + A_WRTLEN + MAXNG * sizeof(struct artgroup)) {
            printf("read returned %d\n", len) ;
            xerror("Bad read in readrec, %ld", dp) ;
      }
      if (bfr[0] != (char)A_PREFIX) {
            printf("len=%d, dp=%ld\n", len, (long)dp);
            fprintf(stderr, "bfr: %o %o %o %o %o\n", bfr[0], bfr[1],
bfr[2], bfr[3], bfr[4]);
            fprintf(stderr, "A_PREFIX = %o, (char)A_PREFIX = %o\n", A_PREFIX, (char)A_PREFIX);
            xerror("bad article pointer %ld", dp) ;
      }
      bcopy(bfr + 1, (char *)a, A_WRTLEN) ;
      bcopy(bfr + 1 + A_WRTLEN, (char *)a->a_group, i = a->a_ngroups * sizeof(*a->a_group)) ;
      pp = &a->a_ident ;
      p = bfr + 1 + A_WRTLEN + i ;
      q = a->a_space ;
      len -= 1 + A_WRTLEN + i ;
      i = (a->a_flags & A_DUMMY)? 1 : 4 + a->a_nkwords ;
      while (--i >= 0) {
            *pp++ = q ;
            do if (--len < 0) {
                  read(affd, bfr, 512) ;
                  len = 512 ;
                  p = bfr ;
               }
            while (*q++ = *p++) ;
      }
}



/*
 * Read a pointer from the article file
 */

DPTR
readptr(dp)
      DPTR dp ;
      {
      DPTR new ;

      if (lseek(affd, (long)dp, 0) < 0)
            xerror("seek %ld failed, readptr\n", dp) ;
      if (read(affd, (char *)&new, sizeof(new)) != sizeof(new))
            xerror("read failed, readptr(%ld)\n", dp) ;
      return new ;
}
