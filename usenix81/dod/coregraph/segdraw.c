#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
segdraw(asegptr,anindex)
   struct segstruc *asegptr;
   int anindex;
   {
    long offset;
    short sdopcode,n;
    char *sdstring,shortstring[2];
    float x,y,x1,y1,scfactor;
/*  float minx,miny,maxx,maxy;   NEEDED LATER FOR EXTENTS */


    if (!(asegptr->visbil1ty) && !(asegptr->vsurfptr[anindex]->vshardwr))
      {
       return(0);
      }
    else if(asegptr->vsurfptr[anindex]->vshardwr)
      {
       ddstruct.opcode = SETVSBL;
       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
      }
    setmatrix(asegptr);
    offset = asegptr->pdfptr;
    lseek(pdfd,offset,0);
    while (TRUE)
      {
       pdfread(SHORT,&sdopcode);
       if (sdopcode == PDFENDSEGMENT)
	   break;
       switch (sdopcode)
	 {
	  case PDFMOVE:
	    {
	     pdfread(FLOAT,&x);
	     pdfread(FLOAT,&y);
	     if(! idenflag) /** assume no transformation hardware ***/
		{
		 x1 = x;
		 y1 = y;
		 x = x1 * imxform[0][0] + y1 * imxform[1][0] + imxform[2][0];
		 y = x1 * imxform[0][1] + y1 * imxform[1][1] + imxform[2][1];
		}
	     ddstruct.opcode = MOVE;
	     ddstruct.float1 = x;
	     ddstruct.float2 = y;
	     (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFLINE:
	    {
	     pdfread(FLOAT,&x);
	     pdfread(FLOAT,&y);
	     if(! idenflag) /** assume no transformation hardware in storage ***/
		{
		 x1 = x;
		 y1 = y;
		 x = x1 * imxform[0][0] + y1 * imxform[1][0] + imxform[2][0];
		 y = x1 * imxform[0][1] + y1 * imxform[1][1] + imxform[2][1];
		}
	     if ((pdfcurrent.linestyl == SOLID ) ||
		 (asegptr->vsurfptr[anindex]->lshardwr &&
		 pdfcurrent.linestyl < DOTDASHED))
		{
		 ddstruct.opcode = LINE;
		 ddstruct.float1 = x;
		 ddstruct.float2 = y;
		 (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
		}
	     else
		{
		 simline(x,y); /** needs pointer and index ***/
		 /*** This function will simulate a line of the current ***/
		 /*** line style and width by sending simple line com-  ***/
		 /*** mands to the indexed DD as above (MOVE and LINE)  ***/
		 /*** excluding the initial MOVE which is already done  ***/
		}

	    break;
	    }
	  case PDFTEXT:
	    {
	     pdfread(SHORT,&n);
	     pdfread(n,sdstring);
	     *(sdstring + n) = '\0';

	     if ((pdfcurrent.chqualty == LOW) && (asegptr->vsurfptr[anindex]->txhardwr))
		{
		ddstruct.opcode = TEXT;
		ddstruct.string = sdstring;
		ddstruct.logical = TRUE;
		(*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
		}
	     else if ((pdfcurrent.chqualty == MEDIUM) && (asegptr->vsurfptr[anindex]->txhardwr))
		{
		shortstring[1] = '\0';

		for ( ;*sdstring != '\0';sdstring++)
		   {
		   shortstring[0] = *sdstring;

		   ddstruct.opcode = TEXT;
		   ddstruct.logical = FALSE;
		   ddstruct.string = shortstring;
		   (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);

		   x = x + pdfcurrent.chrspace.dx;
		   y = y + pdfcurrent.chrspace.dy;

		   ddstruct.opcode = MOVE;
		   ddstruct.float1 = x;
		   ddstruct.float2 = y;
		   (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);

		   }
		}
	     else /*** must use software generation ***/
		{
		softstring(sdstring); /*** this function sends the appropriate ***/
				    /*** line drawing commands to the DD to  ***/
				    /*** draw the required text on the re-   ***/
				    /*** quired surface using the char params***/
		}

	     break;
	    }
	  case PDFMARKER:
	    {
	     pdfread(SHORT,&n);

	     if (asegptr->vsurfptr[anindex]->txhardwr)
		{
		ddstruct.opcode = MARK;
		ddstruct.string = n;
		(*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
		}
	     else /*** must use software generation ***/
		{
		softmark(n); /*** this function sends the appropriate ***/
			     /*** line drawing commands to the DD to  ***/
			     /*** draw the required marker on the re-   ***/
			     /*** quired surface using the char params***/
		}

	    break;
	    }
	  case PDFCOLOR:
	    {
	    pdfread(FLOAT,&(pdfcurrent.color[0]));
	    pdfread(FLOAT,&(pdfcurrent.color[1]));
	    pdfread(FLOAT,&(pdfcurrent.color[2]));
	    if (asegptr->vsurfptr[anindex]->clhardwr && !rastererase)
	       {
	       ddstruct.opcode = SETCOL;
	       ddstruct.float1 = pdfcurrent.color[0];
	       ddstruct.float2 = pdfcurrent.color[1];
	       ddstruct.float3 = pdfcurrent.color[2];
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFINTENSITY:
	    {
	    pdfread(FLOAT,&(pdfcurrent.intensty));
	    if (asegptr->vsurfptr[anindex]->inhardwr && !rastererase)
	       {
	       ddstruct.opcode = SETINT;
	       ddstruct.float1 = pdfcurrent.intensty;
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFLINESTYLE:
	    {
	    pdfread(SHORT,&(pdfcurrent.linestyl));
	    if (asegptr->vsurfptr[anindex]->lshardwr)
	       {
	       ddstruct.opcode = SETSTYL;
	       ddstruct.int1 = pdfcurrent.linestyl;
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFLINEWIDTH:
	    {
	    pdfread(FLOAT,&(pdfcurrent.linwidth));
	    if (asegptr->vsurfptr[anindex]->lwhardwr)
	       {
	       ddstruct.opcode = SETWIDTH;
	       ddstruct.float1 = pdfcurrent.linwidth;
	       (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	       }
	    break;
	    }
	  case PDFFONT:
	    {
	    pdfread(SHORT,&(pdfcurrent.font));
	    ddstruct.opcode = SETFONT;
	    ddstruct.int1 = pdfcurrent.font;
	    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFSIZE:
	    {
	    pdfread(FLOAT,&(pdfcurrent.height));
	    pdfread(FLOAT,&(pdfcurrent.width));
	    ddstruct.opcode = SETSIZE;
	    /*** must convert character size, which is used for hardware ***/
	    /*** character size determination by DD, by multiplying by   ***/
	    /*** the average of the x and y scale factors (good as any)  ***/
	    scfactor = (asegptr->scale1[0] + asegptr->scale1[1]) / 2.0;
	    pdfcurrent.height = scfactor * pdfcurrent.height;
	    pdfcurrent.width = scfactor * pdfcurrent.width;
	    ddstruct.float1 = pdfcurrent.height;
	    ddstruct.float2 = pdfcurrent.width;
	    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFSPACE:
	    {
	    pdfread(FLOAT,&(pdfcurrent.chrspace.dx));
	    pdfread(FLOAT,&(pdfcurrent.chrspace.dy));
	    ddstruct.opcode = SETANGLE;
	    /*** must convert spacing according to image transformation ***/
	    /*** without translation                                    ***/
	    x1 = pdfcurrent.chrspace.dx;
	    y1 = pdfcurrent.chrspace.dy;
	    pdfcurrent.chrspace.dx = x1 * imxform[0][0] + y1 * imxform[1][0];
	    pdfcurrent.chrspace.dy = x1 * imxform[0][1] + y1 * imxform[1][1];
	    ddstruct.float1 = atan2(pdfcurrent.chrspace.dy,pdfcurrent.chrspace.dx);
	    (*(asegptr->vsurfptr[anindex]->dev1drive))(&ddstruct);
	    break;
	    }
	  case PDFCHARQUALITY:
	    {
	    pdfread(SHORT,&(pdfcurrent.chqualty));
	    break;
	    }
	  case PDFPICKID:
	    {
	    pdfread(SHORT,&(pdfcurrent.pick2id));

	    /******************************************/
	    /***                                    ***/
	    /*** pdfread(FLOAT,&minx);              ***/
	    /*** pdfread(FLOAT,&miny);              ***/
	    /*** pdfread(FLOAT,&maxx);    EXTENTS?? ***/
	    /*** pdfread(FLOAT,&maxy);              ***/
	    /***                                    ***/
	    /******************************************/
	    break;
	    }
	  default:
	    break;
	 }
      }
    setmatrix(openseg);
    return(0);
   }

