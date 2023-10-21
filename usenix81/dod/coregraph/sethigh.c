#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: sethighlighting                                            */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

sethighlighting(segname,highlght)
   char *segname;
   int highlght;
   {
   char *funcname;
   int errnum,index;
   int previous;

   funcname = "sethighlighting";
   if (highlght != TRUE && highlght != FALSE)
      {
      errnum = 27;
      errhand(funcname,errnum);
      return(2);
      }
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if ((highlght == TRUE || segptr->high1lght == TRUE) && !((segptr == openseg) && opsegemp))
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
		if (segptr->vsurfptr[index]->hlhardwr) /* vector with highlight by segment feature */
		   {
		   ddstruct.opcode = SETHILIT;
		   ddstruct.logical = highlght;
		   (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
		   }
		else
		   {
		   if (segptr->vsurfptr[index]->dehardwr && !previous) /* vector without highlight */
		      {
		      ddstruct.opcode = DELETE;  /* causing a one time blink  */
		      (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
		      segptr->vsurfptr[index]->nwframnd = TRUE;/*** repaint will undo the above ***/
		      }
		   else if (segptr->vsurfptr[index]->erasure && !previous) /* raster */
		      {
		      segdra0(segptr,index); /* erase segment from indexed VS */
					      /* and redraw later              */
		      segptr->vsurfptr[index]->nwframnd = TRUE;/*** repaint will undo the above ***/
		      }
		   else if (segptr->vsurfptr[index]->nwframdv) /* storage tube */
		      {
		      segdraw(segptr,index);
		      /*** redraw this segment only, highlighting by flash ***/
		      }
		   }
		}
	     segptr->high1lght = highlght;
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

