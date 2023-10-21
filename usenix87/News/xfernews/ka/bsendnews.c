#define DEBUG
#include "common.h"

#define ASKIP 4

struct arglist uuargs ;
struct arglist outfiles ;
char *directory ;
int errflag ;
int nfiles ;
int nbatches ;
FILE *newsfp ;
FILE *batfp ;

char resentflag[] = "resentflag" ;
char lastack[] = "lastack" ;
char ackfile[] = "ackfile" ;
char batchtmp[] = "batch.tmp" ;
char newslist[] = "newslist" ;
char news1[] = "newslist.tmp1" ;
char news2[] = "newslist.tmp2" ;



main(argc, argv)
      char **argv ;
      {
      register char **ap ;
      int rflag ;
      int restart ;
      char *p ;
      int i ;
      FILE *fp ;

      setuid(NETNEWS) ;
      ap = argv + 1 ;
      rflag = 0 ;
      while ((p = *ap++) != NULL && *p == '-') {
            if (strcmp(p, "-r") == 0)
                  rflag++ ;
            else
usage:            fatal("usage: bsendnews [ -r ] from to") ;
      }
      if (p == NULL || *ap == NULL)
            goto usage ;
      cd(directory = p) ;
      uuargs.nargs = ASKIP ;

      restart = chksent() ;
      if (nbatches < MAXBATCH) {
            lsdir("out", &outfiles) ;
            nextnewslist() ;
      }
      if (restart)
            mkbatch() ;
      if ((fp = fopen(ackfile, "r")) != NULL) {
            if (restart || fsize(fileno(fp)) > 12
             || outfiles.nargs > 0 || newsfp != NULL) {
                  rm(ackfile) ;		/* should move it and delete later? */
                  if (! restart)
                        mkbatch() ;
                  addfile('a', fp) ;
                  nfiles++ ;
            }
            fclose(fp);
      }
      
      if (nbatches < MAXBATCH)
            dofiles() ;
      sendbatch() ;
      uucp(*ap, rflag) ;
      exit(errflag) ;
}



/*
 * Look through the sent directory, resending files and counting batch files.
 * If the link appears to be down, try to restart and inform administrator.
 */

chksent() {
      char sentname[PATHLEN] ;
      DIR *dp ;
      FILE *fp ;
      struct direct *d ;
      long curtime ;
      long last ;
      long t ;
      int resend ;		/* if set, consider resending articles */
      int nresent ;		/* # articles resent */
      int restart ;		/* if set, link is down */

      nresent = 0 ;
      resend = 1 ;
      restart = 1 ;
      fp = ckopen(lastack, "r") ;
      if (fgets(sentname, FNLEN, fp) == NULL) {
            /* Can occur bacause no locking done */
            msg("lastack is empty file") ;
            last = 0L ;
      } else {
            last = atol(sentname) ;
      }
      fclose(fp) ;
      if ((fp = fopen(resentflag, "r")) != NULL) {
            if (fgets(sentname, FNLEN, fp) == NULL) {
                  msg("%s is empty file", resentflag) ;
                  unlink(resentflag) ;
            } else if (atol(sentname) > last)
                  resend = 0 ;
            else
                  unlink(resentflag) ;
            fclose(fp) ;
      }
      time(&curtime) ;
      last -= 3600 ;
      if ((dp = opendir("sent")) == NULL)
            fatal("no sent directory") ;
      while ((d = readdir(dp)) != NULL) {
            if (d->d_name[0] == '.')
                  continue ;
            else if (badname(d->d_name)) {
                  msg("bad file %s in sent", d->d_name) ;
                  cd("sent") ;
                  movebad(d->d_name) ;
                  cd("") ;
                  continue ;
            }
            t = atol(d->d_name + 1) ;
            if (curtime - t < 24L*3600L)
                  restart = 0 ;
            if (resend && t < last) {
                  printf("resending %s\n", d->d_name) ;
                  addarg(d->d_name, &uuargs) ;
                  nresent++ ;
            }
            if (d->d_name[0] == 'b')
                  nbatches++ ;
      }
      closedir(dp) ;

      if (nbatches < MAXBATCH || nresent > 0)
            restart = 0 ;
      /* generate warning after 48 hours */
      if (nbatches >= MAXBATCH + 1 && restart)
            msg("The link appears to be down") ;
      
      if (nresent) {
            if ((fp = fopen(resentflag, "w")) == NULL)
                  msg("Can't create resentflag") ;
            else {
                  fprintf(fp, "%ld\n", curtime) ;
                  fclose(fp) ;
            }
            msg("Resent %d files", nresent) ;
      }
      return restart ;
}



/*
 * Create the batch files.
 */

dofiles() {
      FILE *fp ;
      int ftype ;
      char file[PATHLEN] ;

      while ((ftype = nextfile(file)) != EOF) {
            if ((fp = fopen(file, "r")) == NULL) {
                  msg("Can't open %s", file) ;
                  continue ;
            }
            if (nfiles >= 194 || nfiles > 0 && fsize(fileno(fp)) + ftell(batfp) > MAXBATSIZE) {
                  if (nbatches >= MAXBATCH - 1) {
                        fclose(fp) ;
                        break ;
                  }
                  sendbatch() ;
                  zapfiles() ;
                  mkbatch() ;
                  nbatches++ ;
                  nfiles = 0 ;
            }
            addfile(ftype, fp) ;
            fclose(fp) ;
            nfiles++ ;
      }
      sendbatch() ;
      zapfiles() ;
      nbatches++ ;
}


/*
 * Regrettably, the code for finding the files to batch is rather convoluted.
 *
 * Nextfile returns the next file to add to the batch.  First, any files in
 * out are considered.  Then the files news.tmp1 and news.tmp2 are processed
 * if they exist.  (They may if the previous bsendnews died in the middle.)
 * Finally, newslist is moved to news.tmp1 (or news.tmp2 if news.tmp1 already
 * exists), and is read.
 *
 * Zapfiles eliminates the files which have been placed in the last batch from
 * further consideration.  (Note that the last file returned by nextfile has
 * NOT been processed when zapfiles is called.)  Files in out are deleted.
 * Files listed in news.tmp[12] are overwritten with nulls if there are entries
 * in these files which have not been processed yet.  Otherwise news.tmp[12]
 * are deleted.
 */

static char lline[PATHLEN] ;
static int outind = 0 ;
static int ateof ;
static char *curfile ;
static int zap1 ;

nextfile(name)
      char *name ;
      {
      char *p ;

      if (outind < outfiles.nargs) {
            p = outfiles.arg[outind++] ;
            sprintf(name, "out/%s", p) ;
            return *p ;
      } else {
            while (newsfp == NULL || fgets(lline, PATHLEN, newsfp) == NULL) {
                  if (nextnewslist() == 0)
                        return EOF ;
            }
            strcpy(name, lline) ;
            if ((p = index(name, '\n')) == NULL)
                  fatal("missing newline in newslist") ;
            *p = '\0' ;
            return 'n' ;
      }
}


zapfiles() {
      static int outdel ;
      FILE *fp ;
      int len ;

      if (outdel < outind) {
            cd("out") ;
            while (outdel < outind)
                  rm(outfiles.arg[outdel++]) ;
            cd("") ;
      }
      if (outind >= outfiles.nargs && curfile != NULL) {
            cd("") ;
            if (zap1 && curfile != news1) {
                  rm(news1) ;
                  zap1 = 0 ;
            }
            if (newsfp == NULL)
                  rm(curfile) ;
            else {
                  fp = ckopen(curfile, "r+") ;
                  len = ftell(newsfp) - strlen(lline) ;
                  while (--len >= 0)
                        if (putc('\0', fp) == EOF)
                              fatal("write error on newstmp") ;
                  fclose(fp) ;
            }
      }
}


/*
 * Open the next newslist file.  First try the temp files, and then open
 * newslist.
 */

nextnewslist() {
      static int did1 ;
      static int state = -1 ;
      register c ;
      char *p ;

      if (newsfp != NULL) {
            fclose(newsfp) ;
            newsfp = NULL ;
      }
      for (;;) switch (++state) {
            case 0:
                  if ((newsfp = fopen(news1, "r")) != NULL) {
                        did1 = zap1 = 1 ;
                        curfile = news1 ;
                        goto found ;
                  }
                  break ;
            case 1:
                  if ((newsfp = fopen(news2, "r")) != NULL) {
                        state = 3 ;
                        curfile = news2 ;
                        goto found ;
                  }
                  break ;
            case 2:
                  if (link(newslist, p = did1? news2 : news1) >= 0) {
                        curfile = p ;
                        rm(newslist) ;
                        newsfp = ckopen(curfile, "r") ;
                        goto found ;
                  }
                  break ;
            default:
                  return 0 ;
      }
found:
      while ((c = getc(newsfp)) == '\0') ;
      ungetc(c, newsfp) ;
      return 1 ;
}


/*
 * Add current batch to list of files to be processed.
 */
sendbatch() {
      long len ;
      long t ;
      static long lastt ;
      char name[FNLEN + 5] ;

      if (batfp == NULL)
            return ;
      closebatch() ;
      time(&t) ;
      if (t <= lastt)
            t = lastt + 1 ;
      lastt = t ;
      sprintf(name, "sent/b%09ld", t) ;
      mv(batchtmp, name) ;
      addarg(name + 5, &uuargs) ;
   /* nbatches++ ; */
}



uucp(to, rflag)
      char *to ;
      {
      char **ap ;

      if (uuargs.nargs == ASKIP)
            return ;
      cd("sent") ;
      qsort((char *)(uuargs.arg + ASKIP), uuargs.nargs - ASKIP, sizeof(char *), comp) ;
      addarg(to, &uuargs) ;
      addarg(NULL, &uuargs) ;
      ap = uuargs.arg + ASKIP ;
      if (rflag)  *--ap = "-r" ;
      *--ap = "-c" ;			/* default on most uucps */
      *--ap = UUCP ;
      if (run(ap, 0, 1) != 0)
            fatal("uucp failed") ;
}


#include "common.c"
