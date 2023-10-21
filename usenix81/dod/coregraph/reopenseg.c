#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
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



/****************************************************************************/
