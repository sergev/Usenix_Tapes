#define DEBUG
#include "common.h"

#define ASKIP 4

struct arglist {
      int nargs ;
      char **first ;
      char *arg[MAXARGS] ;
} ;

struct arglist uuargs ;
char *directory ;
int errflag ;
char resentflag[] = "resentflag" ;
char lastack[] = "lastack" ;
char ackfile[] = "ackfile" ;



main(argc, argv)
      char **argv ;
      {
      register char **ap ;
      int rflag ;
      char *p ;
      int i ;
      struct stat statb ;

      setuid(NETNEWS) ;
      ap = argv + 1 ;
      rflag = 0 ;
      while ((p = *ap++) != NULL && *p == '-') {
            if (strcmp(p, "-r") == 0)
                  rflag++ ;
            else
usage:            fatal("usage: sendnews [ -r ] from to") ;
      }
      if (p == NULL || *ap == NULL)
            goto usage ;
      directory = p ;
      if (chdir(p) < 0)
            fatal("no directory") ;
      uuargs.first = &uuargs.arg[ASKIP] ;
      uuargs.nargs = ASKIP ;
      if (unlink(resentflag) < 0) {
            sendnews(1) ;
      }
      if (uuargs.nargs > ASKIP) {
            if ((i = creat(resentflag, 0666)) < 0)
                  msg("can't create resent flag") ;
            close(i) ;
            msg("resent %d files", uuargs.nargs - ASKIP) ;
      }
      sendnews(0) ;
      if (stat(ackfile, &statb) >= 0
       && (uuargs.nargs > ASKIP || statb.st_size >= MINACK)) {
            long t ;
            char buf[FNLEN + 5] ;

            time(&t) ;
            sprintf(buf, "sent/a%lda", t) ;
            if (link(ackfile, buf) < 0) {
                  msg("can't link ackfile") ;
                  goto uu ;
            }
            if (unlink(ackfile) < 0) {
                  msg("can't unlink ackfile") ;
                  goto uu ;
            }
            insarg(buf + 5, &uuargs) ;
      }
uu:
      uucp(*ap, rflag) ;
      exit(errflag) ;
}



sendnews(resend) {
      char *dir ;
      char sentname[PATHLEN] ;
      DIR *dp ;
      FILE *fp ;
      struct direct *d ;
      long last ;
      int oflow ;

      if (resend == 0) {
            dir = "out" ;
      } else {
            dir = "sent" ;
            if ((fp = fopen(lastack, "r")) == NULL) {
                  msg("can't open lastack") ;
                  return ;
            }
            if (fgets(sentname, FNLEN, fp) == NULL) {
                  /* Can occur bacause no locking done */
                  msg("lastack is empty file") ;
                  fclose(fp) ;
                  return ;
            }
            fclose(fp) ;
            last = atol(sentname) - 3600L ;
      }
      if (chdir(dir) < 0)
            fatal("chdir %s failed", dir) ;
      if ((dp = opendir(".")) == NULL)
            fatal("no .") ;
      oflow = 0 ;
      while ((d = readdir(dp)) != NULL) {
            if (d->d_name[0] == '.')
                  continue ;
            else if (badname(d->d_name)) {
                  msg("bad file %s in %s", d->d_name, dir) ;
                  movebad(d->d_name) ;
                  continue ;
            }
            if (resend) {
                  if (atol(d->d_name + 1) > last)
                        continue ;
                  printf("resending %s\n", d->d_name) ;
            }
            if (uuargs.nargs >= MAXARGS - 3) {
                  oflow++ ;
                  continue ;
            }
            addarg(d->d_name, &uuargs) ;
            if (! resend) {
                  sprintf(sentname, "../sent/%s", d->d_name) ;
                  if (link(d->d_name, sentname) < 0)
                        msg("link %s failed", d->d_name) ;
                  else if (unlink(d->d_name) < 0)
                        msg("unlink %s failed", d->d_name) ;
            }
      }
      closedir(dp) ;
      if (oflow > 0)
            msg("too many files: %d not sent", oflow) ;
      if (chdir("..") < 0)
            fatal("no ..") ;
}



uucp(to, rflag)
      char *to ;
      {
      if (uuargs.first == uuargs.arg + uuargs.nargs)
            return ;
      if (chdir("sent") < 0)
            fatal("no sent dir") ;
      qsort((char *)(uuargs.arg + ASKIP), uuargs.nargs - ASKIP, sizeof(char *), comp) ;
      addarg(to, &uuargs) ;
      addarg(NULL, &uuargs) ;
      if (rflag)  insarg("-r", &uuargs) ;
      insarg("-c", &uuargs) ;		/* still not default on some uucps */
      insarg(UUCP, &uuargs) ;
      if (run(uuargs.first, 0, 0) != 0)
            fatal("uucp failed") ;
      if (chdir("..") < 0)
            fatal("no ..") ;
}



badname(fname)
      char *fname ;
      {
      register c ;

      if ((c = *fname++) != 'a' && c != 'n' && c != 'm')
            return -1 ;
      if ((c = *fname++) < '0' || c > '9')
            return -1 ;
      return 0 ;
}



movebad(fname)
      char *fname ;
      {
      char bad[PATHLEN] ;

      sprintf(bad, "../bad/%s", fname) ;
      unlink(bad) ;
      if (link(fname, bad) < 0)
            fatal("link to bad failed for %s", fname) ;
      if (unlink(fname) < 0)
            fatal("unlink bad file %s failed", fname) ;
}



comp(a, b)
      char **a, **b ;
      {
      return strcmp(*a, *b) ;
}



addarg(fname, argl)
      struct arglist *argl ;
      char *fname ;
      {
      char *p ;

      if (argl->nargs >= MAXARGS)
            fatal("too many articles") ;
      if (fname == NULL)
            p = NULL ;
      else {
            if ((p = malloc(strlen(fname) + 1)) == NULL)
                  fatal("out of space") ;
            strcpy(p, fname) ;
      }
      argl->arg[argl->nargs++] = p ;
}


insarg(fname, argl)
      struct arglist *argl ;
      char *fname ;
      {
      char *p ;

      if (argl->first <= argl->arg)
            fatal("insarg failed") ;
      if (fname == NULL)
            p = NULL ;
      else {
            if ((p = malloc(strlen(fname) + 1)) == NULL)
                  fatal("out of space") ;
            strcpy(p, fname) ;
      }
      *--(argl->first) = p ;
}



run(args, flags, fd)
      char *args[] ;
      int flags ;
      int fd ;
      {
      int pid ;
      int status ;
      int i ;

#ifdef DEBUG
      printf("run") ;				/*DEBUG*/
      for (i = 0 ; args[i] != NULL ; i++)	/*DEBUG*/
            printf(" %s", args[i]) ;		/*DEBUG*/
      putchar('\n') ;				/*DEBUG*/
#endif
      if ((pid = fork()) == -1)
            fatal("Cannot fork") ;
      if (pid == 0) {
            if (flags & SETIN) {
                  close(0) ;
                  if (dup(fd) != 0) {
                        msg("Cannot redirect input") ;
                        exit(127) ;
                  }
                  close(fd) ;
            }
            execv(args[0], args) ;
            msg("exec failed") ;
            exit(127) ;
      }
      while ((i = wait(&status)) != pid && i != -1) ;
      return status ;
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
            exit(3) ;
      }
      reentered = 1 ;
      msg(fmt, a1, a2, a3, a4) ;
      exit(2) ;
}


/*
 * Send mail to administrator.
 */

msg(fmt, a1, a2, a3, a4)
      char *fmt ;
      {
      FILE *fp ;
      int fd ;

      errflag = 1 ;
      fprintf(stderr, fmt, a1, a2, a3, a4) ;
      putc('\n', stderr) ;

      /* be sure stdin is open; otherwise popen won't work */
      if ((fd = open("/dev/null", 0)) > 0)
            close(fd) ;
      if ((fp = popen(MAILCMD, "w")) == NULL)
            fatal("popen failed") ;
      fputs("Subject:  error in sendnews\n\n", fp) ;
      fprintf(fp, fmt, a1, a2, a3, a4) ;
      if (directory != NULL) {
            fputs("\nprocessing ", fp) ;
            fputs(directory, fp) ;
      }
      putc('\n', fp) ;
      if (pclose(fp) != 0)
            fatal("msg failed") ;
}
