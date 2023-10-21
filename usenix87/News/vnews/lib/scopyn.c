scopyn(from, to, n)
      register char *from, *to ;
      register int n ;
      {
      while (--n > 0 && (*to++ = *from++) != '\0') ;
      *to = '\0' ;
}
