#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqhighlighting                                            */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqhighlighting(segname,highlght)
   char *segname;
   int *highlght;
   {
   char *funcname;
   int errnum;

   funcname = "inqhighlighting";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 *highlght = segptr->high1lght;
	 return(0);
	 }
      if (segptr->type == EMPTY)
	 {
	 errnum = 29;
	 errhand(funcname,errnum);
	 return(1);
	 }
      }
   errnum = 29;
   errhand(funcname,errnum);
   return(1);
   }


