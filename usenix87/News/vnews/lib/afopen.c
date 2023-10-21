#include "defs.h"
#include "libextern.h"


afopen() {
      char fname[FPATHLEN] ;

      sprintf(fname, "%s/artfile", LIB) ;
      genafopen(fname, "r") ;
}
