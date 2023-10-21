/*
 * This program inserts entries into the artfile.
 */

#include <stdio.h>
#include "af.h"
#include <sys/types.h>
#include "defs.h"

int newgroup ;
char progname[] = "afinsert" ;
extern char LIB[] ;
extern char SPOOL[] ;


main(argc, argv)
      char **argv ;
      {
      int c ;
      char afname[64] ;

      setbuf(stdout, NULL) ;
#if BSDREL >= 41
      setbuf(stderr, NULL) ;
#endif
      if (argc != 2)
            xerror("Usage: afinsert hist-line") ;
      sprintf(afname, "%s/artfile", LIB) ;
      genafopen(afname, "r+") ;
      getngtab() ;
      if (chdir(SPOOL) < 0)
            xerror("Can't chdir to spool") ;
      doadd(argv[1], 1) ;
      if (newgroup)
            writengfile() ;
      exit(0) ;
}



xerror(msg, a1, a2, a3, a4)
      char *msg ;
      {
      printf(msg, a1, a2, a3, a4) ;
      putchar('\n') ;
      abort() ;
}
