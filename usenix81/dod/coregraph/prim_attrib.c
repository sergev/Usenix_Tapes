#include "inclusions.c"


/***********************************************************************/
/*                                                                     */
/*   The following is the set of functions with which the user can set */
/*   current attribute values for static primitive attributes.         */
/*   The functions do four things:                                     */
/*      1. Check validity of attribute value.                          */
/*      2. Check conditions for setting (no open segment)              */
/*      3. Set flag indicating that primitive attribute changed        */
/*      4. Set value.                                                  */
/*                                                                     */
/***********************************************************************/

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



/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setintensity                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setintensity(intensty)
   float intensty;
   {
    char *funcname;
    int errnum;

    funcname = "setintensity";
    if (intensty < minimum.intensty || intensty > maximum.intensty)
       {
	errnum = 27;
	errhand(funcname,errnum);
	return(2);
       }
    if (osexists == FALSE)
       {
	errnum = 26;
	errhand(funcname,errnum);
	return(1);
       }
    intflag = TRUE;
    current.intensty = intensty;
    return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setlinwidth                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setlinwidth(linwidth)
   float linwidth;
   {
    char *funcname;
    int errnum;

    funcname = "setlinwidth";
    if (linwidth < minimum.linwidth || linwidth > maximum.linwidth)
       {
	errnum = 27;
	errhand(funcname,errnum);
	return(2);
       }
    if (osexists == FALSE)
       {
	errnum = 26;
	errhand(funcname,errnum);
	return(1);
       }
    lwflag = TRUE;
    current.linwidth = linwidth;
    return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setlinstyle                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setlinstyle(linestyl)
   int linestyl;
   {
    char *funcname;
    int errnum;

    funcname = "setlinstyle";
    if (linestyl < minimum.linestyl || linestyl > maximum.linestyl)
       {
       errnum = 27;
       errhand(funcname,errnum);
       return(2);
       }
    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(1);
       }
    lsflag = TRUE;
    current.linestyl = linestyl;
    return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setfont                                                    */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setfont(font)
   int font;
   {
    char *funcname;
    int errnum;

    funcname = "setfont";
    if (font < minimum.font || font > maximum.font)
       {
       errnum = 27;
       errhand(funcname,errnum);
       return(2);
       }
    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(1);
       }
    fntflag = TRUE;
    current.font = font;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setchsize                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setchsize(chwidth,cheight)
   float chwidth,cheight;
   {
    char *funcname;
    int errnum;

    funcname = "setchsize";
    if (chwidth < minimum.charsize.width || cheight < minimum.charsize.height)
       {
       errnum = 27;
       errhand(funcname,errnum);
       return(2);
       }
    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(1);
       }
    szeflag = TRUE;
    current.charsize.width = chwidth;
    current.charsize.height = cheight;
    return(0);
   }


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





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setchquality                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setchquality(chqualty)
   int chqualty;
   {
    char *funcname;
    int errnum;

    funcname = "setchquality";
    if (chqualty < minimum.chqualty || chqualty > maximum.chqualty)
       {
       errnum = 27;
       errhand(funcname,errnum);
       return(errnum);
       }
    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(errnum);
       }
    current.chqualty = chqualty;
    qualflag = TRUE;
    return(0);
   }




/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setpickid                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setpickid(pickid)
   int pickid;
   {
    char *funcname;
    int errnum;

    funcname = "setpickid";
    if (pickid < minimum.pick2id)
       {
       errnum = 27;
       errhand(funcname,errnum);
       return(2);
       }
    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(1);
       }
    picflag = TRUE;
    current.pick2id = pickid;
    return(0);
   }











/****************************************************************************/
/***                                                                      ***/
/*** THE FOLLOWING IS A SET OF FUNCTION WITH WHICH THE USER CAN INQUIRE   ***/
/*** OF THE CURRENT ATTRIBUTE VALUES FOR STATIC PRIMITIVE ATTRIBUTES.     ***/
/***                                                                      ***/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqcolor                                                   */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqcolor(color)
   float color[]; /*** pointer to array of three reals passed ***/
   {
    color[0] = current.color[0];
    color[1] = current.color[1];
    color[2] = current.color[2];
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqintensity                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqintensity(intensty)
   float *intensty;
   {
    *intensty = current.intensty;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqlinwidth                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqlinwidth(linwidth)
   float *linwidth;
   {
    *linwidth = current.linwidth;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqlinstyle                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqlinstyle(linestyl)
   int *linestyl;
   {
    *linestyl = current.linestyl;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqfont                                                    */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqfont(font)
   int *font;
   {
    *font = current.font;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqchsize                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqchsize(chwidth,cheight)
   float *chwidth,*cheight;
   {
    *chwidth = current.charsize.width;
    *cheight = current.charsize.height;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqchspc                                                   */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqchspc(dx,dy,dz)
   float *dx,*dy,*dz;
   {
    *dx = current.chrspace.dx;
    *dy = current.chrspace.dy;
    *dz = 0.0;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqchquality                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqchquality(chqualty)
   int *chqualty;
   {
    *chqualty = current.chqualty;
    return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqpickid                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqpickid(pickid)
   int *pickid;
   {
    *pickid = current.pick2id;
    return(0);
   }













