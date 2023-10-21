#include "inclusions.c"

struct temp1
   {
   float xmin;
   float xmax;
   float ymin;
   float ymax;
   };


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqpos2                                                    */
/*                                                                          */
/*     PURPOSE: INQUIRE CURRENT DRAWING POSITION                            */
/*                                                                          */
/****************************************************************************/

inqpos2(x,y)
   float *x,*y;
   {
   /******************************************************************/
   /***                                                            ***/
   /*** NOTE:CP "WILL" HOLD VALUE FROM MOST RECENTLY SELECTED VIEW ***/
   /*** SURFACE FOLLOWING A LOW-QUALITY TEXT FUNCTION, HENCE CP    ***/
   /*** ALWAYS HOLDS CORRECT VALUE.                                ***/
   /***                                                            ***/
   /******************************************************************/

   *x = cp[0];
   *y = cp[1];

   return(0);
   }




/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqvwparameters                                            */
/*                                                                          */
/*     PURPOSE: INQUIRE CURRENT VIEWING PARAMETERS.                         */
/*                                                                          */
/****************************************************************************/

inqvwparameters(viewparm)
   struct
      {
      struct temp1 vwport;
      struct temp1 wynndow;
      float vwplnorm[3];
      float vwrefpt[3];
      float vwupdir[3];
      float viewdis;
      float frontdis;
      float backdis;
      int projtype;
      float projdir;
      } *viewparm;
   {

   viewparm->vwport.xmin = viewport.xmin;
   viewparm->vwport.xmax = viewport.xmax;
   viewparm->vwport.ymin = viewport.ymin;
   viewparm->vwport.ymax = viewport.ymax;

   viewparm->wynndow.xmin = window.xmin;
   viewparm->wynndow.xmax = window.xmax;
   viewparm->wynndow.ymin = window.ymin;
   viewparm->wynndow.ymax = window.ymax;

   /****************************************************/
   /***                                              ***/
   /*** REMAINING VARIABLES IN STRUCTURE RELATE TO   ***/
   /*** 3D AND ARE NOT USED IN THIS IMPLEMENTATION.  ***/
   /***                                              ***/
   /****************************************************/

   return(0);
   }






/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqvwtransformation                                        */
/*                                                                          */
/*     PURPOSE: INQUIRE CURRENT VIEWING TRANSFORMATION.                     */
/*                                                                          */
/****************************************************************************/

inqvwtransformation(arrayptr)
   float *arrayptr;
   {
   float *matrxptr;

   for(matrxptr = &vwxform[0][0];matrxptr <= &vwxform[1][1];matrxptr++,arrayptr++)
      {
      *arrayptr = *matrxptr;
      }

   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqoutstatusparameters                                     */
/*                                                                          */
/*     PURPOSE: INQUIRE OUTPUT STATUS PARAMETERS.                           */
/*                                                                          */
/****************************************************************************/

inqoutstatusparameters(outstat)
   struct
      {
      int wclipmd;
      int dclipmd;
      int corsystp;
      float devcorsp[2];
      char *vselectd[MAXVSURF];
      char *opsegnam;
      } *outstat;

   {
   register int index;

   outstat->wclipmd = wndwclip;
   outstat->corsystp = coordsys;
   outstat->devcorsp[0] = ndcspace[0];
   outstat->devcorsp[1] = ndcspace[1];

   index = 0;
   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if(surfptr->selected)
	 {
	 outstat->vselectd[index++] = surfptr->name;
	 }
      }

   if(osexists)
      {
      outstat->opsegnam = openseg->seg1name;
      }

   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqsegsurfaces                                             */
/*                                                                          */
/*     PURPOSE: INQUIRE WHICH LOGICAL VIEW SURFACES WERE SELECTED WHEN THE  */
/*              SEGMENT 'segname' WAS CREATED.                              */
/*                                                                          */
/****************************************************************************/

inqsegsurfaces(segname,surfnum,surfaray,arraycnt)
   char *segname;
   int *surfnum;
   char *surfaray[];
   int arraycnt;

   {
   char *funcname;
   int errnum;
   register int index;
   int found;                  /*** BOOLEAN UNDER VERSION 7 ***/

   funcname = "inqsegsurfaces";
   found = FALSE;

   for(segptr = &segment[0];segptr < &segment[SEGNUM];segptr++)
      {
      if(strcmp(segname,segptr->seg1name) == 0)
	 {
	 found = TRUE;
	 break;
	 }
      }
   if(!found)                /*** SPECIFIED SEGMENT EXIST ?? ***/
      {
      errnum = 14;
      errhand(funcname,errnum);
      return(1);
      }

   /*********************************************/
   /***                                       ***/
   /*** COPY NUMBER OF VIEW SURFACES SELECTED ***/
   /*** WHEN SEGMENT WAS CREATED.             ***/
   /***                                       ***/
   /*********************************************/

   *surfnum = segptr->vsurfnum;

   /*************************************************/
   /***                                           ***/
   /*** COPY LOGICAL NAMES OF THOSE VIEW SURFACES ***/
   /***                                           ***/
   /*************************************************/

   for(index = 0;index < segptr->vsurfnum;index++)
      {
      if(index > arraycnt)
	 {
	 errnum = 15;
	 errhand(funcname,errnum);
	 return(2);
	 }
	 else
	    {
	    surfaray[index] = segptr->vsurfptr[index]->name;
	    }
      }

   return(0);
   }




/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqnumberofsegs                                            */
/*                                                                          */
/*     PURPOSE: INQUIRE NUMBER OF ACTIVE SEGMENTS.                          */
/*                                                                          */
/****************************************************************************/

inqnumberofsegs(segcnt)
   int *segcnt;
   {

   *segcnt = segnum;

   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqsegnames                                                */
/*                                                                          */
/*     PURPOSE: INQUIRE EXISTING SEGMENT NAMES.                             */
/*                                                                          */
/****************************************************************************/

inqsegnames(seglist,listcnt)
   char *seglist[];
   int listcnt;
   {

   char *funcname;
   int errnum;
   register int index;

   index = 0;
   for(segptr = &segment[0];segptr < &segment[SEGNUM];segptr++)
      {
      if(segptr->type > NORETAIN)   /*** SEGMENT CURRENTLY EXIST ?? ***/
	 {
	 if(index > listcnt)
	    {
	    errnum = 16;
	    errhand(funcname,errnum);
	    return(1);
	    }
	    else
	       {
	       seglist[index++] = segptr->seg1name;
	       }
	 }
      }

   return(0);
   }
