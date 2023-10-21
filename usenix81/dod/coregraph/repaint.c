#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
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


