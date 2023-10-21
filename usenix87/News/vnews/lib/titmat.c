/*
 * Match title.
 */

titmat(title, titlist)
      char *title;
      register char **titlist;
      {
      char *sindex();

      while (*titlist) {
            if (sindex(title, *titlist))
                  return 1 ;
            titlist++ ;
      }
      return 0 ;
}


char *
sindex(s1, s2)
      register char *s1 ;		/* string being searched */
      register char *s2 ;		/* string being scanned for */
      {

      if (*s2 == '\0')
            return s1 ;
      while (*s1) {
            if (*s1 == *s2 && prefix(s1, s2))
                  return s1 ;
            s1++ ;
      }
      return 0 ;
}
