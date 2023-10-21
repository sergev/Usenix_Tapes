#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: softstring                                                 */
/*                                                                          */
/*     PURPOSE: USING THE CHARACTER PARAMETERS, SIMULATE TEXT DRAWING BY    */
/*              SENDING THE APPROPRIATE LINE COMMANDS TO THE SURFACE.       */
/*                                                                          */
/****************************************************************************/

softstring(string)
   char *string;
   {

   struct
   {
    int c0 : 4;
    int c1 : 4;
    int c2 : 4;
    int c3 : 4;
    int c4 : 4;
    int c5 : 4;
    int c6 : 4;
    int c7 : 4;
    int c8 : 4;
    int c9 : 4;
    int c10 : 4;
    int c11 : 4;
    int c12 : 4;
    int c13 : 4;
    int c14 : 4;
    int c15 : 4;
   } character_strokes[128]; /*** some ascii, others marks ***/

    int stroke_index, command, pen_down,lcflag;
    float position[2], h, w;
    float upx,upy,lfx,lfy,uplfx,uplfy,uprtx,uprty;
    float upx2,upy2,lfx2,lfy2,dnx4,dny4;
    float upxlc,upylc,lfxlc,lfylc,uplfxlc,uplfylc,uprtxlc,uprtylc;
    float upx2lc,upy2lc,lfx2lc,lfy2lc,dnx4lc,dny4lc;
    float ccdx,ccdy,hyp,sine,cosine;
    double sqrt();

/******************** SIMULATE AN 'A',ASCII CODE 65. ************************/
/******************** CAN'T BE TESTED UNTIL VERSION 7 ***********************/
   character_strokes[65].c0 = PENDOWN;
   character_strokes[65].c1 = UP2;
   character_strokes[65].c2 = UP;
   character_strokes[65].c3 = UPRIGHT;
   character_strokes[65].c4 = RIGHT2;
   character_strokes[65].c5 = DOWNRIGHT;
   character_strokes[65].c6 = DOWN2;
   character_strokes[65].c7 = DOWN;
   character_strokes[65].c8 = PENUP;
   character_strokes[65].c9 = UP2;
   character_strokes[65].c10 = PENDOWN;
   character_strokes[65].c11 = LEFT2;
   character_strokes[65].c12 = LEFT2;
   character_strokes[65].c13 = STOP;
/****************************************************************************/

    h = current.charsize.height;
    w = current.charsize.width;
    ccdx = current.chrspace.dx;
    ccdy = current.chrspace.dy;

    hyp = sqrt(ccdx * ccdx + ccdy * ccdy); /*** is there a square root function ? ***/
    cosine = ccdx / hyp;
    sine = ccdy / hyp;

    upx = h / 4. * sine;
    upy = h / 4. * cosine;
    lfx = -(w / 4. * cosine);
    lfy = -(w / 4. * sine);

    upx2 = upx * 2;
    upy2 = upy * 2;
    dnx4 = -upx * 4;
    dny4 = -upy * 4;
    lfx2 = lfx * 2;
    lfy2 = lfy * 2;

    uplfx = upx + lfx;
    uplfy = upy + lfy;
    uprtx = upx - lfx;
    uprty = upy - lfy;

    upxlc = upx/2.;
    upylc = upy/2.;
    lfxlc = lfx/2.;
    lfylc = lfy/2.;

    upx2lc = upx2/2.;
    upy2lc = upy2/2.;
    dnx4lc = dnx4/2.;
    dny4lc = dny4/2.;
    lfx2lc = lfx2/2.;
    lfy2lc = lfy2/2.;

    uplfxlc = uplfx/2.;
    uplfylc = uplfy/2.;
    uprtxlc = uprtx/2.;
    uprtylc = uprty/2.;

    position[0] = cp[0];
    position[1] = cp[1];

    for (;*string != '\0';string++)
       {
	pen_down = FALSE;

	lcflag = FALSE;
	if(*string >= 'a' && *string <= 'z')   /*** LOWER CASE LETTER ? ***/
	   {
	   lcflag = TRUE;
	   }

	for (stroke_index = 0; stroke_index < 18; stroke_index++)
	   {
	    switch(stroke_index)
	       {
		case 0: command = character_strokes[*string].c0;
			break;
		case 1: command = character_strokes[*string].c1;
			break;
		case 2: command = character_strokes[*string].c2;
			break;
		case 3: command = character_strokes[*string].c3;
			break;
		case 4: command = character_strokes[*string].c4;
			break;
		case 5: command = character_strokes[*string].c5;
			break;
		case 6: command = character_strokes[*string].c6;
			break;
		case 7: command = character_strokes[*string].c7;
			break;
		case 8: command = character_strokes[*string].c8;
			break;
		case 9: command = character_strokes[*string].c9;
			break;
		case 10: command = character_strokes[*string].c10;
			 break;
		case 11: command = character_strokes[*string].c11;
			 break;
		case 12: command = character_strokes[*string].c12;
			 break;
		case 13: command = character_strokes[*string].c13;
			 break;
		case 14: command = character_strokes[*string].c14;
			 break;
		case 15: command = character_strokes[*string].c15;
			 break;
		default: break;
	       }
	    if (command == 14) break;
	    switch(command)
	       {
		case 0: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(upxlc,upylc);
			     }
			  else slin2rel(upx,upy);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] + upxlc;
			   cp[1] = cp[1] + upylc;
			   }
			else
			   {
			   cp[0] = cp[0] + upx;
			   cp[1] = cp[1] + upy;
			   }
			break;
		case 1: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(-upxlc,-upylc);
			     }
			  else slin2rel(-upx,-upy);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] - upxlc;
			   cp[1] = cp[1] - upylc;
			   }
			else
			   {
			   cp[0] = cp[0] - upx;
			   cp[1] = cp[1] - upy;
			   }
			break;
		case 2: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(lfxlc,lfylc);
			     }
			  else slin2rel(lfx,lfy);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] + lfxlc;
			   cp[1] = cp[1] + lfylc;
			   }
			else
			   {
			   cp[0] = cp[0] + lfx;
			   cp[1] = cp[1] + lfy;
			   }
			break;
		case 3: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(-lfxlc,-lfylc);
			     }
			  else slin2rel(-lfx,-lfy);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] - lfxlc;
			   cp[1] = cp[1] - lfylc;
			   }
			else
			   {
			   cp[0] = cp[0] - lfx;
			   cp[1] = cp[1] - lfy;
			   }
			break;
		case 4: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(uprtxlc,uprtylc);
			     }
			  else slin2rel(uprtx,uprty);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] + uprtxlc;
			   cp[1] = cp[1] + uprtylc;
			   }
			else
			   {
			   cp[0] = cp[0] + uprtx;
			   cp[1] = cp[1] + uprty;
			   }
			break;
		case 5: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(-uplfxlc,-uplfylc);
			     }
			  else slin2rel(-uplfx,-uplfy);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] - uplfxlc;
			   cp[1] = cp[1] - uplfylc;
			   }
			else
			   {
			   cp[0] = cp[0] - uplfx;
			   cp[1] = cp[1] - uplfy;
			   }
			break;
		case 6: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(-uprtxlc,-uprtylc);
			     }
			  else slin2rel(-uprtx,-uprty);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] - uprtxlc;
			   cp[1] = cp[1] - uprtylc;
			   }
			else
			   {
			   cp[0] = cp[0] - uprtx;
			   cp[1] = cp[1] - uprty;
			   }
			break;
		case 7: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(uplfxlc,uplfylc);
			     }
			  else slin2rel(uplfx,uplfy);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] + uplfxlc;
			   cp[1] = cp[1] + uplfylc;
			   }
			else
			   {
			   cp[0] = cp[0] + uplfx;
			   cp[1] = cp[1] + uplfy;
			   }
			break;
		case 8: pen_down = FALSE;
			break;
		case 9: pen_down = TRUE;
			break;
		case 10: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(upx2lc,upy2lc);
			     }
			  else slin2rel(upx2,upy2);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] + upx2lc;
			   cp[1] = cp[1] + upy2lc;
			   }
			else
			   {
			   cp[0] = cp[0] + upx2;
			   cp[1] = cp[1] + upy2;
			   }
			break;
		case 11: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(-upx2lc,-upy2lc);
			     }
			  else slin2rel(-upx2,-upy2);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] - upx2lc;
			   cp[1] = cp[1] - upy2lc;
			   }
			else
			   {
			   cp[0] = cp[0] - upx2;
			   cp[1] = cp[1] - upy2;
			   }
			break;
		case 12: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(lfx2lc,lfy2lc);
			     }
			  else slin2rel(lfx2,lfy2);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] + lfx2lc;
			   cp[1] = cp[1] + lfy2lc;
			   }
			else
			   {
			   cp[0] = cp[0] + lfx2;
			   cp[1] = cp[1] + lfy2;
			   }
			break;
		case 13: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(-lfx2lc,-lfy2lc);
			     }
			  else slin2rel(-lfx2,-lfy2);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] - lfx2lc;
			   cp[1] = cp[1] - lfy2lc;
			   }
			else
			   {
			   cp[0] = cp[0] - lfx2;
			   cp[1] = cp[1] - lfy2;
			   }
			break;
		case 14: break; /*** should already have broken above ***/
		case 15: if (pen_down)
			  {
			  if(lcflag)
			     {
			     slin2rel(dnx4lc,dny4lc);
			     }
			  else slin2rel(dnx4,dny4);
			  }
			else if(lcflag)
			   {
			   cp[0] = cp[0] + dnx4lc;
			   cp[1] = cp[1] + dny4lc;
			   }
			else
			   {
			   cp[0] = cp[0] + dnx4;
			   cp[1] = cp[1] + dny4;
			   }
			break;
		default: break;
	       }
	   }

	    position[0] = position[0] + current.chrspace.dx;
	    position[1] = position[1] + current.chrspace.dy;

	    movabs2(position[0],position[1]);
       }

   return(0);
   }





