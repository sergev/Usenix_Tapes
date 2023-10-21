#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setro2                                                     */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setro2(segname,angle)
   char *segname;
   float angle;
   {
   char *funcname;
   int errnum,index;
   int previous;

   funcname = "setro2";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if (segptr->type < XFORM2)
	    {
	    errnum = 30;
	    errhand(funcname,errnum);
	    return(3);
	    }
	 if ((segptr->rotate1 != angle) && !((segptr == openseg) && opsegemp))
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
		if (segptr->vsurfptr[index]->rohardwr && FALSE)
		/*** The FALSE insures that this will never happen for now***/
		/*** op.c is not now set up to make use of this hardware  ***/
		/*** feature, so we use software transformation           ***/
		   {
		   ddstruct.opcode = ROTATE2;
		   ddstruct.float1 = angle;
		   (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
		   }
		else
		   {
		   if (segptr->vsurfptr[index]->dehardwr && !previous) /* vector without highlight */
		      {
		      ddstruct.opcode = DELETE;  /* causing a one time blink  */
		      (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
		      }
		   else if (segptr->vsurfptr[index]->erasure && !previous) /* raster */
		      {
		      segdra0(segptr,index); /* erase segment from indexed VS */
					      /* and redraw later              */
		      }
		   segptr->vsurfptr[index]->nwframnd = TRUE;/*** new frame later ***/
		   }
		}
	     segptr->rotate1 = angle;
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
	  ndcspuse = TRUE;
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



