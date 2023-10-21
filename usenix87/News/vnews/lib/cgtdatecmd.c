/*
 * This program takes a date as its argument and writes the UN*X internal
 * value on the standard output.  The primary purpose of this program is
 * to make it unnecessary to load the cgtdate routine as part of readnews
 * and/or vnews.
 */

#include <stdio.h>

main(argc, argv)
      char **argv ;
      {
      char buf[1024] ;
      register int i ;
      long tim ;
      long cgtdate() ;

      buf[0] = '\0' ;
      for (i = 1 ; i < argc ; i++) {
            strcat(buf, " ") ;
            strcat(buf, argv[i]) ;
      }
      tim = cgtdate(buf) ;
      if (tim == -1L)
            return 1 ;
      printf("%ld\n", tim) ;
      return 0 ;
}
