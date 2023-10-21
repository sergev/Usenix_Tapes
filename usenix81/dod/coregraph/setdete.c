#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setdetecatbility                                           */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setdetectability(segname,detectbl)
   char *segname;
   int detectbl;
   {
   char *funcname;
   int errnum,index;

   funcname = "setdetectability";
   if (detectbl != TRUE && detectbl != FALSE)
      {
      errnum = 27;
      errhand(funcname,errnum);
      return(2);
      }
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if ((segptr == openseg) && opsegemp)
	    {
	    return(0);
	    }
	 ddstruct.string = segname;
	 for (index = 0; index < segptr->vsurfnum; index++)
	    {
	    if (segptr->vsurfptr[index]->dthardwr)
	       {
	       ddstruct.opcode = SETDTCT;
	       ddstruct.logical = detectbl;
	       (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
	       }
	    }
	 segptr->detect1bl = detectbl;
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





