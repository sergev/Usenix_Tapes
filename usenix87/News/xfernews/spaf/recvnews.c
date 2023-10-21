#define RECVNEWS 1
#include "common.h"

#ifdef USG
#include <setjmp.h>
#define setexit() setjmp(nextdir)
#define reset() longjmp(nextdir, 1)
jmp_buf nextdir ;	/* label to jump to on major error */
#endif

struct arglist {
      int nargs ;
      char *arg[MAXARGS] ;
} ;

char *directory ;	/* directory currently being processed */
int errflag ;		/* set if any errors */
char lockfile[] = RECVLOCK ;



main(argc, argv)
char **argv ;
{
      char **ap ;

      nice(10) ;
      setuid(NETNEWS) ;		/* in case invoked as root by cron */
      if (setlock(lockfile) == 0) {
            printf("recvnews locked\n") ;
            exit(0) ;
      }
      ap = argv + 1 ;
      setexit() ;
      while (*ap != NULL) {
            directory = *ap++ ;
            if (chdir(directory) < 0)
                  fatal("directory nonexistant") ;
            inputnews() ;
            if (chdir("..") < 0) {
                  printf("can't chdir to %s/..\n", directory) ;
                  break ;
	    }
      }
      if (unlink(lockfile) < 0)
            msg("can't unlink lock") ;
      exit(errflag) ;
}



inputnews() {
      struct arglist in ;
      DIR *dp ;
      struct direct *d ;
      int oflow ;
      int i ;

      in.nargs = 0 ;
      oflow = 0 ;
      if (chdir("in") < 0)
            fatal("in missing") ;
      dp = opendir(".") ;
      if (dp == NULL)
            fatal("no .") ;
      while ((d = readdir(dp)) != NULL) {
            if (d->d_name[0] != '.') {
                  if (in.nargs < MAXARGS)
                        addarg(d->d_name, &in) ;
                  else
                        oflow++ ;
            }
      }
      if (oflow > 0)
            msg("%d articles not processed", oflow) ;
      closedir(dp) ;
      if (in.nargs == 0)
            goto out ;
      qsort((char *)in.arg, in.nargs, sizeof(char *), comp) ;
      sleep(5) ;		/* in case any files half written */
      for (i = 0 ; i < in.nargs ; i++) {
            procfile(in.arg[i]) ;
            free(in.arg[i]) ;
      }
out:
      if (chdir("..") < 0)
            fatal("can't chdir ..") ;
}



procfile(name)
      char *name ;
      {
      FILE *fp ;
      int rc ;

      if (badname(name)) {
            msg("bad input file name %s", name) ;
            movebad(name) ;
            return ;
      }
      if ((fp = fopen(name, "r")) == NULL) {
            msg("unreadable file %s", name) ;
            movebad(name) ;
            return ;
      }
      switch (name[0]) {
      case 'a':
            rc = procack(name, fp) ;
            break ;
      case 'n':
            rc = procnews(name, fp) ;
            break ;
      case 'm':
            rc = procmail(name, fp) ;
            break ;
      default:
            fatal("can't happen %s", name) ;
            break ;
      }
      if (rc < 0)
            movebad(name) ;
      else if (unlink(name) < 0)
            msg("can't unlink %s", name) ;
      fclose(fp) ;
      if ((fp = fopen("../ackfile", "a")) == NULL)
            fatal("Can't open ackfile") ;
      fprintf(fp, "%s\n", name) ;
      fclose(fp) ;
}



procnews(name, fp)
      char *name ;
      FILE *fp ;
      {
      char *arg[2] ;

      arg[0] = RNEWS, arg[1] = NULL ;
      return chkrun(arg, name, fp) ;
}


procmail(name, fp)
      char *name ;
      FILE *fp ;
      {
      char *arg[4] ;
      char buf[DESTLEN] ;
      char *p ;

      setbuf(fp, NULL) ;		/* turn off buffering */
      if (fgets(buf, DESTLEN, fp) == NULL) {
            msg("%s: empty file", name) ;
            return -1 ;
      }
      if (strncmp(buf, "To ", 3) != 0) {
            msg("corrupted mail %s", name) ;
            return -1 ;
      }
      if ((p = strchr(buf, '\n')) == NULL) {
            msg("destination too long, file %s", name) ;
            return -1 ;
      }
      *p = '\0' ;
      arg[0] = RMAIL, arg[1] = buf + 3, arg[2] = NULL ;
      return chkrun(arg, name, fp) ;
}



procack(name, fp)
      char *name ;
      FILE *fp ;
      {
      char line[FNLEN+2] ;
      char *p ;

      if (chdir("../sent") < 0)
            fatal("no sent dir") ;
      while (fgets(line, FNLEN + 2, fp) != NULL) {
            if ((p = strchr(line, '\n')) == NULL) {
                  msg("line too long, file %s", name) ;
bad:              if (chdir("../in") < 0)
                        fatal("return to in") ;
                  return -1 ;
            }
            *p = '\0' ;
            if (badname(line)) {
                  msg("bad file %s acked in %s", line, name) ;
                  goto bad ;
            }
            if (unlink(line) < 0)
                  printf("Can't unlink %s/in/%s, ack file %s\n",
                        directory, line, name) ;
      }
      if (chdir("../in") < 0)
            fatal("return to in") ;
      if ((fp = fopen("../lastack", "w")) == NULL)
            fatal("can't open lastack") ;
      fprintf(fp, "%.9s\n", line + 1) ;
      fclose(fp) ;
      return 0 ;
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



/*
 * Run a program, informing the system administrator if it fails.
 */

chkrun(arg, name, fp)
      char *arg[] ;
      char *name ;
      FILE *fp ;
      {
      char *p ;
      int outfd ;
      int rc ;
      FILE *mailfp ;
      static char outfile[24] ;

      if (outfile[0] == '\0')
            sprintf(outfile, "/tmp/recvnews%d", getpid()) ;
      if ((outfd = creat(outfile, 0666)) < 0)
            fatal("Can't create %s", outfile) ;
      rc = run(arg, fileno(fp), outfd) ;
      close(outfd) ;
      if (rc != 0) {
            if ((mailfp = popen(MAILCMD, "w")) == NULL)
                  fatal("Can't popen MAILCMD") ;
            fprintf(mailfp, "Subject:  error in recvnews\n\n") ;
            if ((rc & 0177) == 0) {
                  fprintf(mailfp, "exit status %d from %s", rc >> 8, arg[0]) ;
            } else {
                  fprintf(mailfp, "%s died with signal %d", arg[0], rc & 0177) ;
                  if (rc & 0200)
                        fprintf(mailfp, " - core dumped") ;
            }
            fprintf(mailfp, "\nfile %s/bad/%s\n", directory, name) ;
            if ((fp = fopen(outfile, "r")) == NULL)
                  fprintf(mailfp, "Can't open %s\n", outfile) ;
            else {
                  fprintf(mailfp, "Output of program:\n") ;
                  while ((rc = getc(fp)) != EOF)
                        putc(rc, mailfp) ;
                  fclose(fp) ;
            }
            pclose(mailfp) ;
            if (unlink(outfile) < 0)
                  msg("can't unlink %s", outfile) ;
            return -1 ;
      }
      if (unlink(outfile) < 0)
            msg("can't unlink %s", outfile) ;
      return 0 ;
}



run(args, in, out)
      char *args[] ;
      int in, out ;
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
            if (in != 0) {
                  close(0) ;
                  if (dup(in) != 0) {
                        msg("Cannot redirect input") ;
                        exit(127) ;
                  }
                  close(in) ;
            }
            if (out != 1) {
                  close(1) ;
                  if (dup(out) != 1) {
                        msg("Cannot redirect output") ;
                        exit(127) ;
                  }
                  close(out) ;
                  close(2) ;
                  if (dup(1) != 2) {
                        msg("Cannot dup 1") ;
                        exit(127) ;
                  }
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
      reentered = 0 ;
      reset() ;
}


/*
 * Send mail to administrator.  Flag is set to indicate fatal error.
 */

msg(fmt, a1, a2, a3, a4)
      char *fmt ;
      {
      FILE *fp ;
      int e = errno ;

      errflag = 1 ;
      fprintf(stderr, fmt, a1, a2, a3, a4) ;
      putc('\n', stderr) ;
      if ((fp = popen(MAILCMD, "w")) == NULL)
            fatal("popen failed") ;
      fputs("Subject:  error in recvnews\n\n", fp) ;
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



setlock(name)
      char *name ;
      {
      FILE *fp ;
      char buf[10] ;

      if ((fp = fopen(name, "r")) != NULL) {
            if (fgets(buf, 10, fp) == NULL) {
                  msg("empty lock file") ;
                  fclose(fp) ;
                  goto lock ;
            }
            fclose(fp) ;
            if (buf[0] < '0' || buf[0] > '9') {
                  msg("no pid in lock file") ;
                  goto lock ;
            }
            if (! procexists(atoi(buf))) {
                  msg("previous recvnews didn't remove lock") ;
                  goto lock ;
            }
            return 0 ;
      }
lock:
      if ((fp = fopen(name, "w")) == NULL)
            fatal("cannot create lock file") ;
      fprintf(fp, "%d\n", getpid()) ;
      fclose(fp) ;
      return 1 ;
}
