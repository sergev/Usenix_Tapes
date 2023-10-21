#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#include "defs.h"
#include "arthead.h"


char *index() ;


doadd(hist, lockit)
      char *hist ;
      {
      FILE *artfp ;
      char fname[64] ;
      struct arthead h ;
      register char *p ;
      extern char progname[] ;

      bzero((char *)&h, sizeof h) ;
      if ((p = index(hist, '\t')) == NULL) {
bad:        printf("Bad history line") ;
            return ;
      }
      p++ ;
      if ((p = index(p, '\t')) == NULL)
            goto bad ;
      p++ ;
      nstrip(p) ;
      hist = p ;
      scopyn(p, fname, 64) ;
      if ((p = index(fname, ' ')) != NULL)
            *p = '\0' ;
      for (p = fname ; *p ; p++)
            if (*p == '.')
                  *p = '/' ;
      if ((artfp = fopen(fname, "r")) == NULL) {
            printf("%s: can't open %s\n", progname, fname) ;
            return ;
      }
      if (gethead(&h, artfp) == NULL) {
            printf("%s: bad header\n", fname) ;
            return ;
      }
      h.h_intnumlines = countlines(artfp) ;
      if (lockit)  aflock() ;
      addart(&h, fname, hist) ;
      if (lockit)  afunlock() ;
      fclose(artfp) ;
      hfree(&h) ;
}



countlines(fp)
      register FILE *fp ;
      {
      register c ;
      int curline, nlines ;

      curline = 1 ;
      nlines = 0 ;
      while ((c = getc(fp)) != EOF) {
            if (c == '\n')
                  curline++ ;
            else
                  nlines = curline ;
      }
      return nlines ;
}
