#include <stdio.h>
#include "af.h"
#include <setjmp.h>

struct jump {
      jmp_buf jbuf ;
      struct jump *nextj ;
} *jmp ;



main(argc, argv)
      char **argv ;
      {
      char **ap = argv + 1 ;
      char *msgid ;
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
            xerror("Usage: dumprec -fartfile message-id...") ;
      pathinit() ;
      sprintf(artfile, "%s/artfile", LIB) ;
      if (strncmp(*ap, "-f", 2) == 0) {
            if (ap[0][2])     strcpy(artfile, *ap + 2) ;
            else              strcpy(artfile, *++ap) ;
            ap++ ;
      }
      genafopen(artfile, "r") ;
      setjmp(j.jbuf) ;
      while ((msgid = *ap++) != NULL) {
            if (msgid[0] >= '0' && msgid[0] <= '9')
                  readrec(dp = atol(msgid), &a) ;
            else if ((dp = lookart(msgid, &a)) == DNULL) {
                  printf("\nNo entry for %s\n", msgid) ;
                  continue ;
            }
            prtentry(&a, dp, msgid) ;
      }
      exit(0) ;
}



prtentry(a, dp, msgid)
      register struct artrec *a ;
      DPTR dp ;
      char *msgid ;
      {
      char *ctime() ;

      printf("\nArticle: %ld %s\n", dp, a->a_ident) ;
      if ((a->a_flags & A_DUMMY) == 0) {
            printf("Subject: %s\n", a->a_title) ;
            printf("From: %s\n", a->a_from) ;
            printf("File: %s\n", a->a_file) ;
      }
      printf("Posted: %s", ctime(&a->a_subtime)) ;
      printf("Received: %s", ctime(&a->a_rectime)) ;
      if (a->a_exptime)
            printf("Expires: %s", ctime(&a->a_exptime)) ;
      printf("Lines: %d\n", a->a_nlines) ;
      printf("Flags: %05o\n", a->a_flags) ;
      prtgroups(a) ;
      prtptr("Parent: ", a->a_parent) ;
      prtchildren(a->a_children) ;
      prtptr("Childchain: ", a->a_childchain) ;
      prtptr("Idchain: ", a->a_idchain) ;
      prtptr("Titlechain: ", a->a_titlechain) ;
}


prtchildren(dp)
      DPTR dp ;
      {
      struct artrec a ;
      char *msg ;

      msg = "Children: ";
      while (dp != DNULL) {
            prtptr(msg, dp) ;
            readrec(dp, &a) ;
            dp = a.a_childchain ;
            msg = "\t  " ;
      }
}


prtgroups(a)
      struct artrec *a ;
      {
      register int i ;
      char *msg = "Newsgroups: ";

      for (i = 0 ; i < a->a_ngroups ; i++) {
            printf("%s%d/%d", msg, a->a_group[i].a_ngnum, a->a_group[i].a_artno);
            prtptr(" ", a->a_group[i].a_ngchain) ;
            msg = "\t    ";
      }
}


prtptr(msg, dp)
      char *msg ;
      DPTR dp ;
      {
      struct artrec a ;

      printf("%s", msg) ;
      if (dp == DNULL)
            printf("null") ;
      else {
            printf("%ld ", dp) ;
            readrec(dp, &a) ;
            printf("%s", a.a_ident) ;
      }
      putchar('\n') ;
}



xerror(msg, a1, a2, a3, a4)
      char *msg ;
      {
      putchar('\n') ;
      printf(msg, a1, a2, a3, a4) ;
      putchar('\n') ;
      longjmp(jmp->jbuf, 1) ;
}
