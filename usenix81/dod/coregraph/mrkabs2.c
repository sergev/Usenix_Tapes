#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: mrkabs2                                                    */
/*                                                                          */
/*     PURPOSE: THE CP IS UPDATED TO (x,y) AND THEN THE SYSTEM-DEFINED      */
/*              MARKER 'n',CENTERED AT THE TRANSFORMED POSITION OF THE CP,  */
/*              IS CREATED.                                                 */
/*                                                                          */
/****************************************************************************/

mrkabs2(mx,my,n)
   float mx,my;
   short n;
   {
   char *funcname;
   int errnum;
   float x,y,xnew,ynew;
   int flag;

   funcname = "mrkabs2";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }
   if (n < 1)
      {
      errnum = 25;
      errhand(funcname,errnum);
      return(2);
      }
   if ((n > stdmarkr) && (n < MINASCII))
      {
      errnum = 25;
      errhand(funcname,errnum);
      /*** do not return, merely report and replace with a question mark ***/
      n = QUESTION;
      }
   if (n > MAXASCII)
      {
      errnum = 25;
      errhand(funcname,errnum);
      /*** do not return, merely report and replace with a question mark ***/
      n = QUESTION;
      }

   if(wndwclip)
      {
      clippt(mx,my,&x,&y,&flag); /*** point outside window? ***/
      if (flag)
	 {
	 return(0);
	 }
      }
   else
      {
      x = mx;
      y = my;
      }
   wldtondc(x,y,&x,&y); /*** world to normalized device coordinates ***/
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
      if (cpchang || !(mx == cp[0] && my == cp[1]))  /*** if the cp has been changed since last put in pdf ***/
	 {
	 ptype = PDFMOVE;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(FLOAT,&x);
	 pdfwrite(FLOAT,&y);
	 }
      ptype = PDFMARKER;
      pdfwrite(SHORT,&ptype);
      pdfwrite(SHORT,&n);
      }
   if(! idenflag)
      {
      if(openseg->type == XLATE2)
	 {
	 /***********************************/
	 /***                             ***/
	 /*** PERFORM A TRANSLATION ONLY. ***/
	 /***                             ***/
	 /***********************************/

	 x = x + imxform[2][0];
	 y = y + imxform[2][1];
	 }
      else if(openseg->type == XFORM2)
	 {
	 /********************************************/
	 /***                                      ***/
	 /*** PERFORM A FULL IMAGE TRANSFORMATION: ***/
	 /*** ROTATE,SCALE, AND TRANSLATE.         ***/
	 /***                                      ***/
	 /********************************************/

	 xnew = x * imxform[0][0] + y * imxform[1][0] + imxform[2][0];
	 ynew = x * imxform[0][1] + y * imxform[1][1] + imxform[2][1];
	 x = xnew;
	 y = ynew;
	 }
      }

/****************************************************************************/
/***from here on this function is essentially the same as TEXT except that***/
/***even if the DD supports text, it is only expected to generate ASCII   ***/
/***characters, not special characters. Others are done by software.      ***/
/***The DD is issued a MARK opcode rather than a TEXT so that it will use ***/
/***default character sizes and orientations and the cp is the center of  ***/
/***the character.                                                        ***/
/****************************************************************************/

   for (surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if (surfptr->selected)
	 {
	 if (picflag && (surfptr->idhardwr)) /*** pick id changed or new segment ***/
	    {
	    ddstruct.opcode = SETPID;
	    ddstruct.string = current.pick2id;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if (intflag) /*** intensity or color changed or new segment ***/
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
	 if(cpchang || !(mx == cp[0] && my == cp[1]))
	    {
	    ddstruct.opcode = MOVE;
	    ddstruct.float1 = x;
	    ddstruct.float2 = y;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if (n >= MINASCII && n <= MAXASCII && surfptr->txhardwr)
	    {
	    ddstruct.opcode = MARK;
	    ddstruct.string = n;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 else /*** must use software generation ***/
	    {
	    softmark(n); /*** this function sends the appropriate ***/
			 /*** line drawing commands to the DD to  ***/
			 /*** draw the required marker on the re-   ***/
			 /*** quired surface using the char params***/
	    }
	 }
      }

   intflag = FALSE;
   clrflag = FALSE;
   picflag = FALSE;
   cpchang = FALSE;
   opsegemp = FALSE;

   cp[0] = mx;
   cp[1] = my;

   return(0);
   }


