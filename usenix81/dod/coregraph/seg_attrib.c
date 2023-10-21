#include "inclusions.c"

double sin(),cos(),atan2();

/**************************************************************************/
/*  The following functions set and inquire the static segment attribute. */
/**************************************************************************/

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setsegtype                                                 */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setsegtype(segtype)
   int segtype;
   {
   char *funcname;
   int errnum;

   funcname = "setsegtype";
   if ((segtype < sgtypmin) || (segtype > sgtypmax))
      {
      errnum = 28;
      errhand(funcname,errnum);
      return(1);
      }
   csegtype = segtype;
   return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqsegtype                                                 */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqsegtype(segtype)
   int *segtype;
   {
   *segtype = csegtype;
   return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqnamedsegtype                                            */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqnamedsegtype(segname,segtype)
   char *segname;
   int *segtype;
   {
   char *funcname;
   int errnum;

   funcname = "inqnamedsegtype";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 *segtype = segptr->type;
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

/****************************************************************************/
/* THE FOLLOWING FUNCTIONS SET AND INQUIRE THE DYNAMIC SEGMENT ATTRIBUTES.  */
/****************************************************************************/

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



/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setsc2                                                     */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setsc2(segname,xscale,yscale)
   char *segname;
   float xscale, yscale;
   {
   char *funcname;
   int errnum,index;
   int previous;

   funcname = "setsc2";

   if(xscale == 0 || yscale == 0)
      {
      errnum = 27;
      errhand(funcname,errnum);
      return(2);
      }
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
	 if ((segptr->scale1[0] != xscale || segptr->scale1[1] != yscale) && !((segptr == openseg) && opsegemp))
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
		if (segptr->vsurfptr[index]->schardwr && FALSE)
		/*** The FALSE insures that this will never happen for now***/
		/*** op.c is not now set up to make use of this hardware  ***/
		/*** feature, so we use software transformation           ***/
		   {
		   ddstruct.opcode = SCALE2;
		   ddstruct.float1 = xscale;
		   ddstruct.float2 = yscale;
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
	     segptr->scale1[0] = xscale;
	     segptr->scale1[1] = yscale;
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


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqdetectability                                           */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqdetectability(segname,detectbl)
   char *segname;
   int *detectbl;
   {
   char *funcname;
   int errnum;

   funcname = "inqdetectability";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 *detectbl = segptr->detect1bl;
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






segdraw(asegptr,anindex)
   struct segstruc *asegptr;
   int anindex;
   {
    long offset;
    short sdopcode,n;
    char *sdstring,shortstring[2];
    float x,y,x1,y1,scfactor;
/*  float minx,miny,maxx,maxy;   NEEDED LATER FOR EXTENTS */


    if (!(asegptr->visbil1ty) && !(asegptr->vsurfptr[anindex]->vshardwr))
      {
       return(0);
      }
    else if(asegptr->vsurfptr[anindex]->vshardwr)
      {
       ddstruct.opcode = SETVSBL;
       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
      }
    setmatrix(asegptr);
    offset = asegptr->pdfptr;
    lseek(pdfd,offset,0);
    while (TRUE)
      {
       pdfread(SHORT,&sdopcode);
       if (sdopcode == PDFENDSEGMENT)
	   break;
       switch (sdopcode)
	 {
	  case PDFMOVE:
	    {
	     pdfread(FLOAT,&x);
	     pdfread(FLOAT,&y);
	     if(! idenflag) /** assume no transformation hardware ***/
		{
		 x1 = x;
		 y1 = y;
		 x = x1 * imxform[0][0] + y1 * imxform[1][0] + imxform[2][0];
		 y = x1 * imxform[0][1] + y1 * imxform[1][1] + imxform[2][1];
		}
	     ddstruct.opcode = MOVE;
	     ddstruct.float1 = x;
	     ddstruct.float2 = y;
	     (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFLINE:
	    {
	     pdfread(FLOAT,&x);
	     pdfread(FLOAT,&y);
	     if(! idenflag) /** assume no transformation hardware in storage ***/
		{
		 x1 = x;
		 y1 = y;
		 x = x1 * imxform[0][0] + y1 * imxform[1][0] + imxform[2][0];
		 y = x1 * imxform[0][1] + y1 * imxform[1][1] + imxform[2][1];
		}
	     if ((pdfcurrent.linestyl == SOLID ) ||
		 (asegptr->vsurfptr[anindex]->lshardwr &&
		 pdfcurrent.linestyl < DOTDASHED))
		{
		 ddstruct.opcode = LINE;
		 ddstruct.float1 = x;
		 ddstruct.float2 = y;
		 (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
		}
	     else
		{
		 simline(x,y); /** needs pointer and index ***/
		 /*** This function will simulate a line of the current ***/
		 /*** line style and width by sending simple line com-  ***/
		 /*** mands to the indexed DD as above (MOVE and LINE)  ***/
		 /*** excluding the initial MOVE which is already done  ***/
		}

	    break;
	    }
	  case PDFTEXT:
	    {
	     pdfread(SHORT,&n);
	     pdfread(n,sdstring);
	     *(sdstring + n) = '\0';

	     if ((pdfcurrent.chqualty == LOW) && (asegptr->vsurfptr[anindex]->txhardwr))
		{
		ddstruct.opcode = TEXT;
		ddstruct.string = sdstring;
		ddstruct.logical = TRUE;
		(*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
		}
	     else if ((pdfcurrent.chqualty == MEDIUM) && (asegptr->vsurfptr[anindex]->txhardwr))
		{
		shortstring[1] = '\0';

		for ( ;*sdstring != '\0';sdstring++)
		   {
		   shortstring[0] = *sdstring;

		   ddstruct.opcode = TEXT;
		   ddstruct.logical = FALSE;
		   ddstruct.string = shortstring;
		   (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);

		   x = x + pdfcurrent.chrspace.dx;
		   y = y + pdfcurrent.chrspace.dy;

		   ddstruct.opcode = MOVE;
		   ddstruct.float1 = x;
		   ddstruct.float2 = y;
		   (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);

		   }
		}
	     else /*** must use software generation ***/
		{
		softstring(sdstring); /*** this function sends the appropriate ***/
				    /*** line drawing commands to the DD to  ***/
				    /*** draw the required text on the re-   ***/
				    /*** quired surface using the char params***/
		}

	     break;
	    }
	  case PDFMARKER:
	    {
	     pdfread(SHORT,&n);

	     if (asegptr->vsurfptr[anindex]->txhardwr)
		{
		ddstruct.opcode = MARK;
		ddstruct.string = n;
		(*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
		}
	     else /*** must use software generation ***/
		{
		softmark(n); /*** this function sends the appropriate ***/
			     /*** line drawing commands to the DD to  ***/
			     /*** draw the required marker on the re-   ***/
			     /*** quired surface using the char params***/
		}

	    break;
	    }
	  case PDFCOLOR:
	    {
	    pdfread(FLOAT,&(pdfcurrent.color[0]));
	    pdfread(FLOAT,&(pdfcurrent.color[1]));
	    pdfread(FLOAT,&(pdfcurrent.color[2]));
	    if (asegptr->vsurfptr[anindex]->clhardwr && !rastererase)
	       {
	       ddstruct.opcode = SETCOL;
	       ddstruct.float1 = pdfcurrent.color[0];
	       ddstruct.float2 = pdfcurrent.color[1];
	       ddstruct.float3 = pdfcurrent.color[2];
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFINTENSITY:
	    {
	    pdfread(FLOAT,&(pdfcurrent.intensty));
	    if (asegptr->vsurfptr[anindex]->inhardwr && !rastererase)
	       {
	       ddstruct.opcode = SETINT;
	       ddstruct.float1 = pdfcurrent.intensty;
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFLINESTYLE:
	    {
	    pdfread(SHORT,&(pdfcurrent.linestyl));
	    if (asegptr->vsurfptr[anindex]->lshardwr)
	       {
	       ddstruct.opcode = SETSTYL;
	       ddstruct.int1 = pdfcurrent.linestyl;
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFLINEWIDTH:
	    {
	    pdfread(FLOAT,&(pdfcurrent.linwidth));
	    if (asegptr->vsurfptr[anindex]->lwhardwr)
	       {
	       ddstruct.opcode = SETWIDTH;
	       ddstruct.float1 = pdfcurrent.linwidth;
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFFONT:
	    {
	    pdfread(SHORT,&(pdfcurrent.font));
	    ddstruct.opcode = SETFONT;
	    ddstruct.int1 = pdfcurrent.font;
	    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFSIZE:
	    {
	    pdfread(FLOAT,&(pdfcurrent.height));
	    pdfread(FLOAT,&(pdfcurrent.width));
	    ddstruct.opcode = SETSIZE;
	    /*** must convert character size, which is used for hardware ***/
	    /*** character size determination by DD, by multiplying by   ***/
	    /*** the average of the x and y scale factors (good as any)  ***/
	    scfactor = (asegptr->scale1[0] + asegptr->scale1[1]) / 2.0;
	    pdfcurrent.height = scfactor * pdfcurrent.height;
	    pdfcurrent.width = scfactor * pdfcurrent.width;
	    ddstruct.float1 = pdfcurrent.height;
	    ddstruct.float2 = pdfcurrent.width;
	    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFSPACE:
	    {
	    pdfread(FLOAT,&(pdfcurrent.chrspace.dx));
	    pdfread(FLOAT,&(pdfcurrent.chrspace.dy));
	    ddstruct.opcode = SETANGLE;
	    /*** must convert spacing according to image transformation ***/
	    /*** without translation                                    ***/
	    x1 = pdfcurrent.chrspace.dx;
	    y1 = pdfcurrent.chrspace.dy;
	    pdfcurrent.chrspace.dx = x1 * imxform[0][0] + y1 * imxform[1][0];
	    pdfcurrent.chrspace.dy = x1 * imxform[0][1] + y1 * imxform[1][1];
	    ddstruct.float1 = atan2(pdfcurrent.chrspace.dy,pdfcurrent.chrspace.dx);
	    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFCHARQUALITY:
	    {
	    pdfread(SHORT,&(pdfcurrent.chqualty));
	    break;
	    }
	  case PDFPICKID:
	    {
	    pdfread(SHORT,&(pdfcurrent.pick2id));

	    /******************************************/
	    /***                                    ***/
	    /*** pdfread(FLOAT,&minx);              ***/
	    /*** pdfread(FLOAT,&miny);              ***/
	    /*** pdfread(FLOAT,&maxx);    EXTENTS?? ***/
	    /*** pdfread(FLOAT,&maxy);              ***/
	    /***                                    ***/
	    /******************************************/
	    break;
	    }
	  default:
	    break;
	 }
      }
    setmatrix(openseg);
    return(0);
   }

segdra0(asegptr,anindex)
   struct segstruc *asegptr;
   int anindex;
   {
    rastererase = TRUE;
    ddstruct.opcode = SETCOL;
    ddstruct.float1 = 0.0;
    ddstruct.float2 = 0.0;
    ddstruct.float3 = 0.0;
    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
    ddstruct.opcode = SETINT;
    ddstruct.float1 = 0.0;
    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
    segdraw(asegptr,anindex);
    rastererase = FALSE;
    return(0);
   }

repaint()
   {
    int vsindex,segindex,flag;
    for (vsindex = 0; vsindex < MAXVSURF; vsindex++)
	{
	 if (surface[vsindex].nwframnd && (surface[vsindex].nwframdv || explicit))
	    {
	     ddstruct.opcode = CLEAR;
	     (*(surface[vsindex].dev1drive))(&ddstruct);
	    }
	}
    for (segindex = 0; segindex < SEGNUM; segindex++)
	{
	 flag = FALSE; /** no view surfaces need repaint **/
	 for (vsindex = 0; vsindex < MAXVSURF; vsindex++)
	     {
	      if (surface[vsindex].dehardwr && segment[segindex].type == NORETAIN && !explicit)
		 {
		  ddstruct.opcode = DELETE;
		  ddstruct.string = surface[vsindex].seg1name;
		  (*(surface[vsindex].dev1drive))(&ddstruct);
		 }
	      else if (surface[vsindex].nwframnd &&
		      (segment[segindex].redraw || segment[segindex].vsurfptr[vsindex]->nwframdv || explicit))
		 {
		  flag = TRUE;
		 }
	     }
	 if (flag && segment[segindex].type >= RETAIN)
	     {
	      for (vsindex = 0; vsindex < segment[segindex].vsurfnum; vsindex++)
		  {
		   if (segment[segindex].vsurfptr[vsindex]->segopclo
		       && (explicit
		       || (segment[segindex].vsurfptr[vsindex]->nwframnd
		       && segment[segindex].redraw)))
		      {
		       ddstruct.opcode = OPENSEG;
		       ddstruct.string = segment[segindex].seg1name;
		       (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
		      }
		   /*** Type really ought to be sent to DD too ***/
		   /*** It doesn't really matter that many of these may be ignored by the DDs ***/
		   /*** but to save time later we may wish to test for usefulness ***/
/*** commented out until above can be implemented ***/
/*                 ddstruct.string = segment[segindex].seg1name;
		   ddstruct.logical = segment[segindex].visbil1ty;
		   ddstruct.opcode = SETVSBL;
		   (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
		   ddstruct.logical = segment[segindex].detect1bl;
		   ddstruct.opcode = SETDTCT;
		   (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
		   ddstruct.logical = segment[segindex].high1lght;
		   ddstruct.opcode = SETHILIT;
		   (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
		   ddstruct.float1 = segment[segindex].scale1[0];
		   ddstruct.float2 = segment[segindex].scale1[1];
		   ddstruct.opcode = SCALE2;
		   (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
		   ddstruct.float1 = segment[segindex].trans1lat[0];
		   ddstruct.float2 = segment[segindex].trans1lat[1];
		   ddstruct.opcode = TRANS2LATE;
		   (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
		   ddstruct.float1 = segment[segindex].rotate1;
		   ddstruct.opcode = ROTATE2;
		   (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
*/
		  }
	      setmatrix(&(segment[segindex]));
	      for (vsindex = 0; vsindex < segment[segindex].vsurfnum; vsindex++)
		  {
		  /*** later, may revise segdraw so that -1 vsindex means ***/
		  /*** do all view surfaces for this segment so that SDF  ***/
		  /*** need only be read once rather than once per VS     ***/
		  if (segment[segindex].vsurfptr[vsindex]->nwframnd &&
		     (segment[segindex].visbil1ty || segment[segindex].vsurfptr[vsindex]->vshardwr)
		     && (segment[segindex].redraw || segment[segindex].vsurfptr[vsindex]->nwframdv || explicit))
		     {
		      segdraw(&segment[segindex],vsindex);
		     }
		  if (segment[segindex].vsurfptr[vsindex]->segopclo
		      && (explicit
		      || (segment[segindex].vsurfptr[vsindex]->nwframnd
		      && segment[segindex].redraw)))
		     {
		       ddstruct.opcode = CLOSEG;
		       (*(segment[segindex].vsurfptr[vsindex]->dev1drive))(&ddstruct);
		     }
		  }
	     }
	 if (segment[segindex].type == NORETAIN)
	     {
	      segment[segindex].type = EMPTY;
	     }
	 else
	     {
	      segment[segindex].redraw = FALSE;
	     }
	}
    for (vsindex = 0; vsindex < MAXVSURF; vsindex++)
	{
	 surface[vsindex].nwframnd = FALSE;
	}
    return(0);
   }


setmatrix(asegptr)
   struct segstruc *asegptr;
   {

   imxform[0][0] = cos(asegptr->rotate1) * asegptr->scale1[0];
   imxform[0][1] = -sin(asegptr->rotate1) * asegptr->scale1[1];
   imxform[1][0] = sin(asegptr->rotate1) * asegptr->scale1[0];
   imxform[1][1] = cos(asegptr->rotate1) * asegptr->scale1[1];
   imxform[2][0] = asegptr->trans1lat[0];
   imxform[2][1] = asegptr->trans1lat[1];

   if(identchk(imxform) == 0)
      {
      idenflag = TRUE;
      }
   else idenflag = FALSE;

    return(0);
   }

reopensegment()
   {
   int index;
/*** delete the segment from devices ***/
   for (index = 0; index < openseg->vsurfnum; index++)
      {
      if (openseg->vsurfptr[index]->dehardwr)
	 {
	 ddstruct.opcode = DELETE;
	 ddstruct.string = openseg->seg1name;
	 (*(openseg->vsurfptr[index]->dev1drive))(&ddstruct);
	 }
/*** reopen the segment by calling DD's ***/
      if (openseg->vsurfptr[index]->segopclo)
	 {
	 ddstruct.opcode = OPENSEG;
	 ddstruct.string = openseg->seg1name;
	 (*(openseg->vsurfptr[index]->dev1drive))(&ddstruct);

	 /*** put back what was already there, leaving it open ***/
	 segdraw(openseg,index);
	 }
      }
/*** set up matrix ***/

/* setmatrix(openseg);*/
   return(0);
   }



newframe()
   {
    int index;
    explicit = TRUE;
    /*** set all currently selected view surfaces' repaint flags ***/
    for (index = 0; index < MAXVSURF; index++)
	{
	 surface[index].nwframnd = surface[index].selected;
	}
    /*** then call repaint ***/
    repaint();
    explicit = FALSE;
   return(0);
   }

