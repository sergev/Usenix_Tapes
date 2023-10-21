#ifndef lint
static	char sccsid[] = "@(#)vt100keys.c 1.6 86/04/11 Copyr 1985 MITRE Corp";
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
#include "ttyvt100.h"
#include "charimage.h"
#include "charscreen.h"
extern int vt52mode, curs_key,alt_keypad_52,appl_key_ansi;
/* This routine takes function keys and converts to vt100 key-strings */
/* These depend on the mode settings: */
/* CURSOR KEYS: */
/* vt52_mode */
/* ANSI mode && curs_key mode */
/* ANSI mode && not curs_key mode */
/* AUXILIARY KEYPAD: */
/* vt52mode && numeric */
/* vt52mode && application */
/* ANSI && numeric */
/* ANSI && application */
/********************************* */
extern int newline;

char *
translate_key(code)
int code;
{
  int flag = 0;
  int flagc = 0;
  /*  Now set up from mode flags: */
  flagc = vt52mode;		/* that's a 1 0r 0 */

#ifdef DEBUG
if(debug2)printf("entered translate with %d\n",code);
#endif DEBUG

  if(!vt52mode)  flagc = 2 + curs_key; /* makes it 2 or 3 */
  if(vt52mode && alt_keypad_52) flag = APPL5;
  if(vt52mode && !alt_keypad_52)flag = NUMER5;
  if(!vt52mode && appl_key_ansi) flag = APPLA;
  if(!vt52mode && !appl_key_ansi) flag = NUMERA;
  switch(code){
    /* Left panel except L1 which */
    /* is the hardware abort control (when combined with "a")*/
  case   KEY_LEFT(2):       
  case   KEY_LEFT(3):       
  case   KEY_LEFT(4):       
  case   KEY_LEFT(5):       
  case   KEY_LEFT(6):       
  case   KEY_LEFT(7): 
  case   KEY_LEFT(8):
  case   KEY_LEFT(9):
  case   KEY_LEFT(10): return("");

    /* right panel but not the cursor keys yet */
    /* we will use all 15 as keypad PF1--15*/

  case   KEY_RIGHT(1): /* PF1 */
		  switch(flag){
		  case NUMER5: return("\033P");
		  case NUMERA: return("\033OP");
		  case APPL5:  return("\033P");
		  case APPLA:  return("\033OP");
		  default:  break;
			  }
		  return("");
  case   KEY_RIGHT(2): /* PF2 */
		  switch(flag){
		  case NUMER5: return("\033Q");
		  case NUMERA: return("\033OQ");
		  case APPL5:  return("\033Q");
		  case APPLA:  return("\033OQ");
		  default:  break;
			  }
		  return("");
  case   KEY_RIGHT(3): /* PF3 */
		  switch(flag){
		  case NUMER5: return("\033R");
		  case NUMERA: return("\033OR");
		  case APPL5:  return("\033R");
		  case APPLA:  return("\033OR");
		  default:  break;
			  }
		  return("");
  case   KEY_RIGHT(4): /*  7  */
		  switch(flag){
		  case NUMER5: return("7");
		  case NUMERA: return("7");
		  case APPL5:  return("\033?w");
		  case APPLA:  return("\033Ow");
		  default:  break;
			  }
		  return("");
  case   KEY_RIGHT(5): /*  8  */
		  switch(flag){
		  case NUMER5: return("8");
		  case NUMERA: return("8");
		  case APPL5:  return("\033?x");
		  case APPLA:  return("\033Ox");
		  default:  break;
			  }
		  return("");
  case   KEY_RIGHT(6): /*  9  */
                  switch(flag){
		  case NUMER5: return("9");
		  case NUMERA: return("9");
		  case APPL5:  return("\033?y");
		  case APPLA:  return("\033Oy");
		  default:  break;
			  }
		  return("");

  case   KEY_RIGHT(7): /*  4  */
		  switch(flag){
		  case NUMER5: return("4");
		  case NUMERA: return("4");
		  case APPL5:  return("\033?t");
		  case APPLA:  return("\033Ot");
		  default:  break;
			  }
		  return("");

  case   KEY_RIGHT(8): /*  5  */
		  switch(flag){
		  case NUMER5: return("5");
		  case NUMERA: return("5");
		  case APPL5:  return("\033?u");
		  case APPLA:  return("\033Ou");
		  default:  break;
			  }
		  return("");


  case   KEY_RIGHT(9): /*  6  */
		  switch(flag){
		  case NUMER5: return("6");
		  case NUMERA: return("6");
		  case APPL5:  return("\033?v");
		  case APPLA:  return("\033Ov");
		  default:  break;
			  }
		  return("");

  case   KEY_RIGHT(10): /* 1  */
		  switch(flag){
		  case NUMER5: return("1");
		  case NUMERA: return("1");
		  case APPL5:  return("\033?q");
		  case APPLA:  return("\033Oq");
		  default:  break;
			  }
		  return("");

  case   KEY_RIGHT(11): /* 2  */
		  switch(flag){
		  case NUMER5: return("2");
		  case NUMERA: return("2");
		  case APPL5:  return("\033?r");
		  case APPLA:  return("\033Or");
		  default:  break;
			  }
		  return("");

  case   KEY_RIGHT(12): /* 3  */
		  switch(flag){
		  case NUMER5: return("3");
		  case NUMERA: return("3");
		  case APPL5:  return("\033?s");
		  case APPLA:  return("\033Os");
		  default:  break;
			  }
		  return("");

  case   KEY_RIGHT(13): /* 0  */
		  switch(flag){
		  case NUMER5: return("0");
		  case NUMERA: return("0");
		  case APPL5:  return("\033?p");
		  case APPLA:  return("\033Op");
		  default:  break;
			  }
		  return("");

  case   KEY_RIGHT(14): /* .  */
		  switch(flag){
		  case NUMER5: return(".");
		  case NUMERA: return(".");
		  case APPL5:  return("\033?n");
		  case APPLA:  return("\033On");
		  default:  break;
			  }
		  return("");


  case   KEY_RIGHT(15): /*ENTER*/
		  switch(flag){
		  case NUMERA:  
		  case NUMER5: if(newline)
		                  return("\r\n");
			       else return("\r");
		  case APPL5:  return("\033?M");
		  case APPLA:  return("\033OM");
		  default:  break;
			  }
		  return("");

    /* Top panel */
  case   KEY_TOP(1):      return("");
  case   KEY_TOP(2):      return("");
  case   KEY_TOP(3): /*  UP   */ 
		  switch(flagc){
		  case 1: return("\033A");
		  case 2: return("\033[A");
		  case 3:  return("\033OA");
		  default:  break;
			  }
		  return("");

  case   KEY_TOP(4): /* DOWN  */
		  switch(flagc){
		  case 1: return("\033B");
		  case 2: return("\033[B");
		  case 3:  return("\033OB");
		  default:  break;
			  }
		  return("");

  case   KEY_TOP(5): /* LEFT  */
		  switch(flagc){
		  case 1: return("\033D");
		  case 2: return("\033[D");
		  case 3:  return("\033OD");
		  default:  break;
			  }
		  return("");

  case   KEY_TOP(6): /* RIGHT */
		  switch(flagc){
		  case 1: return("\033C");
		  case 2: return("\033[C");
		  case 3:  return("\033OC");
		  default:  break;
			  }
		  return("");

  case   KEY_TOP(7): /* PF4   */
		  switch(flag){
		  case NUMER5: return("\033S");
		  case NUMERA: return("\033OS");
		  case APPL5:  return("\033S");
		  case APPLA:  return("\033OS");
		  default:  break;
			  }
		  return("");

  case   KEY_TOP(8): /*-(dash)*/
		  switch(flag){
		  case NUMER5: return("-");
		  case NUMERA: return("-");
		  case APPL5:  return("\033?m");
		  case APPLA:  return("\033Om");
		  default:  break;
			  }
		  return("");

  case   KEY_TOP(9): /*,comma */
		  switch(flag){
		  case NUMER5: return(",");
		  case NUMERA: return(",");
		  case APPL5:  return("\033?l");
		  case APPLA:  return("\033Ol");
		  default:  break;
			  }
		  return("");
    default: return("");
	      }
}



/* This code puts the keyboard into a form that we can use all */
/*  the keys as PF-keys as a vt100; i.e. disables the arrow notion */
/*  on the keypad. Keys will transmit standard SUN-2 keyboard codes.*/

#define		MAX_keydefs	(sizeof (k_board)/sizeof (*k_board))
int     fbdes,
        kb;



struct fbtype   fb;
struct key_id_num {
	char *name;
	int id_num;
	} k_board [] = {
	{"'", 87},
	{",", 107},
	{"-", 40},
	{".", 108},
	{"/", 109},
	{"0", 39},
	{"1", 30},
	{"2", 31},
	{"3", 32},
	{"4", 33},
	{"5", 34},
	{"6", 35},
	{"7", 36},
	{"8", 37},
	{"9", 38},
	{";", 86},
	{"=", 41},
	{"break", 19},
	{"bs", 43},
	{"del", 66},
	{"esc", 29},
	{"l1", 1},
	{"l10", 97},
	{"l2", 3},
	{"l3", 25},
	{"l4", 26},
	{"l5", 49},
	{"l6", 51},
	{"l7", 72},
	{"l8", 73},
	{"l9", 95},
	{"lf", 111},
	{"r1", 21},
	{"r10", 91},
	{"r11", 92},
	{"r12", 93},
	{"r13", 112},
	{"r14", 113},
	{"r15", 114},
	{"r2", 22},
	{"r3", 23},
	{"r4", 45},
	{"r5", 46},
	{"r6", 47},
	{"r7", 68},
	{"r8", 69},
	{"r9", 70},
	{"ret", 89},
	{"f1", 5},
	{"f2", 6},
	{"f3", 8},
	{"f4", 10},
	{"f5", 12},
	{"f6", 14},
	{"f7", 16},
	{"f8", 17},
	{"f9", 18},
	{"tab", 53},
	{"[", 64},
	{"\\", 88},
	{"]", 65},
	{"`", 42},
	{"a", 77},
	{"b", 104},
	{"c", 102},
	{"d", 79},
	{"e", 56},
	{"f", 80},
	{"g", 81},
	{"h", 82},
	{"i", 61},
	{"j", 83},
	{"k", 84},
	{"l", 85},
	{"m", 106},
	{"n", 105},
	{"o", 62},
	{"p", 63},
	{"q", 54},
	{"r", 57},
	{"s", 78},
	{"t", 58},
	{"u", 60},
	{"v", 103},
	{"w", 55},
	{"x", 101},
	{"y", 59},
	{"z", 100}
};
/* rf8,10,12,and 14 are the culprits. when the kbd is initialized */
/* they transmit an escape code instead of the standard sun-2 position*/
void
init_kbd()
{
		prep_kbd ();	/* prepare keyboard */
		set_key("r8",RF(8));
		set_key("r10",RF(10));
		set_key("r12",RF(12));
		set_key("r14",RF(14));

}

int prep_kbd()
{
    if ((fbdes = open ("/dev/fb", 2)) < 0) {
	fprintf (stderr, "%s: couldn't open framebuffer\n", "ttyvt100");
	exit (2);
    }

    if (ioctl (fbdes, FBIOGTYPE, &fb) < 0) {
	fprintf (stderr, "%s: couldn't get the fb struct\n", "ttyvt100");
	exit (2);
    }

    if (fb.fb_type != FBTYPE_SUN2BW) {
	fprintf (stderr, "Sorry, I only do Sun2s\n", "ttyvt100");
	exit (2);
    }

    if ((kb = open ("/dev/kbd", 2)) < 0) {
	fprintf (stderr, "%s: couldn't open keyboard\n", "ttyvt100");
	exit (2);
    }
    return;
}

int set_key(key, keydef)
char *key;
int keydef;
{
    int     found_key = FALSE,
            i;

    struct kiockey  this_key;
    struct kiockey *pk = &this_key;


    switch (*key) {
	case 'C': 
	    pk -> kio_tablemask = CTRLMASK;
	    key++;
	    break;
	case 'S': 
	    pk -> kio_tablemask = SHIFTMASK;
	    key++;
	    break;
	default: 
	    pk -> kio_tablemask = 0;
	    break;
    }


    for (i = 0; i < MAX_keydefs; i++) {
	if (strncmp (key, k_board[i].name, 3) == 0) {
	    found_key = TRUE;
	    break;
	}
    }

    if (!found_key) {
	fprintf (stderr, "%s: couldn't find `%s' key\n", "ttyvt100", key);
	exit (2);
    }

    pk -> kio_station = k_board[i].id_num;

/*    pk -> kio_tablemask = 0; */


		    pk -> kio_entry = keydef;
		
 
/* define key */

    if (ioctl (kb, KIOCSETKEY, pk) < 0) {
	fprintf (stderr, "%s: couldn't define the `%s' key\n", "ttyvt100", key);
	exit (2);
    }
    return;
}
/* end of the keyboard stuff. only do this on entering. whew! */


