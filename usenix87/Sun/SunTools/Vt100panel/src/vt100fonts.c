#ifndef lint
static	char sccsid[] = "@(#)vt100fonts.c 1.9 86/05/02 Copyr 1985 MITRE Corp";
#endif

/*
 * Copyright (c) 1985 by MITRE Corporation
 */

#include <stdio.h>
#include <pixrect/pixrect_hs.h>
#include <sunwindow/window_hs.h>
#include <signal.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <sun/fbio.h>
#include <sundev/kbio.h>
#include <sundev/kbd.h>

#include "ttysw_impl.h"
#include "ttyvt100.h"
#include "charimage.h"
#include "charscreen.h"
#define vanillaChar(c)	((c >= ' ') && (c <= '~'))
extern struct pixwin *csr_pixwin;/* This is the ttysw windowfd */
extern struct pixfont *pixfont;		/* This is what csr_init uses ???? */
extern int chrwidth, underscore;
extern pstring(),bold(),nobold();
extern int debug,vright,cursrow;
extern int fillfunc,bold_norm,graf_norm, g1,g0,full_chrwidth, activeCharset;
extern int vtop,vbottom,vleft,vright;

struct vtfont {			/* hold vt100 font info */
  char fontfile_name[15];	/* name of file in font directory */
  struct pixfont * fontfd;	/* handle after pf_open returns, else 0 */
  int charwidth;		/* width of this font in pixels */
  int line_length;		/* length of vt100 line  */
} fonttab[32] = {
  "gacha.r.8",		0,	8,	79, /* regular  */
  "thin.r.5",		0,	5,	131,/* thin  */
  "wide.r.16",		0,	16,	39, /* wide  */
  "thinwide.r.10",	0,	10,	65, /* wide big screen */
  "widetop.r.16",	0,	16,	39, /* top half */
  "thintop.r.10",	0,	10,	65, /* top half big screen */
  "widebot.r.16",	0,	16,	39, /* bottom half */
  "thinbot.r.10",	0,	10,	65, /* bottom half big screen */
  "gacha.g.8",		0,	8,	79, /* start grafix fonts */
  "thin.g.5",		0,	5,	131,
  "wide.g.16",		0,	16,	39,
  "thinwide.g.10",	0,	10,	65,
  "widetop.g.16",	0,	16,	39,
  "thintop.g.10",	0,	10,	65,
  "widebot.g.16",	0,	16,	39,
  "thinbot.g.10",	0,	10,	65,
  "gacha.b.8",		0,	8,	79, /* start bold fonts */
  "thin.b.6",		0,	5,	131,
  "wide.b.16",		0,	16,	39,
  "thinwide.b.10",	0,	10,	65,
  "widetop.b.16",	0,	16,	39,
  "thintop.b.10",	0,	10,	65,
  "widebot.b.16",	0,	16,	39,
  "thinbot.b.10",	0,	10,	65,
  "gacha.bg.8",		0,	8,	79, /* start bold & grafix */
  "thin.bg.6",		0,	5,	131,
  "wide.bg.16",		0,	16,	39,
  "thinwide.bg.10",	0,	10,	65,
  "widetop.bg.16",	0,	16,	39,
  "thintop.bg.10",	0,	10,	65,
  "widebot.bg.16",	0,	16,	39,
  "thinbot.bg.10",	0,	10,	65
  };

  
/* Finds and sets up fonts and flags for rewriting lines */

find_font(value)
unsigned value;
{
  unsigned type, attrib;
  int  fontindex = 0;
#ifdef DEBUG
if (debug6) printf ("entered find_font with arg: %x\n",value);
#endif
  type = value & 0x7F;
  attrib = (value & 0xFF00) >> 8;
#ifdef DEBUG
if(debug5) printf( "type == %x, attrib = %x\n",type, attrib);
#endif DEBUG
  if(attrib & REVERSE) bold();
  else nobold();
  if(attrib & UNDER) underscore = 1;
  else underscore = 0;
  if(type & NARROW_F) fontindex++;
  if(type & HALF_MASK ) 	/* One of the halfs */
    fontindex += 4;
  else if(type & WIDE_F)
    fontindex += 2;
  if(type &  BOTTOM_F) fontindex += 2;
  if(attrib & BOLD) fontindex += 16;
  if(type & GRAFIX_F) fontindex += 8;
  lookup_font(fontindex);
  return(fontindex);
}
 

lookup_font(index)
     int index;
{
  int returncode = 0;		/* means can't open font */
  if(fonttab[index].fontfd == 0) /* not open yet */
    {
      if(fontopen(index) != 0) 	returncode = -1;
    }
  pixfont = fonttab[index].fontfd; /* set up for new font parameters */
  chrwidth = fonttab[index].charwidth;
  vright = fonttab[index].line_length;
/*  vsetlinelength(image[cursrow], vright);*/
  return(returncode);
}

char*  main_font_directory;

fontopen(index)
     int index;
{
  char  *p, fullname[60];
  int debug = 1;
  main_font_directory = getenv ("VTFONTS");
  strcpy(fullname, main_font_directory);
  strcat(fullname,"/");
  strcat(fullname,fonttab[index].fontfile_name);
  if((fonttab[index].fontfd = pf_open(fullname)) == NULL)
    {
      if(debug) printf("can't open %s font \n",fonttab[index].fontfile_name);
      return(-1);
    }
  return(0);
}

set_font(value)
int  value;
{
  int type;
  grafon();
  bold_norm = (fillfunc & BOLD) ? 1 : 0;
  if(bold_norm == 1) marks[cursrow] |= BOLD_F;
  else marks[cursrow] &= ~BOLD_F;
  if(graf_norm == 2) marks[cursrow] |= GRAFIX_F;
  else marks[cursrow] &= ~GRAFIX_F;
  type = marks[cursrow];
  type |= (fillfunc << 8);
  find_font( type);

}
/* This routine will watch the line font marks as we go to another row*/
/* and change fonts, calling rewrite if necessary */
/* Hopefully mostly simple cases where no font changes are  */
/* required. Next best case is where no wide or double chars are */
/* involved where we just change to correct font. Worst case is doubles. */
/* where we call rewrite() to possibly rewrite entire line in "new" font.*/

check_marks(now, was)
int now, was;
{
  int type;

#ifdef DEBUG
  if(debug4) printf("entering check_marks with marks %x, %x  \n",marks[was],marks[now]);
#endif

  if((marks[now]& ~BUSY) == (marks[was] & ~BUSY)) return;
  if(graf_norm == 2) marks[now] |= GRAFIX_F;
  else marks[now] &= ~GRAFIX_F;
  type = marks[now] + ((fillfunc & 0xFF) << 8);
  find_font( type);
  return;
}

rewrite (row)			/* if a double is called after line */
int row;			/* has been written  */

{
  char  save_image[133], chp[133];
  char *chpp, *imagep;
  int savecol,l, i,k, ochrwidth, olinelength, opixfont;
  short type, save_graf,save_bold,save_attrib;
  if(marks[row] & BUSY == 0) {		/* nothing in the line yet */
    set_font(5);
    return;
  }
  /* see long comment below for reason why we save these here */
  set_font(5);
  save_attrib = fillfunc;
  save_graf = graf_norm;
  save_bold = bold_norm;
  savecol = curscol;
  strncpy(save_image,image[row],132); 
#ifdef DEBUG
if(debug7) printf("IMAGE %d:%s \n",row, image[row]);
#endif
  opixfont = (int) pixfont;	/* remember font parameters */
  olinelength = vright;
  ochrwidth = chrwidth;
  lookup_font(0);		/* use standard font to blank line */
  pclearline(0,79,row);		/* clear out the old text */
  pixfont = (struct pixfont *) opixfont; /* reset font stuff */
  chrwidth = ochrwidth;
  vright = olinelength;
#ifdef DEBUG
if(debug5) printf("rewrite row with %x font marks\n", marks[row]);
#endif
  chpp = &chp[0];
  imagep = &save_image[0];
  for (i = 0, k = 0; *imagep ;i++ )
    {
      *chpp++ = *imagep++;
      if(reflections[row][i] != reflections[row][i+1])
	{
	  *chpp = '\0';
	  find_font(reflections[row][k]);
	  curscol = k;
	  writePartialLine(&chp[0], k);
	  chpp = &chp[0];
	  k = i + 1;
	}
    }
  *chpp = '\0';
  find_font(reflections[row][k]);
  curscol = k;
  writePartialLine(&chp[0], k);
  vsetlinelength(image[row], i);



  /* ********************************************************* */
  /* Since the attributes and graphics mode may have been set */
  /* after the text which we have just rewritten was originally */
  /* written, we have to save and restore them before and after */
  /* rewriting the line and we must also reset the font from those */
  /* parameters........................*/
  /* ************************************************************** */
  curscol = savecol;		/* restore current flags and column */
  graf_norm = save_graf;
  fillfunc = save_attrib;
  bold_norm = save_bold;
  if(graf_norm == 2) marks[cursrow] |= GRAFIX_F;
  else marks[cursrow] &= ~GRAFIX_F;
  type = marks[cursrow] + ((fillfunc & 0xFF) << 8);
  find_font(type);		/* and put font right for where we are */
#ifdef DEBUG
if(debug7) printf("image %d:%s \n",row,image[row]);
#endif
				
}

resquirt(row)
     int row;
{
  char * chpp, *imagep;
  char chp[133],save_image[133];
  int i, k;
  strncpy(save_image,image[row],133); 
  chpp = &chp[0];
  imagep = &save_image[0];
  for (i = 0, k = 0; *imagep ;i++ )
    {
      *chpp++ = *imagep++;
      if(reflections[row][i] != reflections[row][i+1])
	{
	  *chpp = '\0';
	  find_font(reflections[row][k]);
	  curscol = k;
	  writePartialLine(&chp[0], k);
	  chpp = &chp[0];
	  k = i + 1;
	}
    }
  *chpp = '\0';
  find_font(reflections[row][k]);
  curscol = k;
  writePartialLine(&chp[0], k);
  vsetlinelength(image[row], i);
}
