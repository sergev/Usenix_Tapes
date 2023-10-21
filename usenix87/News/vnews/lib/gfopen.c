#include <stdio.h>
#include "defs.h"
#include "libextern.h"

FILE *ngfp ;
int maxng ;


gfopen() {
      char buf[FPATHLEN] ;

      sprintf(buf, "%s/groupfile", LIB);
      if ((ngfp = fopen(buf, "r")) == NULL
       && (sleep(2), ngfp = fopen(buf, "r")) == NULL)
            xerror("can't open newsgroup file") ;
      if (fgets(buf, sizeof buf, ngfp) == NULL)
            xerror("newsgroup file is empty") ;
      maxng = atoi(buf) ;
}


gfclose() {
      fclose(ngfp) ;
      ngfp = NULL ;
}
