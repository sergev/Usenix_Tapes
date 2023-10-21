/*
 * Pass 2:  Check header lines for errors.
 *
 * Copyright 1984 by Kenneth Almquist.  All rights reserved.
 *     Permission is granted for anyone to use and distribute, but not
 *     sell, this program provided that this copyright notice is retained.
 */

#include "postnm.h"

char *cmsg[] = {		/* list of control messages */
      "cancel",
      "version",
      "ihave",
      "sendme",

      "newgroup",
      "rmgroup",
      "sendsys",
      "senduuname",
      "checkgroups",
      "delsub",
      NULL
};
#define PRIVCMSG (&cmsg[4])	/* start of priviledged control messages */



checkheader() {
      char *p, *q;
      struct hline *hp;

      curhdr = hdrline[HTSUBJECT];
      if (curhdr == NULL) {
            if (hdrline[HTCONTROL]) {
                  sprintf(bfr, "Subject: %s", hdrline[HTCONTROL]->body);
                  prochdr(bfr, 1);
            } else {
                  fprintf(stderr, "postnm: no subject line\n");
                  nerrors++;
            }
      }
      setexit();
      if (hdrline[HTCOMMAND]) {
            hlselect(hdrline[HTCOMMAND]);
            hlzap(HTCOMMAND);
            skipbl();
            p = scanp;
            scnchr(" \t\n");
            q = scanp;
            skipws(" \t\n");
            if (*scanp != '\0')
cmdjsynerr:        jsynerr("Command must be \"reply\", \"followup\", or \"both\"");
            *q = '\0';
            if (equal(p, "reply") || equal(p, "r"))
                  hlzap(HTNEWSGROUPS), hlzap(HTDIST);
            else if (equal(p, "followup") || equal(p, "f"))
                  hlzap(HTTO);
            else if (!equal(p, "both"))
                  goto cmdjsynerr;
      }

      for (hp = hfirst ; hp ; hp = hp->next) {
            fixheader(hp);
      }
      if (references && hdrline[HTNEWSGROUPS] && !hdrline[HTREFERENCES]) {
            sprintf(bfr, "References: %s", references);
            prochdr(bfr, 1);
      }
      hlzap(HTBCC);
      cknewsgroups();
      *addrp = NULL;
}



/*
 * Process a header line.
 */

fixheader(hp)
      struct hline *hp;
      {
      extern char *begaddr, *endaddr;
      char *p;
      register char **pp;
      time_t tim;
      struct tm *tm;
      static char month[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
				  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

      if (setexit())
            return;
      hlselect(hp);
      switch (hp->type) {
      case HTTO:
      case HTCC:
      case HTBCC:
            for (;;) {
                  skipws(" \t\n,");
                  if (*scanp == '\0')
                        break;
                  if (addrp >= &addrlist[MAXADDR])
                        jsynerr("too many mail addresses");
                  parseaddr();
                  *addrp++ = getval(begaddr, endaddr, NULL);
            }
            break;

      case HTDIST:
            commalist();
            for (pp = nlist ; *pp ; pp++)
                  if (equal(*pp, "net"))
                        *pp = "world";
            rmdup(nlist);
            if (nlist[0] == NULL || nlist[1] == NULL && equal(nlist[0], "world"))
                  hlzap(HTDIST);
            else
                  ckdist();
            for (pp = nlist ; *pp ; pp++)
                  if (equal(*pp, "world"))
                        *pp = "net";
            goto outlist;

      case HTFOLLOWUPTO:
            commalist();
            if (nlist[0] == NULL)
                  jsynerr("empty followup-to line");
            rmdup(nlist);
            bcopy((char *)nlist, (char *)follist, sizeof follist);
            goto outlist;

      case HTNEWSGROUPS:
            commalist();
            if (nlist[0] == NULL)
                  jsynerr("empty newsgroups line");
            rmdup(nlist);
            bcopy((char *)nlist, (char *)nglist, sizeof nglist);

outlist:    outp = bfr;
            if (nlist[0]) {
                  for (pp = nlist ; *pp ; pp++) {
                        if (outp != bfr)
                              outstr(",");
                        outstr(*pp);
                  }
                  outstr("\n");
                  hp->fixed = savestr(bfr);
            }
            break;

      case HTEXPIRES:
            if ((tim = cgtdate(scanp)) == -1L)
                  jsynerr("Can't parse date");
            tm = gmtime(&tim);
            sprintf(bfr, "%d %s %d %02d:%02d:%02d GMT\n", tm->tm_mday,
                    month[tm->tm_mon], tm->tm_year, tm->tm_hour, tm->tm_min,
                    tm->tm_sec);
            hp->fixed = savestr(bfr);
            break;

      case HTCONTROL:
            p = scanp;
            scnchr(" \t\n");
            getval(p, scanp, bfr);
            for (pp = cmsg ; *pp && !equal(*pp, bfr) ; pp++);
            if (*pp == NULL)
                  jsynerr("Unknown control message \"%s\"", bfr);
            else if (pp >= PRIVCMSG && !isadmin())
                  jsynerr("Get your system administrator to send this control message for you.");
            break;

      case HTSUBJECT:
            if (*scanp == '\n')
                  jsynerr("empty subject line");
            if (prefix(scanp, "cmsg "))
                  jsynerr("subjects beginning with cmsg confuse inews");
            if (prefix(scanp, "Re: ") && !hdrline[HTREFERENCES] && !references)
                  jsynerr("followup articles must have references lines");
            break;

      case HTREFERENCES:
            if (references && !equal(scanp, references)) {
                  fprintf(stderr, "postnm: line %d: you changed the references line but I fixed it\n", curhdr->linno);
                  curhdr->fixed = references;
            }
            break;

      case HTAPPROVED:
            ckapproved();
            break;

      case HTORGANIZATION:
            {
                  /* Inews ignores organization lines */
                  /* I shouldn't have to do this */
                  char **pp, **qq;
                  extern char **environ;
                  sprintf(bfr, "ORGANIZATION=%s", scanp);
                  nstrip(bfr);
                  envkludge[0] = savestr(bfr);
                  qq = envkludge + 1;
                  for (pp = environ ; pp < environ + 199 && *pp ; pp++)
                        if (!prefix(*pp, "ORGANIZATION="))
                              *qq++ = *pp;
                  *qq = 0;
                  environ = envkludge;
            }
            break;

      default:
            break;
      }
}



/*
 * Check that newsgroups or distributions are legal.
 */

checklist(file, thing)
      char *file, *thing;
      {
      char fname[FPATHLEN];
      FILE *fp;
      char *left[MAXGRPS + 1], **pp;

      sprintf(fname, "%s/%s", LIB, file);
      fp = ckfopen(fname, "r");
      bcopy((char *)nlist, (char *)left, sizeof(left));
      while (fgets(bfr, LBUFLEN, fp) != NULL) {
            scanp = bfr;
            scnchr(" \t");
            *scanp = '\0';
            for (pp = left ; *pp ; pp++) {
                  if (equal(*pp, bfr)) {
                        do *pp = *(pp + 1);
                        while (*pp++);
                        break;
                  }
            }
      }
      fclose(fp);
      for (pp = left ; *pp ; pp++)
            synerr("illegal %s %s", thing, *pp);
}



/*
 * Check distributions for validity.  If they aren't in the distributions
 * file, we try the active file.
 */

ckdist() {
      FILE *fp;
      char *p;
      char **pp;
      char *dlist[MAXGRPS + 1];

      bcopy((char *)nlist, (char *)dlist, sizeof dlist);
      sprintf(bfr, "%s/distributions", LIB);
      if ((fp = fopen(bfr, "r")) != NULL) {
            while (dlist[0] && fgets(bfr, LBUFLEN, fp)) {
                  if (bfr[0] == '\n' || bfr[0] == '#')
                        continue;
                  if ((p = strpbrk(bfr, " \t\n")) == NULL)
                        continue;
                  *p = '\0';
                  rmlist(bfr, dlist);
            }
            fclose(fp);
      }
      else  printf("postnm: warning: no distributions file\n");
      if (dlist[0]) {
            sprintf(bfr, "%s/active", LIB);
            fp = ckfopen(bfr, "r");
            while (dlist[0] && fgets(bfr, LBUFLEN, fp)) {
                  if ((p = strpbrk(bfr, " \n")) == NULL)
                        continue;
                  *p = '\0';
                  for (pp = dlist ; *pp ; ) {
                        if (ngmatch(bfr, *pp))
                              rmlist(*pp, dlist);
                        else
                              pp++;
                  }
            }
            fclose(fp);
      }

      prtbad(dlist, "distribution", curhdr);
}



/*
 * Check the newsgroups on the Newsgroups: and Followup-To: lines
 * for validity.  We do both at once so that we only have to scan
 * the active file once.
 */

cknewsgroups() {
      FILE *actfp;
      char *p;
      int didng = 0;
      char **pp;

      if (nglist[0] == NULL && follist[0] == NULL)
            return;
      bcopy((char *)nglist, (char *)nlist, sizeof nlist);
      sprintf(bfr, "%s/active", LIB);
      actfp = ckfopen(bfr, "r");
      while ((nlist[0] || follist[0]) && fgets(bfr, LBUFLEN, actfp)) {
            if ((p = index(bfr, ' ')) == NULL && (p = index(bfr, '\n')) == NULL)
                  continue;
            *p = '\0';
            if (rmlist(bfr, nlist) >= 0)
                  didng = 1;
            rmlist(bfr, follist);
      }
      fclose(actfp);

      /* Not all newsgroups on followup need be legal */
      if (nlist[0] && (!hdrline[HTREFERENCES] || !didng))
            prtbad(nlist, "newsgroup", hdrline[HTNEWSGROUPS]);
      else {
            /* check for moderated groups */
            for (pp = nglist ; *pp ; pp++) {
                  if (hdrline[HTAPPROVED] == NULL && ngmatch(*pp, MODGROUPS)) {
                        getmod(*pp);
                        break;
                  }
            }
      }
      prtbad(follist, "newsgroup", hdrline[HTFOLLOWUPTO]);
}


prtbad(pp, what, hl)
      char **pp;
      char *what;
      struct hline *hl;
      {
      hlselect(hl);
      while (*pp)
            synerr("Bad %s %s", what, *pp++);
}



/*
 * Remove an entry from an array of character strings.
 */

rmlist(entry, list)
      char *entry, **list;
      {
      register char **pp;
      register char **qq;
      int retval;

      qq = list;
      retval = -1;
      for (pp = list ; *pp ; pp++) {
            if (!equal(*pp, entry))
                  *qq++ = *pp;
            else
                  retval = 0;		/* removed one */
      }
      *qq = NULL;
      return retval;
}



/*
 * Check "Approved:" line.  Currently, we require an address which must
 * include an at sign and which cannot be enclosed in angle brackets.
 * An optional real name in parenthesis is allowed, but not other comments.
 */

ckapproved() {
      char *start = scanp;
      extern char *beglocal, *endlocal, *begdomain;

      if (*scanp == '(' || *scanp == ',' || *scanp == '\n')
bad:        jsynerr("Illegal address");
      parseaddr();
      if (beglocal != start || *endlocal != '@' || begdomain != endlocal + 1)
            goto bad;
      skipbl();
      if (*scanp)
            goto bad;
}



/*
 * Parse a comma separated list into its components.
 */

commalist() {
      char **pp;
      char *p;

      pp = nlist;
      for (;;) {
            skipws(" \t\n,");
            if (*scanp == '\0')
                  break;
            p = scanp;
            scnchr(" \t\n,(");
            if (pp >= nlist + MAXGRPS)
                  jsynerr("list too long");
            *pp++ = getval(p, scanp, NULL);
      }
      *pp = NULL;
}



outstr(str)
      char *str;
      {
      strcpy(outp, str);
      outp += strlen(str);
}



rmdup(list)
      char **list;
      {
      register char **pp, **qq;

      while (*list) {
            for (pp = qq = list + 1 ; *pp ; pp++)
                  if (!equal(*pp, *list))
                        *qq++ = *pp;
            *qq = NULL;
            list++;
      }
}



/*
 * Set up a given header for parsing.
 */

hlselect(hp)
      struct hline *hp;
      {
      curhdr = hp;
      scanp = hp->body;
}


hlzap(type)
      int type;
      {
      register struct hline *hp, **prev;

      prev = &hfirst;
      while ((hp = *prev) != NULL) {
            if (hp->type == type)
                  *prev = hp->next;	/* delete line */
            else
                  prev = &hp->next;
      }
      hdrline[type] = NULL;
      hlast = NULL;
      for (hp = hfirst ; hp ; hp = hp->next)
            hlast = hp;
}



/*
 * Figure out the moderator of a group.
 */

getmod(newsgroup)
      char *newsgroup;
      {
      FILE *fp;
      char *p;
      char *q;
      char s[BUFLEN];
      FILE *popen();

      sprintf(bfr, "%s/moderators", LIB);
      fp = ckfopen(bfr, "r");
      for (;;) {
            if (fgets(bfr, LBUFLEN, fp) == NULL)
                  goto notfound;
            if ((p = strpbrk(bfr, " \t")) == NULL)
                  continue;		/* "Can't happen" */
            *p++ = '\0';
            if (equal(bfr, newsgroup))
                  break;
      }
      fclose(fp);
      while (*p == ' ' || *p == '\t')
            p++;
      nstrip(p);
      moderator = savestr(p);
      return;

notfound:
      fclose(fp);

      /* try to use return path from old article */
      sprintf(bfr, "%s/active", LIB);
      fp = ckfopen(bfr, "r");
      while (fgets(bfr, LBUFLEN, fp) != NULL) {
            p = index(bfr, ' ');
            *p++ = '\0';
            if (equal(bfr, newsgroup))
                  break;
      }
      fclose(fp);
      if ((q = strpbrk(p, " \t\n")) != NULL)
            *q = '\0';
      scopyn(p, s, 32);
      dirname(newsgroup, bfr);
      sprintf(bfr + strlen(bfr), "/%ld", atol(s));
      printf("postnm: trying to find moderator for %s from %s\n", newsgroup, bfr);
      if ((fp = fopen(bfr, "r")) != NULL) {
            while (fgets(bfr, LBUFLEN, fp) != NULL && bfr[0] != '\n') {
#ifdef notdef /* "From:" line has same info */
                  if (prefix(bfr, "Path:")) {
                        scanp = bfr + 5;
                        s[0] = '\0';
                        for (;;) {
                              scchr("!:@^% \t\n");
                              p = scanp;
                              scnchr("!:@^% \t\n");
                              q = scanp;
                              scchr("!:@^% \t\n");
                              if (*scanp)
                                    getval(p, q, s);
                              else
                                    break;
                        }
                        if (s[0] == '\0')
                              continue;
                        if (index(s, '.') == NULL)
                              strcat(s, ".UUCP");
                        *q = '\0';
                        sprintf(bfr, "%s@%s", p, s);
                        moderator = savestr(bfr);
                        printf("postnm: assuming moderator for %s is %s\n", newsgroup, bfr);
                  }
#endif
                  if (prefix(bfr, "From:")) {
                        getaddr(bfr + 5, s);
                        moderator = savestr(s);
                  }
                  else if (prefix(bfr, "Approved:")) {
                        getaddr(bfr + 9, s);
                        moderator = savestr(s);
                  }
            }
      }

      if (fp != NULL)
            fclose(fp);
      if (! moderator)
            synerr("Can\'t find moderator for newsgroup %s", newsgroup);
#ifdef NOTIFY
      sprintf(bfr, "mail \'%s\'", NOTIFY);
#else
      sprintf(bfr, "mail \'%s\'", ADMIN);
#endif
      if ((fp = popen(bfr, "w")) == NULL)
            fprintf(stderr, "postnm: can\'t send mail to administrator\n");
      else {
            fprintf(fp, "Subject: missing moderator entry for %s\n\n", newsgroup);
            fprintf(fp, "From: The postnm program.\n");
            fprintf(fp, "\nThere is no entry for %s in %s/moderators\n",
                    newsgroup, LIB);
            if (moderator)
                  fprintf(fp, "The moderator is probably %s\n", moderator);
            pclose(fp);
      }
}
