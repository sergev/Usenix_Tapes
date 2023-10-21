/*
 * This is a sample unbatcher.  It is called with the batch file on stdin
 * and creates the unbatched files in the current directory.  Exit status:
 * 2  => wrong batcher invoked.
 * 33 => batch file was truncated.
 * 34 => write error occurred.
 * 1  => illegal batch file.
 */

#include "common.h"

char unbattmp[] = "../unbatch.tmp" ;


main(argc, argv)
      char **argv ;
      {
      register c ;
      FILE *ufp ;
      int seqno ;
      char hdline[64] ;
      char fname[FNLEN] ;

      seqno = getpid() % 9000 ;
      if (fgets(hdline, 64, stdin) == NULL) {
            printf("empty batch file\n") ;
            exit(33) ;
      }
      if (hdline[0] != 'a') {
            printf("bad version\n") ;
            exit(2) ;
      }
      if (atol(hdline + 1) != fsize(fileno(stdin))) {
            printf("bad size\n") ;
            exit(33) ;
      }
      if (index(hdline, '\n') == NULL) {
            printf("header too long\n") ;
            exit(1) ;
      }
      while ((c = getc(stdin)) != EOF) {
            sprintf(fname, "%c%04d", c, seqno++) ;
            if (badname(fname) || fname[0] == 'b') {
                  printf("bad file type\n") ;
                  exit(1) ;
            }
            if ((ufp = fopen(unbattmp, "w")) == NULL) {
                  printf("can't create unbatch.tmp") ;
                  exit(34) ;
            }
            while ((c = getc(stdin)) != ENDMARK) {
                  if (c == EOF) {
                        printf("unexpected EOF\n") ;
                        exit(1) ;
                  }
                  if (putc(c, ufp) == EOF) {
                        printf("write error on unbatch.tmp\n") ;
                        exit(34) ;
                  }
            }
            if (fclose(ufp) == EOF) {
                  printf("close error on unbatch.tmp\n") ;
                  exit(34) ;
            }
            if (link(unbattmp, fname) < 0) {
                  printf("link to %s failed\n", fname) ;
                  exit(34) ;
            }
            if (unlink(unbattmp) < 0) {
                  unlink(fname) ;
                  printf("can't unlink unbatch.tmp\n") ;
                  exit(34) ;
            }
      }
      exit(0) ;
}


badname(fname)
      char *fname ;
      {
      register c ;

      if ((c = *fname++) != 'a' && c != 'n' && c != 'm' && c != 'b')
            return -1 ;
      if ((c = *fname++) < '0' || c > '9')
            return -1 ;
      return 0 ;
}


fsize(fd) {
      struct stat st ;

      if (fstat(fd, &st) < 0)
            return 0L ;
      return st.st_size ;
}
