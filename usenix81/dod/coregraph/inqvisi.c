#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqvisibility                                              */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqvisibility(segname,visbilty)
   char *segname;
   int *visbilty;
   {
   char *funcname;
   int errnum;

   funcname = "inqvisibility";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 *visbilty = segptr->visbil1ty;
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


