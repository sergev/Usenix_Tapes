#include "inclusions.c"

double sin(),cos(),atan2();

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

