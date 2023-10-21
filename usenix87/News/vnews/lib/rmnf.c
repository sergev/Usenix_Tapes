/*
 * Zap the notesfile indicator.
 */

#define equal(s1, s2)	(strcmp(s1, s2) == 0)

rmnf(title)
      char *title ;
      {
      register char *p ;

      p = title + strlen(title) - 7 ;
      if (p > title && equal(p, " - (nf)")) {
            *p = '\0' ;
            return 1 ;
      }
      return 0 ;
}
