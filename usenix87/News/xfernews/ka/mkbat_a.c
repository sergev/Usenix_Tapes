/*
 * This is a sample batch generator.  The three routines are:
 *	mkbatch - set up a batch
 *	addfile - add a file to the current batch
 *	closebatch - finish processing the current batch
 */

#include "common.h"

extern FILE *batfp ;
extern char batchtmp[] ;



mkbatch() {
      batfp = ckopen(batchtmp, "w") ;
      fputs("a000000\n", batfp) ;
}


closebatch() {
      long len ;

      if ((len = ftell(batfp)) <= 0L)
            fatal("bad ftell") ;
      if (fseek(batfp, 0L, 0))
            fatal("batch file seek error") ;
      fprintf(batfp, "a%06ld", len) ;
      if (fclose(batfp) == EOF)
            fatal("batch file close error") ;
      batfp = NULL ;
}


addfile(type, fp)
      register FILE *fp ;
      {
      register int c ;

      if (putc(type, batfp) == EOF)
bad:        fatal("batch file write error") ;
      while ((c = getc(fp)) != EOF)
            if (c != (ENDMARK & 0377) && putc(c, batfp) == EOF)
                  goto bad ;
      if (putc(ENDMARK, batfp) == EOF)
            goto bad ;
}
