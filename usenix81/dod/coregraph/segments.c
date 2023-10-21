#include "inclusions.c"


double sin(),cos();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: createseg                                                  */
/*                                                                          */
/*     PURPOSE: CREATE A NEW, EMPTY SEGMENT.                                */
/*                                                                          */
/****************************************************************************/

createseg(segname)
   char *segname;
   {
   char *funcname;
   int errnum;
   register int index;

   funcname = "createseg";
   if (osexists)
      {
      errnum = 8;
      errhand(funcname,errnum);
      return(1);
      }
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
       {
       if((segptr->type == DELETED) || (segptr->type == EMPTY))
	  {
	  openseg = segptr;
	  break;
	  }
       if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	  {
	  errnum = 31;
	  errhand(funcname,errnum);
	  return(2);
	  }
       }
   if(csegtype == RETAIN)
      {
      if(defsegat.scale2[0] != 1 || defsegat.scale2[1] != 1 || defsegat.trans2lat[0] != 0 || defsegat.trans2lat[1] != 0 || defsegat.rotate2 != 0)
	 {
	 errnum = 32;
	 errhand(funcname,errnum);
	 return(3);
	 }
      }
      else if(csegtype == XLATE2)
	 {
	 if(defsegat.scale2[0] != 1 || defsegat.scale2[1] != 1 || defsegat.rotate2 != 0)
	    {
	    errnum = 32;
	    errhand(funcname,errnum);
	    return(3);
	    }
	 }
   if (validvt() != 0)  /*** valid viewing transformation? ***/
      {
      errnum = 34;
      errhand(funcname,errnum);
      return(5);
      }
   errnum = 33;  /*** assume error ***/
   openseg->vsurfnum = 0; /*** SET NUMBER OF RELATED SURFACES TO SEGMENT EQUAL TO 0 ***/
   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if (surfptr->selected)
	 {
	 errnum = 0;  /*** no error because one is selected ***/
	 /*************************************************************************/
	 /***                                                                   ***/
	 /*** PLACE POINTERS TO RELATED SURFACES IN THE OPEN SEGMENT STRUCTURE. ***/
	 /***                                                                   ***/
	 /*************************************************************************/

	 openseg->vsurfptr[(openseg->vsurfnum)++] = surfptr;

	 if(surfptr->segopclo)
	    {
	    ddstruct.opcode = OPENSEG;
	    ddstruct.string = openseg->seg1name;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 }
      }
   if (errnum == 33)  /*** never found one selected ***/
      {
      errhand(funcname,errnum);
      return(4);
      }

   ndcspuse = TRUE;
   openseg->type = csegtype;
   if(csegtype >= RETAIN)
      {
      openseg->seg1name =segname;
      }

   /*******************************************/
   /***                                     ***/
   /*** SET SEGMENT DYNAMIC ATTRIBUTES FROM ***/
   /*** DEFAULT SEGMENT ATTRIBUTES          ***/
   /***                                     ***/
   /*******************************************/

   openseg->visbil1ty = defsegat.visbil2ty;
   openseg->detect1bl = defsegat.detect2bl;
   openseg->high1lght = defsegat.high2lght;
   openseg->scale1[0] = defsegat.scale2[0];
   openseg->scale1[1] = defsegat.scale2[1];
   openseg->trans1lat[0] = defsegat.trans2lat[0];
   openseg->trans1lat[1] = defsegat.trans2lat[1];
   openseg->rotate1 = defsegat.rotate2;
   openseg->pdfptr = pdfnext;
   openseg->segsize = 0;

   /*********************************************/
   /***                                       ***/
   /*** CONSTRUCT IMAGE TRANSFORMATION MATRIX ***/
   /***                                       ***/
   /*********************************************/

   imxform[0][0] = cos(defsegat.rotate2) * defsegat.scale2[0];
   imxform[0][1] = -sin(defsegat.rotate2) * defsegat.scale2[1];
   imxform[1][0] = sin(defsegat.rotate2) * defsegat.scale2[0];
   imxform[1][1] = cos(defsegat.rotate2) * defsegat.scale2[1];
   imxform[2][0] = defsegat.trans2lat[0];
   imxform[2][1] = defsegat.trans2lat[1];

   if(identchk(imxform) == 0)
      {
      idenflag = TRUE;
      }
   else idenflag = FALSE;

   /***************************************************/
   /***                                             ***/
   /*** ADJUST SEGMENT STATE AND OUTPUT PRIMITIVE   ***/
   /*** FLAGS TO INITIAL OPEN SEGMENT VALUE OF TRUE ***/
   /***                                             ***/
   /***************************************************/

   opsegemp = TRUE;
   intflag = TRUE;
   clrflag = TRUE;
   lsflag = TRUE;
   lwflag = TRUE;
   fntflag = TRUE;
   szeflag = TRUE;
   angflag = TRUE;
   qualflag = TRUE;
   picflag = TRUE;
   prevseg = TRUE;
   osexists = TRUE;

   /***********************************************/
   /***                                         ***/
   /*** CURRENT PRIMITIVE ATTRIBUTES ARE SET TO ***/
   /*** DEFAULT PRIMITIVE ATTRIBUTES            ***/
   /***                                         ***/
   /***********************************************/

   current.color[0] = defaultt.color[0];
   current.color[1] = defaultt.color[1];
   current.color[2] = defaultt.color[2];
   current.intensty = defaultt.intensty;
   current.linwidth = defaultt.linwidth;
   current.linestyl = defaultt.linestyl;
   current.charsize.width = defaultt.charsize.width;
   current.charsize.height = defaultt.charsize.height;
   current.chrspace.dx = defaultt.chrspace.dx;
   current.chrspace.dy = defaultt.chrspace.dy;
   current.chqualty = defaultt.chqualty;
   current.pick2id = defaultt.pick2id;

   /***********************************/
   /***                             ***/
   /*** INITIALIZE CURRENT POSITION ***/
   /***                             ***/
   /***********************************/

   cp[0] = 0;
   cp[1] = 0;

   segnum++;
   return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: closeseg                                                   */
/*                                                                          */
/*     PURPOSE: THE CURRENTLY OPENED SEGMENT BECOMES CLOSED.                */
/*                                                                          */
/****************************************************************************/

closeseg()
   {
   char *funcname;
   int errnum;
   register int index;

   funcname = "closeseg";
   if (! osexists)
      {
      errnum = 26;
      errhand(funcname,errnum);
      return(1);
      }

   if(openseg->type == NORETAIN)
      {
      openseg->type = DELETED;
      }
   for (index = 0; index < segptr->vsurfnum; index++)
      {
      if (openseg->vsurfptr[index]->segopclo)
	 {
	 ddstruct.opcode = CLOSEG;
	 ddstruct.string = openseg->seg1name;
	 (*(openseg->vsurfptr[index])->dev1drive)(&ddstruct);
	 }
      }
   /*******************************************/
   /***                                     ***/
   /*** PLACE END OF SEGEMENT MARKER IN PDF ***/
   /***                                     ***/
   /*******************************************/

   if(openseg->type >= RETAIN)
      {
      ptype = PDFENDSEGMENT;
      pdfwrite(1,&ptype);
      }

   osexists = FALSE;
   openseg = NULL;
   return(0);
   }



/****************************************************************************/
/*                                                                          */
/*     FUNCTION: deleteseg                                                  */
/*                                                                          */
/*     PURPOSE: THE RETAINED SEGMENT 'segname' IS DELETED.                  */
/*                                                                          */
/****************************************************************************/

deleteseg(segname)
   char *segname;
   {
   char *funcname;
   int errnum;
   register int index;

   funcname = "deleteseg";
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 if (segptr == openseg)
	    {
	    closeseg(); /*** close the open segment ***/
	    }
	 segptr->type = DELETED;
	 if(segptr->visbil1ty)
	    {
	    for (index = 0; index < segptr->vsurfnum; index++)
	       {
	       if (segptr->vsurfptr[index]->dehardwr)
		  {
		  ddstruct.opcode = DELETE;
		  ddstruct.string = segptr->seg1name;
		  (*(segptr->vsurfptr[index])->dev1drive)(&ddstruct);
		  }
	       else if (segptr->vsurfptr[index]->erasure)
		  {
		  segdra0(segptr,index); /*** redisplay with 0 intensity and color ***/
		  }
	       else
		  {
		  if (batchupd)
		     {
		     (segptr->vsurfptr[index])->nwframnd = TRUE;/*** new frame later ***/
		     }
		  else
		     {
		     repaint();/*** do a new frame action now ***/
		     }
		  }
	       }
	    }
	 --segnum;
	 return(0);
	 }
      if (segptr->type == EMPTY)
	 {
	 errnum = 14;
	 errhand(funcname,errnum);
	 return(1);
	 }
      }
   errnum = 14;
   errhand(funcname,errnum);
   return(1);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: deleteallsegs                                              */
/*                                                                          */
/*     PURPOSE: ALL RETAINED AND NON-RETAINED SEGMENTS ARE DELETED.         */
/*                                                                          */
/****************************************************************************/

deleteallsegs()
   {
   char *funcname;
   int errnum;
   register int index;
   int found;                  /*** BOOLEAN UNDER VERSION 7 ***/

   funcname = "deleteallsegs";
   found = FALSE;
   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if(surfptr->vinit)
	 {
	 found = TRUE;
	 }
      }
   if(found == FALSE)
      {
      errnum = 35;
      errhand(funcname,errnum);
      return(1);
      }
   for(segptr = &segment[0];segptr < &segment[SEGNUM] && segptr->type != EMPTY;segptr++)
      {
      if(segptr == openseg)
	 {
	 closeseg();
	 }
      segptr->type = DELETED;
      }
   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if(surfptr->vinit)
	 {
	 if (batchupd)
	    {
	    surfptr->nwframnd = TRUE;/*** new frame later ***/
	    }
	    else
	       {
	       ddstruct.opcode = CLEAR;
	       (*(surfptr->dev1drive))(&ddstruct);
	       }
	 }
      }
   pdfnext = 0;          /*** RESET PSEUDO DISPLAY FILE POINTER ***/
   segnum = 0;           /*** RESET SEGMENT NUMBER COUNTER ***/
   return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: renameseg                                                  */
/*                                                                          */
/*     PURPOSE: EXISTING RETAINED SEGMENT NAMED BY THE PARAMETER 'segname'  */
/*              IS HENCEFORTH KNOWN BY THE NAME SPECIFIED BY THE 'newname'  */
/*              PARAMETER.                                                  */
/*                                                                          */
/****************************************************************************/

renameseg(segname,newname)
   char *segname;
   char *newname;
   {
   char *funcname;
   int errnum;

   funcname = "renameseg";

   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(newname,segptr->seg1name) == 0))
	 {
	 errnum = 36;
	 errhand(funcname,errnum);
	 return(1);
	 }
      }
   for (segptr = &segment[0]; segptr < &segment[SEGNUM]; segptr++)
      {
      if((segptr->type != DELETED) && (strcmp(segname,segptr->seg1name) == 0))
	 {
	 segptr->seg1name = newname;
	 return(0);
	 }
      if (segptr->type == EMPTY)
	 {
	 errnum = 37;
	 errhand(funcname,errnum);
	 return(2);
	 }
      }
   errnum = 37;
   errhand(funcname,errnum);
   return(2);
   }




/****************************************************************************/
/*                                                                          */
/*     FUNCTION: validvt                                                    */
/*                                                                          */
/*     PURPOSE: WHEN A SEGMENT IS CREATED, THE CURRENT VIEWING PARAMETERS   */
/*              ARE CHECKED FOR CONSISTENCY TO ENSURE THAT A VALID          */
/*              VIEWING TRANSFORMATION HAS BEEN SPECIFIED.                  */
/*                                                                          */
/****************************************************************************/

validvt()
   {
   /**************************************************************/
   /***                                                        ***/
   /*** SITUATIONS WHICH CAUSE ERRORS ONLY APPLY TO 3D,SO      ***/
   /*** FOR 2D IMPLEMENTATION, THIS FUNCTION ALWAYS RETURNS 0. ***/
   /***                                                        ***/
   /**************************************************************/

   return(0);
   }
