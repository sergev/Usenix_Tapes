#include "common.h"

FILE *batfp ;
int nfiles ;


addfile(type, fp)
      FILE *fp ;
      {
      register int c ;

      if (putc(type, batfp) == EOF)
bad:        fatal("batch file write error") ;
      while ((c = getc(fp)) != EOF)
            if (putc(c, batfp) == EOF)
                  goto bad ;
      if (putc(ENDMARK, batfp) == EOF)
            goto bad ;
      nfiles++ ;
}



raddfile(type, fp)
      register FILE *fp ;
      {
      register int c ;

      if (putc(type, batfp) == EOF)
bad:        fatal("batch file write error") ;
      while ((c = getc(fp)) != EOF)
            if (putc(c, batfp) == EOF)
                  goto bad ;
      if (putc(ENDMARK, batfp) == EOF)
            goto bad ;
      nfiles++ ;
}
