#include "common.h"
#include "pack.h"

extern FILE *batfp ;
extern char batchtmp[] ;

long bits ;
int nbits ;
char saverelay[200] ;
char relaysite[64] ;
char postsite[64] ;
char artid[16] ;


mkbatch() {
      batfp = ckopen(batchtmp, "w") ;
      fputs("bXXX", batfp) ;
      saverelay[0] = '\0' ;
      nbits = 0 ;
}


closebatch() {
      long len ;

      addbits(3, LFTYPE) ;		/* EOF marker */
      if (nbits > 0)
            addbits(0, BS - nbits) ;		/* pad to byte length */
      if ((len = ftell(batfp)) <= 0L)
            fatal("bad ftell") ;
      if (fseek(batfp, 1L, 0))
            fatal("batch file seek error") ;
      putc(len << 4 | BVERSION, batfp) ;
      putc(len >> 4, batfp) ;
      putc(len >>12, batfp) ;
      if (fclose(batfp) == EOF)
            fatal("batch file close error") ;
      batfp = NULL ;
}


addfile(type, fp)
      FILE *fp ;
      {
      int count ;
      char buf[200] ;
      char *p ;

      if ((p = index(ftypes, type)) == NULL)
            fatal("bad file type %c", type) ;
      addbits(p - ftypes, LFTYPE) ;
      if (type == 'n') {
            newshead(fp) ;
      }
      while ((count = fread(buf, 1, 200, fp)) > 0)
            bputchars(buf, count) ;
      addbits(PKEOFMARK, LEOFMARK) ;
}


newshead(fp) {
      char line[1024] ;
      char temp[512] ;
      char *p, *q ;
      char *lp, *endp ;
      int ltype ;
      int first ;

      first = 1 ;
      postsite[0] = '\0' ;
      while ((line[0] = '\0', fgets(line, 1024, fp)) != NULL
	&& line[0] != '\n' && (endp = index(line, '\n')) != NULL) {
            *endp = '\0' ;
            for (ltype = 0 ; !prefix(line, hprefix[ltype]) ; ltype++) ;
            lp = line + strlen(hprefix[ltype]) ;
            if (first) {
                  first = 0 ;
                  if (ltype == NHRELAY && equal(lp, saverelay)) {
                        addbits(1, 1) ;
                        continue ;
                  } else
                        addbits(0, 1) ;
            }
            switch (ltype) {
            case NHRELAY:
            case NHVERSION:
                  if ((p = index(lp, ';')) != NULL && ! prefix(p, "; site "))
                        goto unknown ;
                  p += 7 ;
                  if (ltype == NHRELAY) {
                        strcpy(saverelay, lp) ;
                        strcpy(relaysite, p) ;
                  } else
                        strcpy(postsite, p) ;
                  do *(p-6) = *p ;
                  while (*p++) ;
                  break ;
            case NHMESSAGEID:
                  if (endp[-1] != '>' || (p = index(lp, '@')) == NULL)
                        goto unknown ;
                  *--endp = '\0' ;
                  getartid(lp) ;
                  if (equal(p+1, postsite) && index(p+1, '@') == NULL)
                        endp = p ;
                  break ;
            case NHARTICLEID:
                  if (artid[0]) {
                        if (! equal(lp, artid))
                              goto unknown ;
                        
                        lp = NULL ;
                  }
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
            case NHUNKNOWN:
                  if (prefix(line, "Date-Received:"))
                        continue ;
                  break ;
            unknown:
                  ltype = NHUNKNOWN ;
                  lp = line ;
            }
            addbits(ltype, 5) ;
            if (lp) {
                  *endp++ = '\n' ;
                  bputchars(lp, endp - lp) ;
            }
      }
      if (line[0] == '\n')
            addbits(NHSEP, 5) ;
      else {
            addbits(NHEND, 5) ;
            bputchars(line, strlen(line)) ;
      }
}


genartid(msgid)
      char *msgid ;
      {
      register char *p, *q ;

      strcpy(temp, line);
      p = index(temp, '@');
      *p++ = '\0';
      q = index(p, '.');
      if (q)
            *q++ = '\0';
      p[8] = '\0';
      sprintf(artid, "%s.%s", p, temp);
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
            if ((rnbits += ep->clen) >= BS) {
                  do {
                        if (putc(rbits, batfp) == EOF)
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


prefix(full, pref)
      register char *full, *pref;
      {
      register c ;
      while ((c = *pref++)) {
            if (*full++ != c)
                  return 0 ;

      }
      return 1 ;
}
