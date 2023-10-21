#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqro2                                                     */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqro2(segname,angle)
   char *segname;
   float *angle;
   {
   char *funcname;
   int errnum;

   funcname = "inqro2";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if(segptr->type != XFORM2)
	    {
	    errnum = 30;
	    errhand(funcname,errnum);
	    return(2);
	    }
	 *angle = segptr->rotate1;
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

