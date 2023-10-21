/*
 * This routine generates a list of articles for postnews.
 */

#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#include "artfile.h"
#include "ng.h"



artlist(ngname, pattern)
      char *ngname, *pattern ;
      {
      static struct ngrec ng ;
      struct artrec a ;
      ARTNO artno ;
      DPTR dp ;
      static int first = 1 ;

      if (first || strcmp(ngname, ng.g_name) != 0) {
            gfopen() ;
            ALL_GROUPS(ng) {
                  if (strcmp(ng.g_name, ngname) == 0)
                        goto found ;
            }
            xerror("%s does not appear in groupfile", ngname) ;
found:
            gfclose() ;
      }
      if (first) {
            afopen() ;
            first = 0 ;
      }
      fputs("NUMB  AUTHOR		TITLE\n", stdout) ;
      BKWD_GROUP(ng.g_num, artno, dp, a) {
            if (a.a_flags & A_DUMMY)
                  continue ;
            if (sindex(a.a_title, pattern) || sindex(a.a_from, pattern)) {
                  printf("%-5d %.16s\t", artno, a.a_from) ;
                  if (strlen(a.a_from) < 10)
                        putchar('\t') ;
                  if (strlen(a.a_from) < 2)
                        putchar('\t') ;
                  printf("%.40s\n", a.a_title) ;
            }
      }
}
