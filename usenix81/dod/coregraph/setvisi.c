#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setvisibility                                              */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setvisibility(segname,visbilty)
   char *segname;
   int visbilty;
   {
   char *funcname;
   int errnum,index;
   int previous;

   funcname = "setvisibility";
   if (visbilty != TRUE && visbilty != FALSE)
      {
      errnum = 27;
      errhand(funcname,errnum);
      return(2);
      }
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if ((visbilty != segptr->visbil1ty) && !((segptr == openseg) && opsegemp))
	    {
	     previous = segptr->redraw;
	     segptr->redraw = TRUE;
	     ddstruct.string = segname;
	     for (index = 0; index < segptr->vsurfnum; index++)
		{
		if (segptr == openseg)
		   {
		   if (openseg->vsurfptr[index]->segopclo)
		      {
		      ddstruct.opcode = CLOSEG;
		      (*(openseg->vsurfptr[index]->dev1drive))(&ddstruct);
		      }
		   }
		if (segptr->vsurfptr[index]->vshardwr)
		   {
		    ddstruct.opcode = SETVSBL;
		    ddstruct.logical = visbilty;
		    (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
		   }
		else
		   {
		   if (segptr->vsurfptr[index]->dehardwr && !visbilty && !previous)
		      {
		       ddstruct.opcode = DELETE;
		       (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
		      }
		   else if (segptr->vsurfptr[index]->erasure && !visbilty && !previous)
		      {
		       segdra0(segptr,index);
		      }
		   else
		      {
		      segptr->vsurfptr[index]->nwframnd = TRUE;/*** new frame later ***/
		      }
		   }
		}
	     segptr->visbil1ty = visbilty;
	     if (!batchupd)
		{
		 repaint();
		}
	     if (segptr == openseg)
		{
		 reopensegment();/** no arguments, assumes previously open segment **/
				 /** sets up transformation matrix, etc. no copying**/
		}
	    }
	 else if ((segptr == openseg) && opsegemp)
	    {
	    setmatrix(segptr);
	    }
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


