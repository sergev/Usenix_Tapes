char *
ckmalloc(nbytes)
      unsigned nbytes ;
      {
      register char *p ;
      char *malloc() ;

      if ((p = malloc(nbytes)) == (char *)0)
            xerror("out of space") ;
      return p ;
}
