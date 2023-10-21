/*
 * Hashing function.
 * First arg is the string, second is the number of elements in the table.
 * Currently, we use a CRC.
 */

int
hash(string, nelem)
      char *string ;
      {
      register unsigned char *p ;
      register int h ;

      h = 0 ;
      for (p = string ; *p ; ) {
            h = (long)(((long)h << 8) + *p++) % nelem ;
      }
      return h ;
}
