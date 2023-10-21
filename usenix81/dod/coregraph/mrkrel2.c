#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: mrkrel2                                                    */
/*                                                                          */
/*     PURPOSE: THE CP IS UPDATED TO A RELATIVE (x,y) AND THEN THE          */
/*              SYSTEM-DEFINED MARKER 'n',CENTERED AT THE TRANSFORMED       */
/*              POSITION OF THE CP, IS CREATED.                             */
/*                                                                          */
/****************************************************************************/

mrkrel2(dx,dy,n)
   float dx,dy;
   int n;
   {
   char *funcname;
   int errnum;

   funcname = "mrkrel2";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }
   if (n < 1)
      {
      errnum = 25;
      errhand(funcname,errnum);
      return(2);
      }
   if ((n > stdmarkr) && (n < MINASCII))
      {
      errnum = 25;
      errhand(funcname,errnum);
      /*** do not return, merely report and replace with a question mark ***/
      n = QUESTION;
      }
   if (n > MAXASCII)
      {
      errnum = 25;
      errhand(funcname,errnum);
      /*** do not return, merely report and replace with a question mark ***/
      n = QUESTION;
      }

   return(mrkabs2(cp[0] + dx,cp[1] + dy,n));
   }















