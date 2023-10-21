#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqsc2                                                     */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqsc2(segname,xscale,yscale)
   char *segname;
   float *xscale,*yscale;
   {
   char *funcname;
   int errnum;

   funcname = "inqsc2";
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
	 *xscale = segptr->scale1[0];
	 *yscale = segptr->scale1[1];
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


