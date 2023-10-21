#include <stdio.h>

/*
 * hfgets is like fgets, but deals with continuation lines.
 * It also ensures that even if a line that is too long is
 * received, the remainder of the line is thrown away
 * instead of treated like a second line.
 */

char *
hfgets(buf, len, fp)
      char *buf;
      int len;
      register FILE *fp;
      {
      register int c;
      register char *cp;

      cp = fgets(buf, len, fp);
      if (cp == NULL || *cp == '\n')
            return cp;

      c = strlen(cp);
      len -= c;
      cp += c - 1;
      if (*cp != '\n') {
            /* Line too long - part read didn't fit into a newline */
            while ((c = getc(fp)) != '\n' && c != EOF);
      }
      while ((c = getc(fp)) == ' ' || c == '\t') {	/* for each cont line */
            /* Continuation line. */
            while ((c = getc(fp)) == ' ' || c == '\t');	/* skip white space */
            if (--len > 0)
                  *cp++ = ' ';
            do {
                  if (--len > 0)
                        *cp++ = c ;
            } while ((c = getc(fp)) != '\n' && c != EOF) ;
      }
      *cp++ = '\n';
      *cp++ = '\0';
      if (c != EOF)		/* Bug in Apollo UN*X */
            ungetc(c, fp);	/* push back first char of next header */
      return buf;
}
