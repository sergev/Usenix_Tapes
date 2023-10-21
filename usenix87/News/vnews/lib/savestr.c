/*
 * Make a copy of a string
 */

char
*savestr(s)
      char *s ;
      {
      char *p ;
      char *ckmalloc() ;

      p = ckmalloc(strlen(s) + 1) ;
      strcpy(p, s) ;
      return p ;
}
