#include "common.h"


char *directory ;
char *file ;


main(argc, argv)
      char **argv ;
      {
      long t ;
      char *from, *lastc ;
      char to[PATHLEN] ;
      char **ap ;
      char prefix ;
      int fd ;

      prefix = 'n' ;
      ap = argv + 1 ;
      if (ap[0][0] == '-' && ap[0][1] == 't') {
            if ((prefix = ap[0][2]) == '\0')
usage:            fatal("usage: qnews [ -tc ] directory [ file ]") ;
            ap++ ;
      }
      if ((directory = *ap++) == NULL)
            goto usage ;
      from = *ap ;
      if (from != NULL && strcmp(from, "%s") == 0)
            from = NULL ;
      time(&t) ;
      sprintf(to, "%s/%c%lda", directory, prefix, t) ;
      lastc = to + strlen(to) - 1 ;
      signal(SIGTERM, SIG_IGN) ;
      for (;;) {
            if (from != NULL)
                  fd = link(from, to) ;
            else {
#ifdef USG
                  fd = open(to, O_WRONLY | O_CREAT | O_EXCL, 0444) ;
#else
                  fd = creat(to, 0444) ;
#endif
            }
            if (fd >= 0)
                  break ;
            if (errno != EEXIST && errno != EPERM || *lastc == 'z')
                  fatal("can't create %s", to) ;
            *lastc += 1 ;
      }
      if (from == NULL) {
            char buf[BUFSIZ] ;
            int count ;

            file = to ;
            while ((count = read(0, buf, BUFSIZ)) > 0) {
                  if (write(fd, buf, count) != count) {
                        fatal("write error") ;
                  }
            }
            if (count < 0) {
                  fatal("read error") ;
            }
      }
      exit(0) ;
}



/*
 * Fatal error.
 * Print error message and send mail to administrator.
 */

fatal(fmt, a1, a2, a3, a4)
      char *fmt ;
      {
      static int reentered = 0 ;

      if (reentered) {
            fprintf(stderr, "fatal entered recursively\n") ;
            fprintf(stderr, fmt, a1, a2, a3, a4) ;
            if (file != NULL)
                  unlink(file) ;
            exit(3) ;
      }
      reentered = 1 ;
      msg(fmt, a1, a2, a3, a4) ;
      if (file != NULL)
            if (unlink(file) < 0)
                  msg("unlink failed: %s", file) ;
      exit(2) ;
}


/*
 * Send mail to administrator.  Flag is set to indicate fatal error.
 */

msg(fmt, a1, a2, a3, a4)
      char *fmt ;
      {
      FILE *fp ;
      int e = errno ;

      fprintf(stderr, fmt, a1, a2, a3, a4) ;
      putc('\n', stderr) ;
      if ((fp = popen(MAILCMD, "w")) == NULL)
            fatal("popen failed") ;
      fputs("Subject:  error in qnews\n\n", fp) ;
      fprintf(fp, fmt, a1, a2, a3, a4) ;
      if (directory != NULL) {
            fputs("\nprocessing ", fp) ;
            fputs(directory, fp) ;
      }
      putc('\n', fp) ;
      fprintf(fp, "errno = %d\n", e) ;
      if (pclose(fp) != 0)
            fatal("msg failed") ;
}
