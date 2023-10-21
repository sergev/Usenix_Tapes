#include <stdio.h>
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "newsdefs.h"
#include "af.h"


char *index() ;
FILE *ckfopen() ;

static int idsize = 10007 ;	/* default size of message ID hash table */
static int grpsize =  512 ;	/* default maximum newsgroup number */
static int ttlsize = 5003 ;	/* default size of title hash table */

char progname[] = "afbuild" ;
int newgroup ;


main(argc, argv)
      char **argv ;
      {
      int c ;
      char curname[64], newname[64] ;
      char buf[256] ;
      char histname[64] ;
      FILE *histfp ;
      extern char LIB[], SPOOL[] ;
      extern char *optarg ;

      setbuf(stdout, NULL) ;
#if BSDREL >= 41
      setbuf(stderr, NULL) ;
#endif
      pathinit() ;
      sprintf(histname, "%s/history", LIB) ;
      while ((c = getopt(argc, argv, "h:m:g:t:")) != EOF) switch (c) {
            case 'h':
                  scopyn(optarg, histname, 64) ;
                  break ;
            case 'm':
                  idsize = numarg(c) ;
                  break ;
            case 'g':
                  grpsize = numarg(c) ;
                  break ;
            case 't':
                  ttlsize = numarg(c) ;
                  break ;
      }
      sprintf(curname, "%s/artfile", LIB) ;
      sprintf(newname, "%s.tmp", curname) ;
      unlink(newname) ;
      makeaf(newname, idsize, grpsize, ttlsize) ;
      if (strcmp(histname, "none") != 0) {
            readngtab() ;
            genafopen(newname, "r+") ;
            if ((histfp = fopen(histname, "r")) == NULL)
                  xerror("Can't open history file") ;
            if (chdir(SPOOL) < 0)
                  xerror("Can't chdir to %s", SPOOL) ;
            while (fgets(buf, 256, histfp) != NULL)
                  doadd(buf, 0) ;
            if (newgroup)
                  writengfile() ;
      }
      aflock() ;
      sleep(10) ;		/* let the dust settle */
      while (fgets(buf, 256, histfp) != NULL)
            doadd(buf, 0) ;
      close(affd) ;
      if (rename(newname, curname) < 0)
            xerror("rename failed") ;
      afunlock() ;
      sync() ;
      exit(0) ;
}



numarg(c) {
      extern char *optarg ;
      register char *p ;

      for (p = optarg ; *p ; p++)
            if (*p < '0' || *p > '9')
                  xerror("Illegal number after -%c option", c) ;
      return atoi(optarg) ;
}



readngtab() {
      char file[FPATHLEN] ;
      FILE *fp ;
      struct stat statb ;
      char line[512] ;
      register char *p ;
      extern char LIB[] ;

      sprintf(file, "%s/groupfile", LIB) ;
      if (stat(file, &statb) >= 0 && statb.st_size > 0) {
            getngtab() ;
      } else {
            fprintf(stderr, "afbuild: reconstructing groupfile.\n") ;
            ngtinit() ;
            sprintf(file, "%s/active", LIB) ;
            fp = ckfopen(file, "r") ;
            while (fgets(line, 512, fp) != NULL) {
                  if (line[0] == '\n')
                        continue ;
                  if ((p = index(line, ' ')) != NULL)
                        *p = '\0' ;
                  if ((p = index(line, '\n')) != NULL)
                        *p = '\0' ;
                  newng(line) ;
                  newgroup++ ;
            }
      }
}



xerror(msg, a1, a2, a3, a4)
      char *msg ;
      {
      extern int errno ;
      int e = errno ;

      fputs("afbuild: ", stderr) ;
      fprintf(stderr, msg, a1, a2, a3, a4) ;
      putc('\n', stderr) ;
      fprintf(stderr, "errno=%d\n", e) ;
      fflush(stderr) ;		/* 4.2 BSD buffers stderr! */
      exit(2) ;
}
