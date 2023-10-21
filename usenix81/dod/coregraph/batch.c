#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: beginbatchofupdates                                        */
/*                                                                          */
/*     PURPOSE: DENOTES THE BEGINNING OF A BATCH OF CHANGES TO THE PICTURE. */
/*                                                                          */
/****************************************************************************/

beginbatchofupdates()
   {
   char *funcname;
   int errnum;

   funcname = "beginbatchofupdates";

   if(batchupd)
      {
      errnum = 17;
      errhand(funcname,errnum);
      return(1);
      }

   batchupd = TRUE;

   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: endbatchofupdates                                          */
/*                                                                          */
/*     PURPOSE: DENOTES THE END OF A BATCH OF CHANGES TO THE PICTURE.       */
/*                                                                          */
/****************************************************************************/

endbatchofupdates()
   {
   char *funcname;
   int errnum;
   struct viewsurf *vslist[VSURFNUM];
   register int listcnt,index;

   funcname = "endbatchofupdates";

   if(!batchupd)
      {
      errnum = 18;
      errhand(funcname,errnum);
      return(1);
      }

/*** NEW FRAME ALL SURFACES WHOSE NEW FRAME NEEDED FLAG IS SET ***/
   repaint();

/*** eventually should add drain buffer command to DD ***/
/*** to be sent after each primitive command, only if no batch ***/

   batchupd = FALSE;
   return(0);
   }
