#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setchspc                                                   */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setchspc(dx,dy,dz)
   float dx,dy,dz;
   {
    char *funcname;
    int errnum;

    funcname = "setchspc";
    /************************************************************/
    /***                                                      ***/
    /*** all values are legal for character space coordinates ***/
    /***                                                      ***/
    /************************************************************/

    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(1);
       }
    angflag = TRUE;
    current.chrspace.dx = dx;
    current.chrspace.dy = dy;
    /****************************************************************/
    /***                                                          ***/
    /*** the dz value is not used since implementation is only 2D ***/
    /***                                                          ***/
    /****************************************************************/

    return(0);
   }





