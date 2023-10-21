#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: settr2                                                     */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

settr2(segname,dx,dy)
   char *segname;
   float dx,dy;
   {
   char *funcname;
   int errnum,index;
   int previous;

   funcname = "settr2";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if (segptr->type < XLATE2)
	    {
	    errnum = 30;
	    errhand(funcname,errnum);
	    return(3);
	    }
	 if ((segptr->trans1lat[0] != dx || segptr->trans1lat[1] != dy) && !((segptr == openseg) && opsegemp))
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
		if (segptr->vsurfptr[index]->trhardwr && FALSE)
		/*** The FALSE insures that this will never happen for now***/
		/*** op.c is not now set up to make use of this hardware  ***/
		/*** feature, so we use software transformation           ***/
		   {
		   ddstruct.opcode = TRANS2LATE;
		   ddstruct.float1 = dx;
		   ddstruct.float2 = dy;
		   (*(segptr->vsurfptr[index]->dev1drive))(&ddstruct);
		   }
		else
		   {
		   if (segptr->vsurfptr[index]->dehardwr && !previous) /* vector without highlight */
		      {
		      ddstruct.opcode = DELETE;  /* delete, then redraw later */
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
	     segptr->trans1lat[0] = dx;
	     segptr->trans1lat[1] = dy;
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

