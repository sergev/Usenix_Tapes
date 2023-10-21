#include <stdio.h>
#include "af.h"
#include "ng.h"
#include <setjmp.h>

struct jump {
      jmp_buf jbuf ;
      struct jump *nextj ;
} *jmp ;



main(argc, argv)
      char **argv ;
      {
      char **ap = argv + 1 ;
      char *gname ;
      int gnum ;
      struct artrec a ;
      DPTR dp ;
      char artfile[64] ;
      struct jump j ;
      extern int LIB[] ;
      long atol() ;

      jmp = &j ;
      if (setjmp(j.jbuf)) {
            exit(2) ;
      }
      if (argc < 2)
            xerror("Usage: dumpng -fartfile -n newsgroup...") ;
      pathinit() ;
      sprintf(artfile, "%s/artfile", LIB) ;
      if (strncmp(*ap, "-f", 2) == 0) {
            if (ap[0][2])     strcpy(artfile, *ap + 2) ;
            else              strcpy(artfile, *++ap) ;
            ap++ ;
      }
      genafopen(artfile, "r") ;
      gfopen() ;

      while ((gname = *ap++) != NULL) {
            if ((gnum = prtngheader(gname)) >= 0) {
                  prtngchain(gname, gnum) ;
            }
      }
      exit(0) ;
}



prtngheader(gname)
      char *gname ;
      {
      struct ngrec g ;

      ALL_GROUPS(g) {
            if (strcmp(g.g_name, gname) == 0)
                  goto found ;
      }
      printf("\nUnknown newsgroup %s\n", gname) ;
      return -1 ;

found:
      printf("\nNewsgroup %s (%d)", g.g_name, g.g_num) ;
      if (g.g_flags & G_MOD)
            printf("  moderated") ;
      printf("\n\n") ;
      return g.g_num ;
}



prtngchain(gname, gnum)
      char *gname ;
      {
      DPTR dp ;
      struct artrec a ;
      ARTNO an ;
      int i ;

      BKWD_GROUP(gnum, an, dp, a) {
            for (i = 0 ; ; i++) {
                  if (i >= a.a_ngroups) {
                        printf("???  ") ;
                        break ;
                  } else if (a.a_group[i].a_ngnum == gnum) {
                        printf("%-5d", a.a_group[i].a_artno) ;
                        break ;
                  }
            }
            printf("%-22.22s %-30.30s %.20s\n",
                   a.a_ident, a.a_title, a.a_from) ;
      }
}




xerror(msg, a1, a2, a3, a4)
      char *msg ;
      {
      putchar('\n') ;
      printf(msg, a1, a2, a3, a4) ;
      putchar('\n') ;
      longjmp(jmp->jbuf, 1) ;
}
