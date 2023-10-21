#define DEBUG
#include "common.h"

#define ASKIP 3

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

      chksent() ;
      lsdir("out", &outfiles) ;
      nextnewslist() ;
      if ((fp = fopen(ackfile, "r")) != NULL && fsize(fileno(fp)) > MINACK
       || outfiles.nargs > 0 || newsfp != NULL) {
            mkbatch() ;
            if (fp) {
                  rm(ackfile) ;
                  addfile('a', fp) ;
            }
      }
      if (fp)
            fclose(fp) ;
      dofiles() ;
      uucp(*ap, rflag) ;
      exit(errflag) ;
}



/*
 * Look through the sent directory, resending files and counting batch files.
 */

chksent() {
      char sentname[PATHLEN] ;
      DIR *dp ;
      FILE *fp ;
      struct direct *d ;
      long last ;
      int resend ;
      int nresent ;

      resend = unlink(resentflag) ;
      nresent = 0 ;
      fp = ckopen(lastack, "r") ;
      if (fgets(sentname, FNLEN, fp) == NULL) {
            /* Can occur bacause no locking done */
            msg("lastack is empty file") ;
            last = 0L ;
      } else {
            last = atol(sentname) - 3600L ;
      }
      fclose(fp) ;
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
            if (resend && atol(d->d_name + 1) < last) {
                  printf("resending %s\n", d->d_name) ;
                  addarg(d->d_name, &uuargs) ;
                  nresent++ ;
            }
            if (d->d_name[0] == 'b')
                  nbatches++ ;
      }
      closedir(dp) ;
      if (nresent) {
            if (close(creat(resentflag, 0666)) < 0)
                  msg("Can't create resentflag") ;
            msg("Resent %d files", nresent) ;
      }
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
            if (nfiles > 0 && fsize(fileno(fp)) + ftell(batfp) > MAXBATSIZE) {
                  if (nbatches >= MAXBATCH) {
                        fclose(fp) ;
                        break ;
                  }
                  sendbatch() ;
                  zapfiles() ;
                  mkbatch() ;
            }
            addfile(ftype, fp) ;
            fclose(fp) ;
      }
      sendbatch() ;
      zapfiles() ;
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


#ifndef PACK
mkbatch() {
      batfp = ckopen(batchtmp, "w") ;
      fputs("a000000\n", batfp) ;
      nfiles = 0 ;
}
#else
mkbatch() {
      batfp = ckopen(batchtmp, "w") ;
      fputs("b\5\1\1\1", batfp) ;
      nfiles = 0 ;
      saverelay[0] = '\0' ;
      nbits = 0 ;
}
#endif


#ifndef PACK
sendbatch() {
      long len ;
      long t ;
      static long lastt ;
      char name[FNLEN + 5] ;

      if (batfp == NULL)
            return ;
      if ((len = ftell(batfp)) <= 0L)
            fatal("bad ftell") ;
      if (fseek(batfp, 1L, 0))
            fatal("batch file seek error") ;
      fprintf(batfp, "%06ld", len) ;
      if (fclose(batfp) == EOF)
            fatal("batch file close error") ;
      batfp = NULL ;
      if ((t = time(NULL)) <= lastt)
            t = lastt + 1 ;
      lastt = t ;
      sprintf(name, "sent/b%09ld", t) ;
      mv(batchtmp, name) ;
      addarg(name + 5, &uuargs) ;
      nbatches++ ;
}
#else
sendbatch() {
      long len ;
      long t ;
      static long lastt ;
      char name[FNLEN + 5] ;

      if (batfp == NULL)
            return ;
      addbits(3, 2) ;		/* EOF marker */
      if (nbits > 0)
            addbits(0, BS - nbits) ;		/* pad to byte length */
      if ((len = ftell(batfp)) <= 0L)
            fatal("bad ftell") ;
      if (fseek(batfp, 2L, 0))
            fatal("batch file seek error") ;
      putc(len & 0377, batfp) ;
      putc(len >> 8 & 0377, batfp) ;
      putc(len >> 16 & 0377, batfp) ;
      if (fclose(batfp) == EOF)
            fatal("batch file close error") ;
      batfp = NULL ;
      if ((t = time(NULL)) <= lastt)
            t = lastt + 1 ;
      lastt = t ;
      sprintf(name, "sent/b%09ld", t) ;
      mv(batchtmp, name) ;
      addarg(name + 5, &uuargs) ;
      nbatches++ ;
}
#endif


#ifndef PACK
addfile(type, fp)
      FILE *fp ;
      {
      register int c ;

      if (putc(type, batfp) == EOF)
bad:        fatal("batch file write error") ;
      while ((c = getc(fp)) != EOF)
            if (putc(c, batfp) == EOF)
                  goto bad ;
      if (putc(ENDMARK, batfp) == EOF)
            goto bad ;
      nfiles++ ;
}
#else
addfile(type, fp)
      FILE *fp ;
      {
      int count ;
      char buf[200] ;
      char *p ;

      if ((p = index(ftypes, type)) == NULL)
            fatal("bad file type %c", type) ;
      addbits(p - ftypes, 2) ;
      if (type == 'n') {
            newshead(fp) ;
      }
      while ((count = fread(buf, 1, 200, fp)) > 0)
            bputchrs(buf, count) ;
      addbits(PKENDMARK, 9) ;
      nfiles++ ;
}


newshead(fp) {
      char line[1024] ;
      char *p, *q ;
      char *lp, *endp ;
      int ltype ;
      int first ;

      first = 1 ;
      while ((line[0] = '\0', fgets(line, 1024, fp)) != NULL
	&& line[0] != '\n' && (endp = index(line, '\n')) != NULL) {
            *endp = '\0' ;
            for (ltype = 0 ; ltype < 31 ; ltype++) {
                  if (*(p = nprefix[ltype]) != '\0' && strncmp(p, line, strlen(p)) == 0)
                        break ;
            }
            lp = line + strlen(nprefix[ltype]) ;
            if (first) {
                  if (ltype == NHRELAY && equal(lp, saverelay)) {
                        addbits(1, 1) ;
                        continue ;
                  } else
                        addbits(0, 1) ;
                  first = 0 ;
            }
            switch (ltype) {
            case NHRELAY:
            case NHVERSION:
                  if ((p = index(lp, ';')) != NULL && ! prefix("; site ", p))
                        goto unknown ;
                  if (ltype == NHRELAY && endp - lp < 200)
                        strcpy(saverelay, lp) ;
                  p += 7 ;
                  do *(p-6) = *p ;
                  while (*p++) ;
                  break ;
            case NHMSGID:
                  if (endp[-1] != '>')
                        goto unknown ;
                  *--endp = '\0' ;
                  break ;
/*
            case NHDATE:
            case NHPOSTED:
            case NHEXPIRES:
                  tim = getdate(lp) ;
                  if (tim <= 0)
                        goto unknown ;
                  addbits(ltype, 5) ;
                  addbits(tim & 077777, 15) ;
                  addbits(tim >> 15 & 077777, 15) ;
                  break ;
*/
            case NHUNKNOWN ;
                  if (prefix("Date-Received:", line))
                        continue ;
                  break ;
            unknown:
                  ltype = NHUNKNOWN ;
                  lp = line ;
            }
            addbits(ltype, 5) ;
            *endp++ = '\n' ;
            *endp++ = '\0' ;
            bputchars(lp, endp - lp) ;
      }
      if (line[0] == '\n')
            addbits(NHSEP, 5) ;
      else {
            addbits(NHEND, 5) ;
            bputchars(line, strlen(line)) ;
      }
}



bputchars(p, count)
      register char *p ;
      {
      register long rbits ;
      register int rnbits ;
      register struct encode *ep ;

      rbits = bits ;
      rnbits = nbits ;
      while (--count >= 0) {
            ep = &etab[*p++ & 0177] ;
            rbits |= ep->cbits << rnbits ;
            if ((rnbits += ep->cbits) >= BS) {
                  do {
                        if (putc(bits, batfp) == EOF)
                              fatal("batch file write error") ;
                        rbits >>= BS ;
                  } while ((rnbits -= BS) >= BS) ;
            }
      }
      bits = rbits ;
      nbits = rnbits ;
}


addbits(b, nb) {
      bits |= (long)b << nbits ;
      if ((nbits += nb) >= BS) {
            do {
                  if (putc(bits, batfp) == EOF)
                        fatal("batch file write error") ;
                  bits >>= BS ;
            } while ((nbits -= BS) >= BS) ;
      }
}
#endif

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
      *--ap = UUCP ;
      if (run(ap, 0, 1) != 0)
            fatal("uucp failed") ;
}


#include "common.c"
