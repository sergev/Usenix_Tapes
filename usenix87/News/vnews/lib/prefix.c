int
prefix(full, pref)
      register char *full, *pref ;
      {
      register char c ;

      while ((c = *pref++) != '\0')
            if (*full++ != c)
                  return 0 ;
      return 1 ;
}
