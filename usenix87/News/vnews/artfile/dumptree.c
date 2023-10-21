#include <stdio.h>
#include "artfile.h"
#include "libextern.h"

char *savestr();

char *seen[2000];
char **seenp = seen;



main(argc, argv)
      char **argv;
      {
      char artfile[64] ;
      struct artrec a;
      DPTR dp;
      int i;
      int rootflg = 0;
      struct article *ap;
      char *msgid;
      struct article *rootid;
      char *title;
      extern int optind;
      extern char *optarg;

      pathinit();
      sprintf(artfile, "%s/artfile", LIB) ;
      while ((i = getopt(argc, argv, "f:r")) != EOF) {
            switch (i) {
            case 'f':
                  strcpy(artfile, optarg);
                  break;
            case 'r':
                  rootflg++;
                  break;
            }
      }
      if (optind >= argc)
            xerror("Usage: prttree message-id\n");
      msgid = argv[optind];
      genafopen(artfile, "r") ;
      if ((dp = lookart(msgid, &a)) == DNULL)
            xerror("No entry for %s", msgid);
      rootid = msgid;
      if ((a.a_flags & A_DUMMY) == 0)
            title = savestr(a.a_title);
      else
            title = "";
      if (rootflg) {
            seenp = seen;
            while (dp != DNULL) {
                  readrec(dp, &a);
                  rootid = savestr(a.a_ident);
                  if (lookid(rootid)) {
                        printf("loop discovered while searching for parent:\n");
                        printf("	%ld %s\n", dp, rootid);
                        do {
                              readrec(dp = a.a_parent, &a);
                              printf("	%ld %s\n", dp, a.a_ident);
                        } while (strcmp(rootid, a.a_ident) != 0);
                        break;
                  }
                  *seenp++ = rootid;
                  dp = a.a_parent;
            }
      }
      if ((dp = lookart(rootid, &a)) == DNULL)
            xerror("rootid");
      if ((a.a_flags & A_DUMMY) == 0)
            title = a.a_title;
      seenp = seen;
      printf("Discussion tree for %s\n", title);
      prttree(dp, &a, 0, DNULL);
      exit(0);
}


prttree(dp, a, indent, parent)
      DPTR dp;
      struct artrec *a;
      int indent;
      DPTR parent;
      {
      int i;
      char *p;
      DPTR dp2;
      struct artrec a2;
      struct article *ap;

      i = indent;
      while (i >= 8) {
            putchar('\t');
            i -= 8;
      }
      while (i >= 1) {
            putchar(' ');
            i--;
      }
      indent += 3;
      printf("%ld %s", dp, a->a_ident);
      p = savestr(a->a_ident);
      if (a->a_parent != parent) {
            printf(" (parent=%ld!)", a->a_parent);
      }

      if (lookid(p)) {
            printf(" (duplicate)\n");
            return;
      }
      *seenp++ = p;
      putchar('\n');
      for (dp2 = a->a_children ; dp2 != DNULL ; dp2 = a2.a_childchain) {
            readrec(dp2, &a2);
            prttree(dp2, &a2, indent, dp);
      }
}



xerror(fmt, a1, a2, a3, a4)
      char *fmt;
      {
      fprintf(stderr, fmt, a1, a2, a3, a4);
      putc('\n', stderr);
      exit(1);
}



lookid(id)
      register char *id;
      {
      register char **p;

      for (p = seen ; p < seenp ; p++)
            if (**p == *id && strcmp(*p, id) == 0)
                  return 1;
      return 0;
}
