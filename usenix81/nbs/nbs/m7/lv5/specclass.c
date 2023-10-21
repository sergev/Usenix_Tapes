/*  specclass.c
*
*     This routine matches the patterns "?1",  "?2", "?3", "?4", "?5" and "?6".
*     The code passed distinguishes between which pattern it is.
*
*     Arguments: code - value of '1', '2', '3', '4', '5' or '6'.
*                charac - the input character to be checked for a match.
*
*     UE's: None.
*
*                             Programmer: G. Skillman
*/
specclass(code, charac)

char   code, charac;
{
       extern int      YES, NO;
       extern char     EOS, LT;

       if (charac == EOS || charac == LT)
               return(NO);

       switch (code)
       {
               case '1': /* Any character. */
                    return(YES);
                    break;

               case '2': /* Any alpha-numeric character. */
                    if ((charac >= 'A' && charac <= 'Z') ||
                        (charac >= 'a' && charac <= 'z') ||
                        (charac >= '0' && charac <= '9'))
                             return(YES);
                    break;

               case '3': /* Any alphabetic character. */
                    if ((charac >= 'A' && charac <= 'Z') ||
                        (charac >= 'a' && charac <= 'z'))   
                             return(YES);
                    break;

               case '4': /* Any upper case letter. */
                    if (charac >= 'A' && charac <= 'Z')
                             return(YES);
                    break;

               case '5': /* Any lower case letter. */
                    if (charac >= 'a' && charac <= 'z')
                             return(YES);
                    break;

               case '6': /* Any digit. */
                    if (charac >= '0' && charac <= '9')
                             return(YES);
                    break;

       }
       return(NO);
}
