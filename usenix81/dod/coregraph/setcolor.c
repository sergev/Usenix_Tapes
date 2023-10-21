#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setcolor                                                   */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setcolor(color)
   float color[]; /*** pointer to array of three reals passed ***/
   {
    char *funcname;
    int errnum,i;

    funcname = "setcolor";
    for (i = 0;i < 3;i++)
	{
	if (color[i] < minimum.color[i] || color[i] > maximum.color[i])
	   {
	    errnum = 27;
	    errhand(funcname,errnum);
	    return(2);
	   }
	}
    if (osexists == FALSE)
       {
	errnum = 26;
	errhand(funcname,errnum);
	return(1);
       }
    clrflag = TRUE;
    current.color[0] = color[0];
    current.color[1] = color[1];
    current.color[2] = color[2];
    return(0);
   }



