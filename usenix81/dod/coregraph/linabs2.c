#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: linabs2                                                    */
/*                                                                          */
/*     PURPOSE: DESCRIBE A LINE OF AN OBJECT IN WORLD COORDINATES.THIS LINE */
/*              RUNS FROM CP TO THE POSITION SPECIFIED. CP IS UPDATED.      */
/*                                                                          */
/****************************************************************************/

linabs2(x,y)
   float x,y;
   {
   char *funcname;
   int errnum;
   register int index;
   int flag;
   float x1,y1,x2,y2;
   float x1new,y1new,x2new,y2new;
   float dummyx,dummyy;

   funcname = "linabs2";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }
   if ((cp[0] == x) && (cp[1] == y))
      {
      return(0); /*** no lines of zero length ***/
      }
   if(wndwclip)
      {
      clipline(x,y,&x1,&y1,&x2,&y2,&flag);
      if (flag) /*** line totally clipped outside of window ***/
	 {
	 cp[0] = x;
	 cp[1] = y;
	 cpchang = TRUE;
	 return(0);
	 }
      }
   else
      {
      x1 = cp[0];
      y1 = cp[1];
      x2 = x;
      y2 = y;
      }
   wldtondc(x1,y1,&x1,&y1); /*** world to normalized device coordinates ***/
   wldtondc(x2,y2,&x2,&y2);
   if (openseg->type >= RETAIN)
      {
      if (picflag) /*** pick id changed or new segment ***/
	 {
	 ptype = PDFPICKID;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(SHORT,&current.pick2id);
	 /*** PICK-ID EXTENTS WILL ADDED LATER ***/
	 }
      if (intflag) /*** intensity changed or new segment ***/
	 {
	 ptype = PDFINTENSITY;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(FLOAT,&current.intensty);
	 }
      if (clrflag) /*** color changed ***/
	 {
	 ptype = PDFCOLOR;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(FLOAT,&current.color[0]);
	 pdfwrite(FLOAT,&current.color[1]);
	 pdfwrite(FLOAT,&current.color[2]);
	 }
      if (lsflag) /*** line style changed ***/
	 {
	 ptype = PDFLINESTYLE;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(SHORT,&current.linestyl);
	 }
      if (lwflag) /*** line width changed ***/
	 {
	 ptype = PDFLINEWIDTH;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(FLOAT,&current.linwidth);
	 }
      if (cpchang)  /*** if the cp has been changed since last put in pdf ***/
	 {
	 ptype = PDFMOVE;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(FLOAT,&x1);
	 pdfwrite(FLOAT,&y1);
	 }
      ptype = PDFLINE;
      pdfwrite(SHORT,&ptype);
      pdfwrite(FLOAT,&x2);
      pdfwrite(FLOAT,&y2);
      }
/*** before sending to the devices we must use image transformation on primitives ***/
   if(! idenflag)
      {
      if(openseg->type == XLATE2)
	 {
	 /***********************************/
	 /***                             ***/
	 /*** PERFORM A TRANSLATION ONLY. ***/
	 /***                             ***/
	 /***********************************/

	 x1 = x1 + imxform[2][0];
	 y1 = y1 + imxform[2][1];

	 x2 = x2 + imxform[2][0];
	 y2 = y2 + imxform[2][1];
	 }
      else if(openseg->type == XFORM2)
	 {
	 /********************************************/
	 /***                                      ***/
	 /*** PERFORM A FULL IMAGE TRANSFORMATION: ***/
	 /*** ROTATE,SCALE, AND TRANSLATE.         ***/
	 /***                                      ***/
	 /********************************************/

	 x1new = x1 * imxform[0][0] + y1 * imxform[1][0] + imxform[2][0];
	 y1new = x1 * imxform[0][1] + y1 * imxform[1][1] + imxform[2][1];

	 x2new = x2 * imxform[0][0] + y2 * imxform[1][0] + imxform[2][0];
	 y2new = x2 * imxform[0][1] + y2 * imxform[1][1] + imxform[2][1];
	 x1 = x1new;
	 y1 = y1new;
	 x2 = x2new;
	 y2 = y2new;
	 }
      }

   for (surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if (surfptr->selected)
	 {
	 if (picflag && (surfptr->idhardwr)) /*** pick id changed or new segment ***/
	    {
	    ddstruct.opcode = SETPID;
	    ddstruct.int1 = current.pick2id;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if (intflag) /*** intensity changed or new segment ***/
	    {
	    if (surfptr->inhardwr)
	       {
	       ddstruct.opcode = SETINT;
	       ddstruct.float1 = current.intensty;
	       (*(surfptr->dev1drive))(&ddstruct);
	       }
	    }
	 if (clrflag) /*** color changed ***/
	    {
	    if (surfptr->clhardwr)
	       {
	       ddstruct.opcode = SETCOL;
	       ddstruct.float1 = current.color[0];
	       ddstruct.float2 = current.color[1];
	       ddstruct.float3 = current.color[2];
	       (*(surfptr->dev1drive))(&ddstruct);
	       }
	    }
	 if (lsflag) /*** line style changed ***/
	    {
	    if (surfptr->lshardwr)
	       {
	       ddstruct.opcode = SETSTYL;
	       ddstruct.int1 = current.linestyl;
	       (*(surfptr->dev1drive))(&ddstruct);
	       }
	    }
	 if (lwflag) /*** line width changed ***/
	    {
	    if (surfptr->lwhardwr)
	       {
	       ddstruct.opcode = SETWIDTH;
	       ddstruct.float1 = current.linwidth;
	       (*(surfptr->dev1drive))(&ddstruct);
	       }
	    }
	 if (cpchang)  /*** cp changed since last sent to DD ***/
	    {
	    ddstruct.opcode = MOVE;
	    ddstruct.float1 = x1;
	    ddstruct.float2 = y1;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if ((current.linestyl == SOLID ) || (surfptr->lshardwr && current.linestyl < DOTDASHED))
	    {
	    ddstruct.opcode = LINE;
	    ddstruct.float1 = x2;
	    ddstruct.float2 = y2;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 else
	    {
	    simline(x1,y1,x2,y2);
	    /*** This function will simulate a line of the current ***/
	    /*** line style and width by sending simple line com-  ***/
	    /*** mands to the indexed DD as above (MOVE and LINE)  ***/
	    /*** excluding the initial MOVE which is already done  ***/
	    }
	 }
      }

   intflag = FALSE;
   clrflag = FALSE;
   lsflag = FALSE;
   lwflag = FALSE;
   picflag = FALSE;
   opsegemp = FALSE;
   cpchang = FALSE;

   /**************************************************************/
   /***                                                        ***/
   /*** IF SPECIFIED ENDPOINT OF LINE SEGMENT IS OUTSIDE OF    ***/
   /*** CURRENT WINDOW, THE PDF WILL NOT PROPERLY REFLECT THE  ***/
   /*** VALUE FOR THE CP;THEREFORE FLAG MUST BE SET TO SIGNIFY ***/
   /*** THE NEEDED UPDATE BEFORE NEXT OUTPUT PRIMITIVE IS      ***/
   /*** WRITTEN TO THE PDF.                                    ***/
   /***                                                        ***/
   /**************************************************************/

   if(wndwclip)
      {
      clippt(x,y,&dummyx,&dummyy,&flag);
      if(flag)
	 {
	 cpchang = TRUE;
	 }
      }

   cp[0] = x;
   cp[1] = y;

   return(0);
   }


