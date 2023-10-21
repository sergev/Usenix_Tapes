#include "inclusions.c"


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: initvwsurface                                              */
/*                                                                          */
/*     PURPOSE: OBTAIN ACCESS TO THE LOGICAL VIEW SURFACE 'surfname' AND    */
/*              TO INITIALIZE THAT SURFACE.                                 */
/*                                                                          */
/****************************************************************************/

initvwsurface(surfname)
   char *surfname;
   {
   char *funcname;
   int errnum;

   funcname = "initvwsurface";
   if(sysinit == FALSE)
      {
      errnum = 4;
      errhand(funcname,errnum);
      return(3);
      }

   /*********************************************************************/
   /***                                                               ***/
   /*** SEARCH SURFACE STRUCTURE FOR SPECIFIED SURFACE NAME AND CHECK ***/
   /*** TO SEE IF IT HAS BEEN INITIALIZED. IF NOT, INITIALIZE IT.     ***/
   /***                                                               ***/
   /*********************************************************************/

   if(vsnsrch(surfname) == FALSE)
      {
      /********************************************************************/
      /*** CHECK LOGICAL DEVICE NAME TABLE FOR MATCHING SURFACE NAME.   ***/
      /*** IF NONE EXIST, THEN ERROR TYPE 2 FOR SUBROUTINE HAS OCCURED. ***/
      /*** TEMPORARILY ASSUME A MATCH WAS FOUND,AND A POINTER TO IT     ***/
      /*** RETURNED.                                                    ***/
      /********************************************************************/
      }
   /*** ASSUME HAVE A CORRECT SURFACE POINTER ***/
   if(surfptr->vinit == TRUE)
      {
      errnum = 2;
      errhand(funcname,errnum);
      return(1);
      }
   ddstruct.opcode = CLEAR;
   (*(surfptr->dev1drive))(&ddstruct);        /*** CLEAR SURFACE FOR GRAPHIC ***/
   ddstruct.opcode = INITIAL;        /*** OUTPUT AND INITIALIZE.    ***/
   (*(surfptr->dev1drive))(&ddstruct);

   /*****************************************/
   /***                                   ***/
   /*** INFORM DEVICE OF NDC SPACE BOUNDS ***/
   /***                                   ***/
   /*****************************************/

   ddstruct.opcode = NDCSP2;
   ddstruct.float1 = ndcspace[0];
   ddstruct.float2 = ndcspace[1];
   (*(surfptr->dev1drive))(&ddstruct);

   surfptr->vinit = TRUE;  /*** MARK SURFACE AS BEING INITIALIZED. ***/
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: selectvwsurface                                            */
/*                                                                          */
/*     PURPOSE: SELECT THE LOGICAL VIEW SURFACE 'surfname' FOR ALL          */
/*              SUBSEQUENT GRAPHIC OUTPUT UNTIL THE SURFACE IS DESELECTED   */
/*              WITH A 'deselectvwsurface' FUNCTION CALL.                   */
/*                                                                          */
/****************************************************************************/

selectvwsurface(surfname)
   char *surfname;
   {
   char *funcname;
   int errnum;

   funcname = "selectvwsurface";

   if(osexists == TRUE)   /*** IS A SEGMENT OPEN? ***/
      {
      errnum = 8;
      errhand(funcname,errnum);
      return(4);
      }
   /*******************************************************************/
   /***                                                             ***/
   /*** SEARCH SURFACE TABLE FOR MATCH TO 'sysnam' THAT CORRESPONDS ***/
   /*** TO SPECIFIED 'surfname'.                                    ***/
   /***                                                             ***/
   /*******************************************************************/

   if(vsnsrch(surfname) == FALSE)
      {
      /********************************************************************/
      /*** SUBROUTINE TO SEARCH LOGICAL NAME TABLE FOR A 'usernam' THAT ***/
      /*** MATCHES 'surfname' SO THAT THE CORRESPONDING 'sysnam' CAN BE ***/
      /*** ACCESSED. IF A MATCH DOESN'T EXIST, THEN ERROR#3 FOR THIS    ***/
      /*** FUNCTION WILL OCCUR.                                         ***/
      /********************************************************************/
      }
   /*** ASSUME HAVE A CORRECT SURFACE POINTER ***/
   if(surfptr->vinit == FALSE)   /*** VIEW SURFACE NOT INITIALIZED? ***/
      {
      errnum = 5;
      errhand(funcname,errnum);
      return(1);
      }
   if(surfptr->selected)         /*** SURFACE ALREADY BEEN SELECTED? **/
      {
      errnum = 6;
      errhand(funcname,errnum);
      return(2);
      }
   surfptr->selected = slectnxt;
   recentvs = surfptr;
   slectnxt++;
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: deselectvwsurface                                          */
/*                                                                          */
/*     PURPOSE: DESELECTS THE LOGICAL VIEW SURFACE "surfname" FOR ALL       */
/*              SUBSEQUENT GRAPHIC OUTPUT UNTIL RESELECTED.                 */
/*                                                                          */
/****************************************************************************/

deselectvwsurface(surfname)
   char *surfname;
   {
   char *funcname;
   int errnum;
   register int highest;
   struct viewsurf *temptr;

   funcname = "deselectvwsurface";

   if(osexists == TRUE)   /*** IS A SEGMENT OPEN? ***/
      {
      errnum = 8;
      errhand(funcname,errnum);
      return(2);
      }
   /*******************************************************************/
   /***                                                             ***/
   /*** SEARCH SURFACE TABLE FOR MATCH TO 'sysnam' THAT CORRESPONDS ***/
   /*** TO SPECIFIED 'surfname'.                                    ***/
   /***                                                             ***/
   /*******************************************************************/
   if(vsnsrch(surfname) == FALSE)
      {
      /********************************************************************/
      /*** SUBROUTINE TO SEARCH LOGICAL NAME TABLE FOR A 'usernam' THAT ***/
      /*** MATCHES 'surfname' SO THAT THE CORRESPONDING 'sysnam' CAN BE ***/
      /*** ACCESSED. IF A MATCH DOESN'T EXIST, THEN ERROR#3 FOR THIS    ***/
      /*** FUNCTION WILL OCCUR.                                         ***/
      /********************************************************************/
      }
   if(!surfptr->selected)
      {
      errnum = 9;
      errhand(funcname,errnum);
      return(1);
      }
   surfptr->selected = 0;
   if(surfptr == recentvs)
      {
      highest = 0;
      for(temptr = &surface[0];temptr < &surface[MAXVSURF];temptr++)
	 {
	 if(temptr->selected > highest)
	    {
	    highest = temptr->selected;
	    recentvs = temptr;
	    }
	 }
      }
   return(0);
   }




/****************************************************************************/
/*                                                                          */
/*     FUNCTION: termvwsurface                                              */
/*                                                                          */
/*     PURPOSE: TERMINATE ACCESS TO THE VIEW SURFACE 'surfname'.            */
/*                                                                          */
/****************************************************************************/

termvwsurface(surfname)
   char *surfname;
   {
   char *funcname;
   int errnum,i;
   int deleteseg();

   funcname = "termvwsurface";

   /*******************************************************************/
   /***                                                             ***/
   /*** SEARCH SURFACE TABLE FOR MATCH TO 'sysnam' THAT CORRESPONDS ***/
   /*** TO SPECIFIED 'surfname'.                                    ***/
   /***                                                             ***/
   /*******************************************************************/

   if(vsnsrch(surfname) == FALSE)
      {
      /********************************************************************/
      /*** SUBROUTINE TO SEARCH LOGICAL NAME TABLE FOR A 'usernam' THAT ***/
      /*** MATCHES 'surfname' SO THAT THE CORRESPONDING 'sysnam' CAN BE ***/
      /*** ACCESSED. IF A MATCH DOESN'T EXIST, THEN ERROR#3 FOR THIS    ***/
      /*** FUNCTION WILL OCCUR.                                         ***/
      /********************************************************************/
      }
   /*** ASSUME HAVE A CORRECT SURFACE POINTER ***/
   if(surfptr->vinit == FALSE)   /*** VIEW SURFACE NOT INITIALIZED? ***/
      {
      errnum = 5;
      errhand(funcname,errnum);
      return(1);
      }
   if(surfptr->selected)         /*** SURFACE CURRRENTLY SELECTED? **/
      {
      errnum = 10;
      errhand(funcname,errnum);
      return(2);
      }
   /***********************************************************************/
   /***                                                                 ***/
   /*** SEARCH FOR SEGMENTS IN THE SEGMENT TABLE WHOSE IMAGES APPEAR ON ***/
   /*** THE SPECIFIED SURFACE. MARK THOSE POINTERS TO THE VIEW SURFACE  ***/
   /*** AS BEING NULL, UNLESS THE SPECIFIED SURFACE IS THE ONLY ONE     ***/
   /*** ASSOCIATED WITH THE SEGMENT, IN WHICH CASE DELETE THE SEGMENT.  ***/
   /***                                                                 ***/
   /***********************************************************************/

   for(segptr = &segment[0];segptr < &segment[SEGNUM];segptr++)
      {
      if(segptr->type >= NORETAIN)
	 {
	 for(i = 0;i < segptr->vsurfnum;i++)
	    {
	    if(segptr->vsurfptr[i] == surfptr)
	       {
	       if(segptr->vsurfnum == 1)
		  {
		  deleteseg(segptr->seg1name);
		  }
		  else
		     {
		     segptr->vsurfptr[i] = NULL;
		     --(segptr->vsurfnum);
		     }
	       break;
	       }
	    }
	 }
      }
   ddstruct.opcode = CLEAR;
   (*(surfptr->dev1drive))(&ddstruct);          /*** CLEAR THE SURFACE AND   ***/
   ddstruct.opcode = TERMINATE;
   (*(surfptr->dev1drive))(&ddstruct);          /*** TERMINATE ACCESS TO IT. ***/
   surfptr->vinit = FALSE;
   return(0);
   }






/****************************************************************************/
/*                                                                          */
/*     FUNCTION: vsnsrch                                                    */
/*                                                                          */
/*     PURPOSE: SEARCH THE SURFACE STRUCTURE FOR SPECIFIED SURFACE NAME.    */
/*                                                                          */
/****************************************************************************/

vsnsrch(surfname)
   char *surfname;
   {
   int found;

   found = FALSE;
   /*******************************************************************/
   /***                                                             ***/
   /*** SEARCH SURFACE TABLE FOR MATCH TO 'sysnam' THAT CORRESPONDS ***/
   /*** TO SPECIFIED 'surfname'.                                    ***/
   /***                                                             ***/
   /*******************************************************************/
   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if(strcmp(surfname,surfptr->name) == 0)
	 {
	 found = TRUE;
	 break;
	 }
      }
   return(found);
   }
