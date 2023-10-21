#define RECVNEWS 1
#define PACK
#include "common.h"

char *directory ;	/* directory currently being processed */
int errflag ;		/* set if any errors */
char lockfile[] = RECVLOCK ;
char unbatch[] = "unbatch" ;
char unbattmp[] = "../unbatch.tmp" ;
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
      case 'b':
            rc = procbatch(name, fp, &batch) ;
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
            proclist(&batch, unbatch) ;
      }
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
      if ((p = index(buf, '\n')) == NULL) {
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


#ifndef PACK
procbatch(name, fp, l)
      char *name ;
      FILE *fp ;
      struct arglist *l ;
      {
      register c ;
      FILE *ufp ;
      int seqno ;
      char hdline[64] ;
      char fname[FNLEN] ;
      char *emsg ;
      int rc ;

      seqno = getpid() % 9000 ;
      rc = -2 ;
      if (fgets(hdline, 64, fp) == NULL) {
            emsg = "empty batch file" ;
bad:        msg("%s: %s", name, emsg) ;
            return rc ;
      }
      if (atol(hdline + 1) != fsize(fileno(fp))) {
            emsg = "bad size" ;
            goto bad ;
      }
      rc = -1 ;
      if (hdline[0] != 'a') {
            emsg = "bad version" ;
            goto bad ;
      }
      if (index(hdline, '\n') == NULL) {
            emsg = "header too long" ;
            goto bad ;
      }
      l->nargs = 0 ;
      cd(unbatch) ;
      while ((c = getc(fp)) != EOF) {
            sprintf(fname, "%c%04d", c, seqno++) ;
            if (badname(fname) || fname[0] == 'b') {
                  emsg = "bad file type" ;
                  goto error ;
            }
            ufp = ckopen(unbattmp, "w") ;
            while ((c = getc(fp)) != ENDMARK) {
                  if (c == EOF) {
                        emsg = "unexpected EOF" ;
error:                  c = dellist(l) ;
                        msg("%s: %s", name, emsg) ;
                        if (c) fatal("unlinking batch files failed") ;
                        return -1 ;
                  }
                  if (putc(c, ufp) == EOF) {
                        emsg = "write error on unbatch.tmp" ;
                        goto error ;
                  }
            }
            if (fclose(ufp) == EOF) {
                  emsg = "close error on unbatch.tmp" ;
                  goto error ;
            }
            mv(unbattmp, fname, 0) ;
            addarg(fname, l) ;
      }
      return 0 ;
}
#else
procbatch(name, fp, l)
      char *name ;
      FILE *fp ;
      struct arglist *l ;
      {
      register c ;
      FILE *ufp ;
      int seqno ;
      char hdline[64] ;
      char fname[FNLEN] ;
      char *emsg ;
      int rc ;

      seqno = getpid() % 9000 ;
      rc = -2 ;
      if (fread(hdline, 1, 5, fp) != 5) {
            emsg = "empty batch file" ;
bad:        msg("%s: %s", name, emsg) ;
            return rc ;
      }
      if ((hdline[2] & 0377) + ((hdline[3] & 0377) << 8) + ((hdline[4] & 0377) << 8) != fsize(fileno(fp))) {
            emsg = "bad size" ;
            goto bad ;
      }
      rc = -1 ;
      if (hdline[0] != 'b') {
            emsg = "bad version" ;
            goto bad ;
      }
      for (c = hdline[1] - 5 ; c > 0 ; c++)
            getc(fp) ;
      l->nargs = 0 ;
      cd(unbatch) ;
      for (;;) {
            MOREBITS() ;
            if ((c = bits & 03) == 3)
                  break ;
            sprintf(fname, "%c%04d", "nma"[c], seqno++) ;
            ufp = ckopen(unbattmp, "w") ;
            if ((emsg = readfile(c, ufp)) == 0) {
error:                  c = dellist(l) ;
                        msg("%s: %s", name, emsg) ;
                        if (c) fatal("unlinking batch files failed") ;
                        return -1 ;
            }
            if (fclose(ufp) == EOF) {
                  emsg = "close error on unbatch.tmp" ;
                  goto error ;
            }
            mv(unbattmp, fname, 0) ;
            addarg(fname, l) ;
      }
      return 0 ;
}



readfile(hdflag, ufp)
      FILE *ufp ;
      {
      char *lp ;
      char line[1024] ;
      struct decode *dp ;

      lp = line ;
      for (;;) {
            MOREBITS() ;
            dp = dtab[bits & 0777] ;
            USEUP(dp->clen) ;
            if ((dp->cval & 0200) == 0) {
                  if (lp >= line + 1024) {
                        if (fwrite(line, 1024, 1, ufp) != 1024)
                              return "write error" ;
                  }
                  *lp++ = dp->cval ;
                  continue ;
            }
            switch (dp->cval & 0177) {
            case PKENDMARK:
                  goto out ;
            case EXTCHAR:
                  *lp++ = extchar[bits & 037] ;
                  USEUP(5) ;
                  break ;
            }
      }
out:
      if (fwrite(line, lp - line, 1, ufp) != lp - line)
            return "write error" ;
      return NULL ;
}
#endif


/*
 * Delete files in list.  A status code is returned, but fatal is not called
 * on error because dellist is used to clean up the batch directory when an
 * error is already pending.
 */

dellist(l)
      struct arglist *l ;
      {
      register int i ;
      int errcnt ;

      errcnt = 0 ;
      for (i = 0 ; i < l->nargs ; i++)
            if (unlink(l->arg[i]) < 0)
                  errcnt++ ;
      return errcnt ;
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


#include "common.c"
