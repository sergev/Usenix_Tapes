/*
 * gettok -- get token from a String object
 */
/*
 *  Author:     S.M. Orlow
 *              Systex,Inc.
 *              Beltsville, MD 20705
 *              301-474-0111
 *
 *  Contractor: K.E. Gorlen
 *              Computer Systems Laboratory
 *              Rm. 2017, Bld 12A
 *              National Institutes of Health
 *              Bethesda, MD 20205
 *              (301)-496-5361
 *
 * Modification History:
 */

#include "gettok.h"

/* is_char_of -- return 1 when char is in string */
int is_char_of(char ch,char* q) {
      while ( *q != '\0' ) 
        if ( ch == *(q++) ) return 1;
      return 0;
    }/* end is_char_of */

static char* lastp = 0;

char* gettok(char* inbuf,char* tmnl) {
   char* retp;

   if ( inbuf != 0 ) {
     lastp = inbuf;
     retp = inbuf;
     }
    else
     retp = lastp;

    while ( *lastp != '\0' ) { /* find next terminal char */
      if ( is_char_of(*lastp,tmnl) ) break;
      ++lastp;
      };/* end while */
    if ( *lastp != '\0' ) {
      *(lastp++) = '\0';
      while ( *lastp != '\0' ) {/* skip all adjacent terminal chars */
          if ( !is_char_of(*lastp,tmnl) ) break;
	  ++lastp;
	  };/*end while*/
      };/* end if*/
   return retp;
}/* end gettok */
