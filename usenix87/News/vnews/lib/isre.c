/*
 * Check for "Re: " at beginning of string.
 */

isre(t)
      register char *t ;
      {
      if (t == 0)		/* delete this later */
            return 0 ;
      if (strncmp(t, "Re:", 3) == 0
       || strncmp(t, "re:", 3) == 0
       || strncmp(t, "RE:", 3) == 0)
            return 1 ;
      return 0 ;
}
