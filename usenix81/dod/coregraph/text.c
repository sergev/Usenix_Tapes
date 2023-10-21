#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: text                                                       */
/*                                                                          */
/*     PURPOSE: THE CHARACTER STRING IS DRAWN IN THE WORLD COORDINATE       */
/*              SYSTEM.                                                     */
/*                                                                          */
/****************************************************************************/

text(string)
   char *string;
   {
   char *funcname;
   char shortstring[2];
   int errnum,strlength;
   float x,y,chposx,chposy,ndcpx,ndcpy,wldcpx,wldcpy;
   float angle;
   float hw,hv,hxw,hyw,hxv,hyv; /* height in window,height in viewport,x vector of window height,y vector of window height,(same for viewport) */
   float ww,wv,wxw,wyw,wxv,wyv; /* (substitute width for height in above line to get respective mneumonics) */
   float csxw,csyw,csxv,csyv;   /* character spacing of x vector in window, etc. */
   float csxvnew,csyvnew,xnew,ynew;
   float hvtr,wvtr;             /* height of viewport image transformed,width of viewport image transformed */
   int flag;
   double atan2(),sin(),cos(),sqrt();

   funcname = "text";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }
   if(strchk(string) == 2)
      {
      errnum = 23;
      errhand(funcname,errnum);
      return(errnum);
      }

/*** error 3 cannot occur because this is a 2D implementation ***/

/*** for low and medium quality text the DD is notified of the***/
/*** text size so that it can choose from among its set of    ***/
/*** hardware generated character sizes.                      ***/

   if (current.chqualty == LOW && wndwclip)
      {
      clippt(cp[0],cp[1],&x,&y,&flag); /*** point outside window? ***/
      if (flag)
	 {
	 return(0);
	 }
      }
   else
      {
      x = cp[0];
      y = cp[1];
      }
   wldtondc(x,y,&x,&y); /*** world to normalized device coordinates ***/

   /***********************************************************************/
   /***                                                                 ***/
   /*** VIEW TRANSFORM CHARACTER SIZING PARAMETERS. FIRST GET THE ANGLE ***/
   /*** OF ROTATION FOR EACH CHARACTER. THEN COMPUTER X AND Y DIRECTION ***/
   /*** COMPONENT VECTORS OF THE HEIGHT VECTOR IN THE WINDOW. TRANSFORM ***/
   /*** THOSE WINDOW VECTORS INTO VIEWPORT VECTORS, FROM WHICH THE      ***/
   /*** HEIGHT VECTOR FOR THE VIEWPORT CAN BE CALCULATED. SAME HOLDS    ***/
   /*** TRUE FOR THE WIDTH                                              ***/
   /***                                                                 ***/
   /***********************************************************************/

   hw = current.charsize.height;
   ww = current.charsize.width;

   csxw = current.chrspace.dx;
   csyw = current.chrspace.dy;

   angle = atan2(csyw,csxw);

   hxw = -hw * sin(angle);
   hyw = hw * cos(angle);

   hxv = hxw * vwxform[0][0];
   hyv = hyw * vwxform[1][0];

   hv = sqrt(hxv * hxv + hyv * hyv);

   wxw = ww * cos(angle);
   wyw = ww * sin(angle);

   wxv = wxw * vwxform[0][0];
   wyv = wyw * vwxform[1][0];

   wv = sqrt(wxv * wxv + wyv * wyv);

   /***************************************************/
   /***                                             ***/
   /*** VIEW TRANSFORM CHARACTER SPACING PARAMETERS ***/
   /***                                             ***/
   /***************************************************/

   csxv = csxw * vwxform[0][0];
   csyv = csyw * vwxform[1][0];


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
      if (fntflag) /*** font properties changed ***/
	 {
	 ptype = PDFFONT;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(SHORT,&current.font);
	 }
      if (szeflag) /*** size properties changed ***/
	 {
	 ptype = PDFSIZE;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(FLOAT,&(hv));
	 pdfwrite(FLOAT,&(wv));
	 }
      if (angflag) /*** angle properties changed ***/
	 {
	 ptype = PDFSPACE;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(FLOAT,&(csxv));
	 pdfwrite(FLOAT,&(csyv));
	 }
      if (qualflag) /*** character quality changed ***/
	 {
	 ptype = PDFCHARQUALITY;
	 pdfwrite(SHORT,&ptype);
	 pdfwrite(SHORT,&current.chqualty);
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
	 pdfwrite(FLOAT,&x);
	 pdfwrite(FLOAT,&y);
	 }
      ptype = PDFTEXT;
      pdfwrite(SHORT,&ptype);
      strlength = length(string);
      pdfwrite(SHORT,&strlength);
      pdfwrite(strlength,string);
      }

      /*********************************************************************/
      /***                                                               ***/
      /*** NOTE: STRING IS BEING PLACED IN PDF AS A WHOLE NO MATTER WHAT ***/
      /***       THE CHARACTER QUALITY TO SAVE SPACE AND ACCESSES TO     ***/
      /***       FILE EVEN THOUGH CALCULATIONS FOR MEDIUM QUALITY TEXT   ***/
      /***       WILL HAVE TO BE PERFORMED AGAIN UPON READING IN STRING  ***/
      /***       FROM PDF DURING A REPAINT. (CONSIDERED LESS EXPENSIVE)  ***/
      /***                                                               ***/
      /*********************************************************************/

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


	 /****************************************************/
	 /***                                              ***/
	 /*** IMAGE TRANSFORM CHARACTER SPACING PARAMETERS ***/
	 /***                                              ***/
	 /*** NOTE: TRANSLATION VALUES ARE NON-ESSENTIAL   ***/
	 /***       HERE; THUS NOT A FULL TRANSFORMATION.  ***/
	 /***                                              ***/
	 /****************************************************/

	 csxvnew = csxv * imxform[0][0] + csyv * imxform[1][0];
	 csyvnew = csxv * imxform[0][1] + csyv * imxform[1][1];
	 csxv = csxvnew;
	 csyv = csyvnew;

	 }
      }

   /***************************************************/
   /***                                             ***/
   /*** IMAGE TRANSFORM CHARACTER SIZING PARAMETERS ***/
   /***                                             ***/
   /*** NOTE: ONLY SCALING VAUES ARE NECESSARY.     ***/
   /***       SCALE VALUES OF OPPOSITE SIGNS WILL   ***/
   /***       CAUSE UNPREDICTABLE RESULTS.          ***/
   /***                                             ***/
   /***************************************************/

   hvtr = hv * ((openseg->scale1[0] + openseg->scale1[1])/2);
   wvtr = wv * ((openseg->scale1[0] + openseg->scale1[1])/2);



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
	 if(fntflag)   /*** FONT PROPERTY CHANGED ? ***/
	    {
	    ddstruct.opcode = SETFONT;
	    ddstruct.int1 = current.font;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if(szeflag)   /*** SIZE PROPERTY CHANGED ? ***/
	    {
	    ddstruct.opcode = SETSIZE;
	    ddstruct.float1 = hvtr;
	    ddstruct.float2 = wvtr;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if(angflag)   /*** ANGLE PROPERTY CHANGED ? ***/
	    {
	    ddstruct.opcode = SETANGLE;
	    ddstruct.float1 = atan2(csyv,csxv);
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if (cpchang)  /*** cp changed since last sent to DD ***/
	    {
	    ddstruct.opcode = MOVE;
	    ddstruct.float1 = x;
	    ddstruct.float2 = y;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 if ((current.chqualty == LOW) && (surfptr->txhardwr))
	    {
	    ddstruct.opcode = TEXT;
	    ddstruct.string = string;
	    ddstruct.logical = TRUE;
	    (*(surfptr->dev1drive))(&ddstruct);
	    }
	 else if ((current.chqualty == MEDIUM) && (surfptr->txhardwr))
	    {
	    chposx = cp[0];
	    chposy = cp[1];
	    shortstring[1] = '\0';
	    for ( ;*string != '\0';string++)
	       {
	       shortstring[0] = *string;

	       ddstruct.opcode = TEXT;
	       ddstruct.string = shortstring;
	       ddstruct.logical = FALSE;

	       if(wndwclip)
		  {
		  clipbox(chposx,chposy,&flag); /*** character box outside window? ***/
		  if (!flag)
		     {
		     (*(surfptr->dev1drive))(&ddstruct);
		     }
		  }
	       else
		  {
		  (*(surfptr->dev1drive))(&ddstruct);
		  }

	       x = x + csxv;
	       y = y + csyv;

	       chposx = chposx + csxw;
	       chposy = chposy + csyw;

	       ddstruct.opcode = MOVE;
	       ddstruct.float1 = x;
	       ddstruct.float2 = y;
	       (*(surfptr->dev1drive))(&ddstruct);

	       }
	    }
	 else /*** must use software generation ***/
	    {
	    softstring(string); /*** this function sends the appropriate ***/
				/*** line drawing commands to the DD to  ***/
				/*** draw the required text on the re-   ***/
				/*** quired surface using the char params***/
	    }
	 /**********************************************************/
	 /***                                                    ***/
	 /*** RETRIEVE CP COORDINATES FROM DD, PERFORM A REVERSE ***/
	 /*** IMAGE TRANSFORMATION ON THEM, AND CONVERT BACK TO  ***/
	 /*** WORLD COORDINATES.                                 ***/
	 /***                                                    ***/
	 /**********************************************************/

	 ddstruct.opcode = GETCP;
	 (*(surfptr->dev1drive))(&ddstruct);

	 ndcpx = ddstruct.float1;
	 ndcpy = ddstruct.float2;

	 if(! idenflag)
	    {
	    inverse(ndcpx,ndcpy,&ndcpx,&ndcpy);
	    }
	 ndctowld(ndcpx,ndcpy,&wldcpx,&wldcpy);
	 }
      }

   intflag = FALSE;
   clrflag = FALSE;
   lsflag = FALSE;
   lwflag = FALSE;
   picflag = FALSE;
   fntflag = FALSE;
   szeflag = FALSE;
   angflag = FALSE;
   qualflag = FALSE;
   opsegemp = FALSE;
   cpchang = FALSE;

   cp[0] = wldcpx;
   cp[1] = wldcpy;

   return(0);
   }


