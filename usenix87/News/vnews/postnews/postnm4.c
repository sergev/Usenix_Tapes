/*
 * Pass 3:  generate the news and mail headers.
 *
 * Copyright 1984 by Kenneth Almquist.  All rights reserved.
 *     Permission is granted for anyone to use and distribute, but not
 *     sell, this program provided that this copyright notice is retained.
 */

#include "postnm.h"


/*
 * Write out a header.
 */
gennewshdr(fp)
      FILE *fp;
      {
      int didmod = 0;

      for (curhdr = hfirst ; curhdr ; curhdr = curhdr->next) {
            if (setexit())  continue;
            switch (curhdr->type) {
            case HTFROM:
            case HTREPLYTO:
                  newsaddr(curhdr);
                  break;

            case HTTO:
                  fputs("To: ", fp);
                  if (moderator && ! didmod)
                        fprintf(fp, "%s (The Moderator), ", moderator);
                  fputs(curhdr->body, fp);
                  didmod = 1;
                  break;

            default:
                  if (curhdr->fixed)
                        fprintf(fp, "%s: %s", curhdr->hdrname, curhdr->fixed);
                  else
                        fprintf(fp, "%s: %s", curhdr->hdrname, curhdr->body);
                  break;
            }
      }
      if (moderator && !didmod)
            fprintf(fp, "To: %s (The Moderator)\n", moderator);
}


genmailhdr(fp)
      FILE *fp;
      {
      for (curhdr = hfirst ; curhdr ; curhdr = curhdr->next) {
            if (setexit())  continue;
            switch (curhdr->type) {
            case HTREFERENCES:
                  if (hdrline[HTINREPLYTO] == NULL) {
                        register char *p, *q;
                        p = curhdr->fixed? curhdr->fixed : curhdr->body;
                        if ((p = rindex(p, '<')) != NULL
                         && (q = index(p, '>')) != NULL) {
                              fprintf(fp, "In-reply-to: USENET article %s\n",
                                    getval(p, q + 1, bfr));
                        }
                  }
                  break;

            case HTEXPIRES:
                  fputs(curhdr->text, fp);
                  break;

            default:
                  if (curhdr->fixed)
                        fprintf(fp, "%s: %s", curhdr->hdrname, curhdr->fixed);
                  else
                        fputs(curhdr->text, fp);
                  break;
            }
      }
}



newsaddr(hp)
      struct hline *hp;
      {
      char domain[NAMELEN], local[NAMELEN];
      extern char *begreal, *endreal;		/* real name */
      extern char *beglocal, *endlocal;		/* local part */
      extern char *begdomain, *enddomain;	/* domain part */

      hlselect(hp);
      skipws(" \t\n,");
      parseaddr();
      skipws(" \t\n,");
      if (*scanp != '\0') {
            synerr("only one address permitted on \"%s:\" line", hp->hdrname);
            return;
      }
      if (begdomain)
            getval(begdomain, enddomain, domain);
      else
            sprintf(domain, "%s%s", FULLSYSNAME, MYDOMAIN);
      getval(beglocal, endlocal, local);
      fprintf(newsfp, "%s: %s@%s", hp->hdrname, local, domain);
      if (begreal) {
            getval(begreal, endreal, bfr);
            fprintf(newsfp, " (%s)", bfr);
      }
      putc('\n', newsfp);
      if (equal(hp->hdrname, "From")
       && (index(local, '!') || index(domain, '!') || index(bfr, '!')))
            synerr("Exclamation points on From: line break inews");
}
