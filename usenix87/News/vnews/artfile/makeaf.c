#include <stdio.h>
#include "af.h"



/*
 * Create a new artfile.
 */

makeaf(name, idtlen, maxng, ttlen)
      char *name ;
      {
      register FILE *fp ;
      DPTR offset, count ;
      struct afheader afhd ;

      if ((fp = fopen(name, "w")) == NULL)
            xerror("Can't create %s", name) ;
      offset = count = sizeof afhd ;
      afhd.af_magic = AF_MAGIC ;
      afhd.af_version = AF_VERSION ;
      afhd.af_idtab = offset ;
      afhd.af_idtlen = idtlen ;
      offset += idtlen * sizeof (DPTR) ;
      afhd.af_nglist = offset ;
      afhd.af_maxng = maxng ;
      offset += maxng * sizeof (DPTR) ;
      afhd.af_titletab = offset ;
      afhd.af_ttlen = ttlen ;
      offset += ttlen * sizeof (DPTR) ;
      afhd.af_records = afhd.af_free = offset ;
      fwrite((char *)&afhd, sizeof afhd, 1, fp) ;
      while (count < offset) {
            putc('\0', fp) ;
            count++ ;
      }
      if (ferror(fp) || fclose(fp) == EOF)
            xerror("Write error in makeaf") ;
}
