/*
 * Make a copy of a string.  The copy cannot be freed.
 */

#include "str.h"

#define BLOCKSIZE 1024

static char *savearea;
static char *savep;

char *
nsavestr(s)
      char *s ;
      {
      int size = strlen(s) + 1;
      char *p ;
      char *ckmalloc() ;

      if (size > BLOCKSIZE)
            return savestr(s) ;
      if (savearea == 0 || savearea + BLOCKSIZE - savep < size) {
            nsaveclean() ;
            savearea = savep = ckmalloc(BLOCKSIZE) ;
      }
      scopy(s, p = savep) ;
      savep += size ;
      return p ;
}


/*
 * Release unused storage.
 */

nsaveclean() {
      char *realloc() ;
      if (savearea) {
            if (realloc(savearea, savep - savearea) != savearea)
                  xerror("realloc blew it") ;
      }
      savearea = 0 ;
}
