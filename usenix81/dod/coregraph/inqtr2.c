#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqtr2                                                     */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqtr2(segname,dx,dy)
   char *segname;
   float *dx,*dy;
   {
   char *funcname;
   int errnum;

   funcname = "inqtr2";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if(segptr->type == NORETAIN || segptr->type == RETAIN)
	    {
	    errnum = 30;
	    errhand(funcname,errnum);
	    return(2);
	    }
	 *dx = segptr->trans1lat[0];
	 *dy = segptr->trans1lat[1];
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


