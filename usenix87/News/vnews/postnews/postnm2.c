/*
 * Pass one:  read in the article.
 *
 * Copyright 1984 by Kenneth Almquist.  All rights reserved.
 *     Permission is granted for anyone to use and distribute, but not
 *     sell, this program provided that this copyright notice is retained.
 */

#include "postnm.h"

/* values of ht_legal */
#define DITTO	0		/* alias for previous name */
#define LEGAL	1		/* normal field */
#define ONCE	2		/* field may only occur once */
#define ILLEGAL	3		/* user not allowed to specify this field */

struct htype {
      char *ht_name;		/* name of header field */
      int   ht_legal;		/* whether field is legal */
      int   ht_type;		/* field type number */
}
htype[] = {
      "To",		LEGAL,	HTTO,
      "Cc",		LEGAL,	HTCC,
      "Bcc",		LEGAL,	HTBCC,
      "Newsgroups",	ONCE,	HTNEWSGROUPS,
      "Newsgroup",	DITTO,	HTNEWSGROUPS,
      "In-Reply-To",	ONCE,	HTINREPLYTO,
      "References",	ONCE,	HTREFERENCES,
      "Subject",	ONCE,	HTSUBJECT,
      "Expires",	ONCE,	HTEXPIRES,
      "From",		ONCE,	HTFROM,
      "Reply-To",	ONCE,	HTREPLYTO,
      "Followup-To",	ONCE,	HTFOLLOWUPTO,
      "Distribution",	ONCE,	HTDIST,
      "Dist",		DITTO,	HTDIST,
      "Keywords",	LEGAL,	HTKEYWORDS,
      "Control",	ONCE,	HTCONTROL,
      "Organization",	ONCE,	HTORGANIZATION,
      "Summary",	ONCE,	HTSUMMARY,
      "Approved",	ONCE,	HTAPPROVED,
      "Comments",	LEGAL,	HTCOMMENTS,
      "Message-Id",	ONCE,	HTMESSAGEID,
      "Command",	ONCE,	HTCOMMAND,
      "Article-I.d.",	ILLEGAL,HTUNKNOWN,
      "Sender",		ILLEGAL,HTUNKNOWN,
      "Resent-Sender",	ILLEGAL,HTUNKNOWN,
      "Path",		ILLEGAL,HTUNKNOWN,
      "Received",	ILLEGAL,HTUNKNOWN,
      NULL,		LEGAL,	HTUNKNOWN
};

/*
 * Read in the header.
 */

readheader() {
      setexit();
      while (gethline(infp) != EOF) {
            prochdr(bfr, 0);
      }
}



prochdr(val, faked)
      char *val;
      {
      char htname[64];

      curhdr = ckmalloc(sizeof *curhdr);
      curhdr->linno = !faked? linno : 0;
      curhdr->text = savestr(val);
      curhdr->fixed = NULL;
      scanp = curhdr->text;		/* scan input line */
      if (scnchr(" \t\n:") == 0)
            jsynerr("Illegal header line type");
      if (scanp - curhdr->text > 63)
            jsynerr("Header line type too long");
      getval(curhdr->text, scanp, htname);
      fixcase(htname);
      skipbl();
      if (*scanp++ != ':')
            jsynerr("No colon on header line\nDid you remember to leave a blank line after the article header?");
      skipbl();
      curhdr->body = scanp;
      if (*scanp == '\0')
            curhdr->body--;

      gettype(htname, curhdr);

      appheader(curhdr, hlast);
}



gettype(htname, hp)
      char *htname;
      struct hline *hp;
      {
      register struct htype *htp;

      for (htp = htype ; htp->ht_name != NULL && !equal(htp->ht_name, htname) ; htp++);
      if (htp->ht_name == NULL) {
            for (htp = htype ;
                 htp->ht_name && (htp->ht_legal == ILLEGAL || !misspells(htp->ht_name, htname)) ;
                 htp++);
            if (htp->ht_name != NULL)
                  synerr("You misspelled \"%s\"", htp->ht_name);
      }
      while (htp->ht_legal == DITTO)
            htp--;
      hp->type = htp->ht_type;
      if (htp->ht_name == NULL) {
            hp->hdrname = savestr(htname);
      } else {
            hp->hdrname = htp->ht_name;
            if (htp->ht_legal == ILLEGAL)
                  jsynerr("You cannot include a \"%s:\" header", htname);
            else if (htp->ht_legal == ONCE && hdrline[hp->type])
                  jsynerr("Only one \"%s:\" header line is permitted", htname);
            hdrline[hp->type] = hp;
      }
}


/*
 * Get the correct combination of upper case and lower case in a header
 * line name.  The first character and any character immediatly following
 * a minus sign is capitalized.  Everything else to lower case.
 */

fixcase(p)
      register char *p;
      {
      int flag = 1;

      while (*p) {
            if (*p == '-')
                  flag = 1;
            else if (flag) {
                  flag = 0;
                  if (islower(*p))
                        *p = toupper(*p);
            } else {
                  if (isupper(*p))
                        *p = tolower(*p);
            }
            p++;
      }
}



misspells(word, incorrect)
      char *word, *incorrect;
      {
      int i = strlen(word), j = strlen(incorrect);
      register char *p = word, *q = incorrect;

      if (j != i && j != i + 1 && j != i - 1)
            return 0;
      while (*p == *q && *p)
            p++, q++;
      if (i > j) {
            if (equal(p + 1, q))  return 1;
      } else if (i < j) {
            if (equal(p, q + 1))  return 1;
      } else {
            if (equal(p + 1, q + 1))  return 1;
            if (p[0] == q[1] && p[1] == q[0] && equal(p + 2, q + 2))
                  return 1;
      }
      return 0;
}




/*
 * Insert a header after the given header, or at the beginning of
 * the list if NULL is given.
 */

appheader(hp, after)
      struct hline *hp, *after;
      {
      struct hline **hpp;

      if (after)
            hpp = &after->next;
      else
            hpp = &hfirst;
      hp->next = *hpp;
      *hpp = hp;
      if (hlast == after)
            hlast = hp;
}




/*
 * Read a header line into bfr, handling continuations.
 */


gethline(fp)
      register FILE *fp;
      {
      char *p;
      register char *q;
      int c;
      static int nextlinno = 1;

      linno = nextlinno;
      if (feof(fp))
            return EOF;
      p = bfr;
      do {
            if (fgets(p, bfr + LBUFLEN - p, fp) == NULL)
                  return EOF;
            if (bfr[0] == '\n')
                  return EOF;
            for (q = p ; *q != '\0' && *q != '\n' ; q++) {
                  if ((*q < ' ' && *q != '\t') || *q > '~') {
                        fprintf(stderr, "postnm: line %d: illegal character (octal %o)\n",
                                nextlinno, *(unsigned char *)q);
                        nerrors++;
                  }
            }
            if (*q == '\0') {
                  if (strlen(bfr) != LBUFLEN - 1)
                        xerror("line %d: EOF in middle of header line", nextlinno);
                  else
                        xerror("line %d: Line too long", nextlinno);
            }
            nextlinno++;
            p = q + 1;
      } while (ungetc(c = getc(fp), fp), c == ' ' || c == '\t');
      return 0;
}



/*
 * Copy the body of the article to the mail and/or news file.  We have to
 * check for:
 *	1) Control characters.
 *	2) Lines consisting of a single '.'
 *	3) Trailing blank lines, which we silently delete.
 *	4) Article which might trigger line eater bug.
 */

addbody(in)
      register FILE *in;
      {
      register int c;
      register FILE *mail = mailfp, *news = newsfp;
      int nlcount = 1;		/* force initial blank line */
      int dotflag = 0;		/* test for line containing single dot */
      int first = 1;		/* avoid line eater */

      linno++;
      if (!feof(in))
         while ((c = getc(in)) != EOF) {
            if (c == '\n') {
                  if (dotflag)
                        xerror("line %d: some versions of mail and news choke on lines consisting of a single dot", linno - nlcount);
                  linno++;
                  nlcount++;
            } else if (!isprint(c) && !isspace(c) && c != '\b') {
                  if (in == infp)
                        fprintf(stderr, "line %d: illegal character (octal %o)\n", linno, c);
                  else
                        fprintf(stderr, "illegal character in signature file (octal %o)\n", c);
                  nerrors++;
            } else {
                  if (nlcount) {
                        if (first) {
                              first = 0;
                              if ((c == '\t' || c == ' ') && nlcount == 1 && news && in == infp)
                                    putc('\n', news);
                        }
                        if (c == '.')
                              dotflag++;
                        do {
                              if (mail)
                                    putc('\n', mail);
                              if (news)
                                    putc('\n', news);
                        } while (--nlcount > 0);
                  } else
                        dotflag = 0;
                  if (mail)
                        putc(c, mail);
                  if (news)
                        putc(c, news);
            }
      }
      if (mail)
            putc('\n', mail);
      if (news)
            putc('\n', news);
      if (mail) {
            fflush(mail);
            if (ferror(mail))
                  xerror("write error on temp file");
      }
      if (news) {
            fflush(news);
            if (ferror(news))
                  xerror("write error on temp file");
      }
}
