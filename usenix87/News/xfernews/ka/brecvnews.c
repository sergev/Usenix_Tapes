#define RECVNEWS 1
#include "common.h"

char *directory ;	/* directory currently being processed */
int errflag ;		/* set if any errors */
char lockfile[] = RECVLOCK ;
char unbatch[] = "unbatch" ;
char in_dir[] = "in" ;



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
            cd(directory) ;
            inputnews() ;
      }
      if (unlink(lockfile) < 0)
            msg("can't unlink lock") ;
      exit(errflag) ;
}



inputnews() {
      struct arglist in ;

      in.nargs = 0 ;
      lsdir(unbatch, &in) ;
      proclist(&in, unbatch) ;
      in.nargs = 0 ;
      lsdir(in_dir, &in) ;
      sleep(5) ;		/* in case any files half written */
      proclist(&in, in_dir) ;
}


proclist(l, dir)
      struct arglist *l ;
      char *dir ;
      {
      int i ;
      char *p ;
      
      if (l->nargs == 0)
            return ;
      for (i = 0 ; i < l->nargs ; i++) {
            p = l->arg[i] ;
            procfile(p, dir) ;
            free(p) ;
      }
      cd("") ;
}



procfile(name, dir)
      char *name ;
      char *dir ;
      {
      FILE *fp ;
      int rc ;
      struct arglist batch ;

      cd(dir) ;
      if (badname(name) || dir == unbatch && name[0] == 'b') {
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
      case 'b':
            rc = procbatch(name, fp) ;
            break ;
      default:
            fatal("can't happen %s", name) ;
            break ;
      }
      cd(dir) ;
      if (rc < 0)
            movebad(name) ;
      else
            rm(name) ;
      fclose(fp) ;
      if (dir == in_dir && rc != -2) {
            fp = ckopen("../ackfile", "a") ;
            fprintf(fp, "%s\n", name) ;
            fclose(fp) ;
      }
      if (name[0] == 'b' && rc >= 0) {
            cd("") ;
            lsdir(unbatch, &batch) ;
            proclist(&batch, unbatch) ;
      }
}



/*
 * Process a news item.  We simply feed it into rnews.
 */

procnews(name, fp)
      char *name ;
      FILE *fp ;
      {
      char *arg[2] ;

      arg[0] = RNEWS, arg[1] = NULL ;
      return chkrun(arg, name, fp) ;
}


/*
 * Process a mail file.  The first line of the file contains the word
 * "To " followed by the destination of the mail.  The remainder of the
 * file contains the letter.  No attempt is made to return bad mail, so
 * if the mail command fails you will have to return it manually.
 */

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
      if ((p = index(buf, '\n')) == NULL) {
            msg("destination too long, file %s", name) ;
            return -1 ;
      }
      *p = '\0' ;
      arg[0] = RMAIL, arg[1] = buf + 3, arg[2] = NULL ;
      return chkrun(arg, name, fp) ;
}



/*
 * Process an acknowledgement file.  The acknowledged files are removed
 * from the sent directory.  The time stamp on the last acknowledged file
 * is written to the lastack file; files older than the last acknowledged
 * file will be retransmitted by sendnews.
 */

procack(name, fp)
      char *name ;
      FILE *fp ;
      {
      char line[FNLEN+2] ;
      char *p ;

      cd("sent") ;
      while (fgets(line, FNLEN + 2, fp) != NULL) {
            if ((p = index(line, '\n')) == NULL) {
                  msg("line too long, file %s", name) ;
bad:              return -1 ;
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
      if ((fp = fopen("../lastack", "w")) == NULL)
            fatal("can't open lastack") ;
      fprintf(fp, "%.9s\n", line + 1) ;
      fclose(fp) ;
      return 0 ;
}


/*
 * Process a batch file.  The first byte of the batch file indicates
 * which unbatcher program is to be used, in order to simplify the task
 * of supporting multiple batch formats.
 */

procbatch(name, fp)
      char *name ;
      FILE *fp ;
      {
      char c ;
      char batcher[PATHLEN] ;
      char *arg[2] ;
      int i ;

      if (read(fileno(fp), &c, 1) != 1) {
            msg("empty batch file %s", name) ;
            return -2 ;
      }
      lseek(fileno(fp), 0L, 0) ;
      sprintf(batcher, "%s/unbat_%c", LIBDIR, c) ;
      arg[0] = batcher, arg[1] = NULL ;
      cd(unbatch) ;
      if ((i = chkrun(arg, name, fp)) < 0) {
            rmall() ;
            return i ;
      }
      return 0 ;
}



/*
 * Remove all files in the current directory
 */

rmall() {
      DIR *dp ;
      struct direct *d ;
      int nfail ;

      nfail = 0 ;
      if ((dp = opendir(".")) == NULL)
            fatal("Can't open unbatch/.") ;
      while ((d = readdir(dp)) != NULL) {
            if (badname(d->d_name))
                  continue ;
            if (unlink(d->d_name))
                  nfail++ ;
      }
      closedir(dp) ;
      if (nfail)
            fatal("%d files could not be removed from unbatch", nfail) ;
}



/*
 * Set the lock to avoid multiple invokations of recvnews.  This code
 * may fail if two copies of recvnews are invoked simultaneously.  A
 * new copy of recvnews should be started up every 10 or 15 minutes by
 * cron.  This code also assumes that it is possible to tell if a process
 * exists by invoking kill with a signal number of zero.  If this does
 * not work it may be possible for the lock to be left on until the
 * next time /tmp is cleaned out.
 */

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


#include "common.c"
