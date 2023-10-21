
/**********************************************************************\
* Written by: Andrew Gaynor (gaynor@topaz.rutgers.edu)                 *
*                                                                      *
* License (Sure it's informal, vague, and has lots of loopholes, but   *
*          the meaning is clear.  I don't even know if it's binding,   *
*          but I give the code in good faith, so accept it likewise.)  *
*                                                                      *
* Hi!  About this code, here...  It's as good as yours, and you may do *
* what you wish with it, as long as you play by these rules:           *
*   (1) Everything that's here now must remain here.  You can COMMENT  *
*       things out, fine, but don't SNIP them out.                     * 
*   (2) All modifications are to be commented as being done by the     *
*       modifier.  That is, if you make a change, put YOUR name on it. *
*       For example, look at how I forgot and added negative numbers.  *
*   (3) I disclaim all responsibility for the consequences of any use  *
*       of this code.  If you use it, I'm not to blame for anything    *
*       that happens.                                                  *
\**********************************************************************/


/* I just hate not seeing it there :-) */
#define then


/**********************************************************************\
* num_in_rad takes a string of an integer in a given radix, in the     *
* form "<num_in_rad>#<rad>", and returns the equivalent (long) integer *
* in decimal.  Possible radixes are 2, 3, ..., 9, A, B, ..., Z.        * 
* Possible digits in a radix r are 0, 1, ..., predecessor (r).  This   *
* thing does no error checking.  I almost forgot: the #<rad> part may  *
* be ommitted, and the radix defaults to 10 (10 == A).                 *
\**********************************************************************/
/**************** gaynor@topaz.rutgers.edu: ADDITION 1 ****************\
* I forgot to handle negative numbers!  They have been added, so the   *
* first thing in <num_in_rad> above may be a minus sign.               *
\**********************************************************************/
long int num_in_rad (num_with_rad_str)

  char *num_with_rad_str;

  {
    /* num_str is the string of the number in radix, the first piece */
    /* of num_with_rad_str - point num_str where num_with_rad_str    */
    /* points - it's really the same thing, once that pesky '#' is   */
    /* replaced with '\0' (if there at all)                          */
    char *num_str = num_with_rad_str;

    int rad_int = 0;  /* the radix as an integer */

    int num_int = 0;  /* the number as an integer */

    /* 1 */ int sign = 1;

    /* 1 */ /* if the number is negative, ... */
    /* 1 */ if (*num_with_rad_str == '-')
    /* 1 */   then
    /* 1 */     {
    /* 1 */       /* record the fact and skip the sign */
    /* 1 */       sign = -1;
    /* 1 */       ++num_str;
    /* 1 */       ++num_with_rad_str;
    /* 1 */     };

    /* while num_with_rad_str doesn't point */
    /* at '#' or '\0', increment it         */
    while (*num_with_rad_str != '#'  &&  *num_with_rad_str)
      ++num_with_rad_str;

    /* if num_with_rad_str points at '#' (ie not '\0') ... */
    if (*num_with_rad_str)
      then /* '#' */
        {
          /* set the terminator for num_str by replacing the '#' with */
          /* '\0' - increment num_with_rad_str to point at the radix  */
          *(num_with_rad_str++) = '\0';

          /* if it points to a digit (decimal), set rad_int to the */
          /* conversion of that digit (in radix) to an integer     */
          /* (decimal) - otherwise, it must be letter-digit, so    */
          /* convert that letter to the proper integer (decimal)   */
          /* (ie A = 10, B = 11, ...)                              */
          rad_int = (int) ((*num_with_rad_str <= '9') ?
                      (*num_with_rad_str - '0') :
                      (*num_with_rad_str - 'A' + 10));

          /* convert num_str to an integer (decimal) - for each digit, */
          /* increase the power by multiplying by the radix and add    */
          /* the digit.                                                */

          /* while num_str points at a digit (in radix) ... */
          while (*num_str)

            /* num_int = rad_int * num_int (increase power)       */
            /*             + the conversion of the next digit (in */
            /*               radix) to an integer (decimal) (as   */
            /*               before)                              */
            num_int = rad_int * num_int
                        + (int) ((*num_str <= '9') ?
                            (*(num_str++) - '0') :
                            (*(num_str++) - 'A' + (char) 10));

          /* restore '#' that was replaced before (with '\0', */
          /* remember?), to act as a terminator for num_str.  */
          *num_str = '#';
        }
      else /* '\0' */
        /* it's assumed to be decimal if no radix supplied */
        while (*num_str)
          num_int = 10 * num_int
                      + (int) (*(num_str++) - '0');

    /* 1 */ /* The sign, below... */
    return (sign * num_int);
  }
