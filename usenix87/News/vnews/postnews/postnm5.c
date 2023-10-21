/*
 * STRING SCANNING ROUTINES:
 *	These routines act on a subject string which is pointed to by scanp.
 *
 * scchr	Skip over characters contained in string.
 * scnchr	Skip over characters not in string.
 * skipbl	Skip over spaces, tabs, and newlines.
 * scomment	Skip to the end of a comment.
 * scqnchr	Like scnchr, but skips over any char if inside quotes.
 * skipsw	Skip over spaces, tabs, newlines, and comments.
 * parseaddr	Parse an address, setting up pointers to various parts.
 * getval	Return the characters between two pointers.
 *
 * Copyright 1984 by Kenneth Almquist.  All rights reserved.
 *     Permission is granted for anyone to use and distribute, but not
 *     sell, this program provided that this copyright notice is retained.
 */

#include "postnm.h"

char *scanp;

scchr(chars)
      char *chars;
      {
      int flag = 0;

      while (*scanp && index(chars, *scanp)) {
            flag++;
            scanp++;
      }
      return flag;
}


scnchr(chars)
      char *chars;
      {
      int flag = 0;

      while (index(chars, *scanp) == NULL) {
            flag++;
            scanp++;
      }
      return flag;
}



skipbl() {
      register char c;

      while ((c = *scanp) == ' ' || c == '\t' || c == '\n')
            scanp++;
}



scomment() {
      int nest;
      register char *p;

      if (*(p = scanp) != '(')
            return 0;
      nest = 0;
      for (;;) {
            if (*p == '(')
                  nest++;
            else if (*p == ')') {
                  if (--nest == 0)
                        break;
            } else {
                  if (*p == '\\')
                        p++;
                  if (*p == '\0')
                        jsynerr("Nonterminated comment");
            }
            p++;
      }
      scanp = p;
      return 1;
}


scqnchr(chars)
      char *chars;
      {
      register char *p = scanp;

      for (;;) {
            if (*p == '"') {
                  p++;
                  while (*p != '"') {
                        if (*p == '\\')
                              p++;
                        if (*p++ == '\0')
                              jsynerr("Nonterminated string");
                  }
            } else {
                  if (index(chars, *p))
                        break;
            }
            p++;
      }
      if (p == scanp)
            return 0;
      else {
            scanp = p;
            return 1;
      }
}



skipws(chars)
      char *chars;
      {
      for (;;) {
            if (*scanp == '(')
                  scomment();
            else if (*scanp == '\0' || index(chars, *scanp) == NULL)
                  break;
            scanp++;
      }
}



/*
 * Parse an address.
 */

char *begreal, *endreal;		/* real name */
char *begaddr, *endaddr;		/* machine address */
char *beglocal, *endlocal;		/* local part */
char *begdomain, *enddomain;		/* domain part */

parseaddr() {
      char *startp = scanp;
      char *p;

      begreal = begdomain = NULL;
      if (*scanp == '.')
syntax:     jsynerr("Illegal address syntax");
      begaddr = beglocal = scanp;
      if (*scanp == '<') {
routeaddr:
            scanp++;
            skipbl();
            begaddr = beglocal = scanp;
            scqnchr(":>");
            if (*scanp == ':') {
                  scanp++;
                  skipbl();
                  beglocal = scanp;
                  scqnchr(">");
            }
            if (*scanp != '>')
                  goto syntax;
            scanp = beglocal;
            scqnchr("@>");
            endlocal = scanp;
            while (index(" \t\n", *(endlocal - 1)))
                  endlocal--;
            if (endlocal <= beglocal)
                  goto syntax;
            if (*scanp == '@') {
                  scanp++;
                  skipbl();
                  begdomain = scanp;
                  scqnchr(">");
                  enddomain = scanp;
                  while (index(" \t\n", *(enddomain - 1)))
                        enddomain--;
                  if (enddomain <= begdomain)
                        goto syntax;
            }
            if (*scanp != '>')
                  goto syntax;
            scanp++;
      } else {
            scqnchr(" \t\n@(,<");
            if (scanp == startp)
                  jsynerr("Program bug");
            endlocal = scanp;
            skipbl();
            if (*scanp == '@') {
                  scanp++;
                  skipbl();
                  begdomain = scanp;
                  if (scnchr(" \t\n(,<") == 0)
                        goto syntax;
                  enddomain = scanp;
                  if (enddomain[-1] == '.')
                        goto syntax;
            } else {
                  do p = scanp, skipbl();
                     while (scqnchr(" \t\n@(,<"));
                  if (*scanp != '<') {
                        scanp = endlocal;
                  } else {
                        begreal = beglocal, endreal = p;
                        goto routeaddr;
                  }
            }
      }
      if (begdomain)
            endaddr = enddomain;
      else
            endaddr = endlocal;
      if (begreal == NULL) {
            p = scanp;
            skipbl();
            if (*scanp != '(') {
                  scanp = p;
            } else {
                  begreal = scanp + 1;
                  scomment();
                  endreal = scanp;
                  scanp++;		/* skip over ')' */
            }
      }
}



char *
getval(p, q, buf)
      char *p, *q, *buf;
      {

      if (buf == NULL)
            buf = ckmalloc(q - p + 1);
      bcopy(p, buf, q - p);
      buf[q - p] = '\0';
      return buf;
}
