#ifndef lint
static	char sccsid[] = "@(#)ttyvt100.c 1.16 86/06/25 Copyr 1985 MITRE Corp.";
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

#undef DEBUG
int debug =  0;			/* to enable any debugging you must */
int debug2 = 0;			/* Define DEBUG and set one of the */
int debug1 = 0;			/* debug flags                     */
int debug4 = 0;
int debug3 = 0;
int debug5 = 0;
int debug6 = 0;
int debug7 = 0;
int debug8 = 0;
#include <sunwindow/cms_grays.h>
#include "ttysw_impl.h"
#include "ttyvt100.h"
#include "charimage.h"
#include "charscreen.h"
#define vanillaChar(c)	((c >= ' ') && (c <= '~'))
char ansiCharBuf[CHAR_BUF_LEN];
extern struct pixwin *csr_pixwin;/* This is the ttysw windowfd */
extern struct pixfont *pixfont;		/* This is what csr_init uses ???? */
extern int chrwidth;
extern int right,top,bottom,left;
extern pstring(),bold(),nobold();
extern void init_kbd();


/* Logical state of window and program switches */
int  graf_norm, bold_norm;
int curscol = 0;	/* cursor column */
int cursrow = 0;	/* cursor row */
int state = ALPHA;	/* ALPHA,VT52, ESCBRKT, ESCBRKTQM, etc, */
int state1 = 0;			/* holds EATCHARS flag at last column */
int vl = 0;			/* accumulate vt52 first arg */
int ac = 0;	/* accumulates last arg in ESCBRKT sequences. 0-->1. */
int ac0 = 0;			/* ditto, but next to last arg.  0 ==> 1 */
int acinit = 0;			/* initial value of ac (with 0 left alone) */
int acinit0 = 0;			/*  ditto for first Pn */
int acm[10];    		/* array of parameters */
int ac_num = 0;			/* number of parms this time */
int scroll_top = 0;		/* definition of top of scrolling region */
int scroll_bottom = 23;		/* bottom of scrolling region */
int fillfunc = 0;		/* 1 -> boldf */
int underscore;         	/* 1 -> underscore */
int report = 0;			/* 1-7 if host asks for something */

/* dimensions of VT100 window */
int vtop = 0;			/* top row of vt100 window  */
int vbottom = 23;		/* bottom row of vt100 window */
int vleft = 0;			/* left column of vt100 window  */
int vright = 79;		/* right column of regular vt100 window */
int tabArray[MAX_SCREEN_WIDTH]; /* array for setting tabs */

/* VT100 mode variables */
int origin_mode = 0;		/* 0 == reset (screen upper left corner) */
                                /* 1 == set (upper -left within margins) */
int newline = 0;		/* also used by ttysw_main */
int local = 0;			/* 1 = local, 0 = online */
int cursor;			/* 0 -> NOCURSOR, 1 -> UNDERCURSOR, */
				/* 2 -> BLOCKCURSOR */
int curs_key = 0;		/*  ANSI cursor key mode [ or O*/
char answer_message[31];	/* holds the answerback message */
int repeat = 0;			/* not implemented */
int interlace = 0;		/* not implemented */
int vt52mode = 0;		/* emulate a vt52 display */
int appl_key_ansi = 0;		/* ANSI application keypad mode */
int wrap = 0;			/* wrap at margin mode */
int big_screen = 0;		/* 132 col mode */
int smooth_scroll = 0;		/* not implemented */

int rev_screen = 0;		/* reverse initial setup BOW or WOB */
int alt_keypad_52 = 0;		/* vt52 keypad mode */
int half_chrwidth = 5;	/* for 132 col mode */
int full_chrwidth = 8;	/* for 80 col mode */
int twice_chrwidth = 16;	/* for 40 col modes */
int twelve_tenths_chrwidth = 10; /* for 66 col mode */
int  g0,g1;		/* to keep character set modes */
int  activeCharset = G0;	/* graf or asc */
int graph_52 = 0;	/* graphics vt52 mode flag */
char charStr[2] = "c";		/* for writing a single character in vright */
char *charNext;
char *charBufMax;
int curscolStart;

/* we will keep a font designator for each character  */
/* so the repair routine will work. Also, need it for rewrite() */
 
/*
 * Interpret a string of characters of length <len>.  Stash and restore
 * the cursor indicator.
 *
 * Note that characters with the high bit set will not be recognized.
 * This is good, for it reserves them for ASCII-8 X3.64 implementation.
 * It just means all sources of chars which might come here must mask
 * parity if necessary.
 *
 */
ansiinit()
{
	int i;
	underscore = 0;
	curscol = vleft;
	cursrow = vtop;
/*	setupfullgraycolormap(); */
	for (i = 0; i < MAX_SCREEN_WIDTH; i++) tabArray[i] = 0;
	for (i = 0; i < MAX_SCREEN_WIDTH; i = i + 8)  tabArray[i]=1;

	g0 = ASC;
	g1 = GRAPH;
#ifdef DEBUG
	if (debug) printf( "to imageinit...\n");
#endif DEBUG
	imageinit();
	init_kbd();
	set_font(5);
	return(1);
}


/* compatibility kludge */
gfxstring(addr, len)
	char *addr;
	int len;
{
	extern struct ttysubwindow *_ttysw;
	if(len ==1 && *addr == '\014') /* map a ^L into vt100 clear */
	  {
	    addr = "\033[2J";
	    len = 4;
	  }
	ttysw_output(_ttysw, addr, len);
}

ttysw_output(ttysw, addr, len0)
	struct ttysubwindow *ttysw;
	register char *addr;
	int len0;
{
	register char c;
	int i;
	register int len;
	char buffer[30];	/* put string to send back to host here */
	char string[30];
	removeCursor();
	for (len = len0; !ttysw->ttysw_frozen && len > 0; len--) {

				/* this is the main loop */
	c = *addr++;		/* Fetch next char from string */
				/*  and bump pointer */
	switch(state){

	case ALPHA:

		ac = 0;
		acinit = 0;
		acinit0 = 0;
		ac0 = 0;
		ac_num = 0;
		for (i=0; i<9; i++,acm[i]=0); /* clear parm array */

		switch (c) {	/* Do control characters first */
           		case '\000' :   break; /* null character */
			case '\005' :   answerback();		break;
			case CTRL(G):	blinkscreen();		break;
			case CTRL(H):	pos(curscol-1,cursrow);
#ifdef DEBUG
					if(debug)printf("backspace\n");
#endif DEBUG
					break;
			case CTRL(I):	
					for (i = curscol+1; i <= vright; i++){
					  if (tabArray[i])
					    {
					      pos(i,cursrow);
					      break;
					    }
					}
					if(i >= vright)
					  pos(vright, cursrow); 
					break;
				
			case CTRL(J):	/* linefeed */
			case CTRL(K):   /* VT interpret as LF */
			case CTRL(L):   /* FF   "        " "  */ 
					Index();
					break;

			case CTRL(M):	
					if(newline)
					  {
					    pos( 0,cursrow);
					    Index();
					  }
					else pos( 0, cursrow);
					break;
			case CTRL(N):   G1_select();
					break;
			case CTRL(O):   G0_select();
					break;
			case '\033':	state = ESCAPE;
					break;

		        			
			default:   /* c is likely a real alpha character */
					if (c < 32) break; /* guess not! */
					if(curscol > vright) /* means we have written into last column */
					  {
					    switch (wrap)
					      {
					      case 1:
						curscol = 0;
						pos(curscol,cursrow);
						Index();
						break;
					      case 0:                   /* eat the character */
						charStr[0] = c;
						writePartialLine(charStr,vright);
						reflections[cursrow][vright] =  marks[cursrow] + (fillfunc<<8);
						goto loopback;
					      }
					  }
					    
					    /* and now handle the normal case "curscol not vright" */
					curscolStart = curscol;	/* initialize for loop */
					charNext = &ansiCharBuf[0];
					*charNext = c; /* put character into output stream */
					charNext++;	   /* and bump stream pointer */
					marks[cursrow] |= BUSY;
					reflections[cursrow][curscol] =  marks[cursrow] + (fillfunc<<8);
					curscol++;   /* and bump column  */
					*charNext = '\0';
					writePartialLine(&ansiCharBuf[0], curscolStart); /* and write to screen  */
					break;		      
				      }
		break;

	case ESCAPELPRN:
		switch (c){
		case 'A': g0 = UK;
			  grafon();
			  if(activeCharset == G0) set_font(5);
			  break;
			  
		case '1':
		case '2':

		case 'B': g0 = ASC;
			  grafon();
			  if(activeCharset == G0) set_font(5);
			  break;
		case '0': g0 = GRAPH;
			  grafon();
			  if(activeCharset == G0) set_font(5);
			  break;
		default:  break;
			}
		state = ALPHA;
		break;
       case ESCAPERPRN:
		switch (c){
		case 'A': g1 = UK;
			  grafon();
			  if(activeCharset == G1) set_font(5);
			  break;
		case '1':
		case '2':
		case 'B': g1 = ASC;
			  grafon();
			  if(activeCharset == G1) set_font(5);
			  break;
		case '0': g1 = GRAPH;
			  grafon();
			  if(activeCharset == G1) set_font(5);
			  break;
		default:  break;
			}
		state = ALPHA;
		break;
        case ESCAPESHARP:
		switch (c){
		case '8': Escreen();
			  break;
		case '3': mark_top(cursrow);
			  break;
			  
		case '4': mark_bottom(cursrow);
			  break;
		case '5': clear_width(cursrow);
			  break;
		case '6': mark_wide(cursrow);
			  break;
		default:  break;
			}
		state = ALPHA;
		break;
				    

	case ESCBRKT:		/* esc[   accumulate parms */
	case ESCBRKTQM:		/* same logic for esc[? */
		if ('0' <= c && c <= '9') {
		   ac = ((char)ac)*10 + c - '0'; /* char for inline muls */
		   break;
		} else if (c == ';') {
			
			acm[ac_num] = ac;
			ac_num++;
			ac0 = ac; ac = 0;	/* for compat up */
			break;
		} else {
		  if(c != '?'){
		    acinit = ac;		/* got a alpha */
		    acinit0 = ac0;              /* default both Pns */
		    acm[ac_num] = ac;
		    ac_num++;
		    if(ac0 == 0 && ac != 0 && ac_num == 1)
		      {
			ac0 = ac; /* if only one then it isthe first */
			ac = 0;
		      }
		    if(ac0 == 0) ac0 = 1; /* default is 1 not 0 */
		    if(ac ==0) ac = 1;
#ifdef DEBUG
	if(debug) printf("acs %d %d %d %d\n",acinit,ac0,ac,ac_num);
#endif DEBUG
		  }
   if (state == ESCBRKT){
#ifdef DEBUG
   		if(debug) printf("in CSI %c sequence\n",c);
#endif DEBUG
	    switch ( c ) {
			case 'A':	pos(curscol,cursrow-ac0); 	break;
			case 'B':	pos(curscol,cursrow+ac0); 	break;
			case 'C':	pos(curscol+ac0,cursrow); 	break;
			case 'D':	pos(curscol-ac0,cursrow); 	break;
			case 'f':
			case 'H':	abs_pos(ac-1,ac0-1);
					break;
			case 'J':
			 switch(acinit){
			 	case 0:	cleol(cursrow);
					del_lines(cursrow+1,vbottom);
					clearMarks(cursrow+1,vbottom);
					set_font(5);
					break;
				case 1: del_char(vleft,curscol, cursrow);
					del_lines(vtop,cursrow-1);
					clearMarks(vtop,cursrow-1);
					set_font(5);
					break;
				case 2:
 				        del_lines(vtop,vbottom);
					clearMarks(vtop,vbottom);
					set_font(5);
					break;
				}
				break;
			case 'K':
			     switch(acinit){
			     	case 0:
				   cleol(cursrow);
				   break;
				case 1:
				   del_char( vleft, curscol,cursrow);
				   break;
				case 2:
				   del_char(vleft,curscol,cursrow);
				   cleol(cursrow);
				   break;
			       }
					break;
		      case '?': state = ESCBRKTQM;
					break;
/* for now just bold and nobold */
		      case 'm': /* attributes on/off */
				if(ac_num == 0)
				  {
				    acm[0] = acinit;
				    ac_num++;
				  }
				for( i = 0; i < ac_num; i++)
				  {
				    switch(acm[i]){
				    case 0:   All_off();
					      break;
				    case 1:   Bold_on();
					      break;
				    case 4:   Under_on();
					      break;
				    case 5:   Blink_on();
					      break;
				    case 7:   Reverse_on();
					      break;
					    }
				  }
#ifdef DEBUG				
if(debug4) printf("attributes %d %d %d %d %d\n", acm[0],acm[1],acm[2],acm[3],acm[4]);					
#endif DEBUG
				break;
/* reports coming up */						
			case 'c':	/*report what are you? */
					ttysw_input(_ttysw,"\033[?1;0c",7);
					break;
/* tabs  clear */
			case 'g':	if(acinit == 0 ) {
					  tabArray[curscol] = 0;
					  break;}
					if(acinit == 3){
                                        for(i = 0; i<= MAX_SCREEN_WIDTH-1;i++)
					    tabArray[i] = 0;}
					  break;
/* line feed mode set */
			case 'h':	if(acinit == 20) newline = 1;
									break;
/* line feed node reset */
			case 'l':	if(acinit == 20) newline = 0;
									break;
/* status requests from host */
                        case 'n':	if(acinit == 5)
			                 ttysw_input(_ttysw,"\033[0n",4);
				
                                	if(acinit == 6)
					  {
			    sprintf(string,"\033[%u;%uR",cursrow+1,curscol+1);
			    ttysw_input(_ttysw,string,strlen(string));
			  }
									break;
/* define scroll region for indexing */
			case 'r':	set_scroll_region(); break;
/* report terminal parameters */
			case 'x':	if(acinit == 0)
			  ttysw_input(_ttysw,"\033[2;1;1;112;112;1;0x",20);
					if(acinit == 1)
			  ttysw_input(_ttysw,"\033[3;1;1;112;112;1;0x", 20);		  
					break;
			case 'q':	/* led business - ignore */	break;

			}	/* end of ESCBRKT switch */
              if(state == ESCBRKT)  state = ALPHA;
		ac = 0;
		ac0 = 0;
		acinit = 0;
		acinit0 = 0;
		break;
		}		/* not  ESCBRKT must be [? */
	       	
if(state == ESCBRKTQM){		/* must be esc[? */
#ifdef DEBUG
			if(debug)printf("esc-[? %c\n",c);
#endif DEBUG
		switch (c){
		    case 'h':
			switch(acinit){
			    case 1:	curs_key = 1; break;
			    case 3:	big_screen = 1;
					vright = 131;
					del_lines(vtop,vbottom);
					clearMarks(vtop,vbottom);
					ac_num = 0;
					scroll_top = vtop;
					scroll_bottom = vbottom;
					pos(0,0);
					set_font(5);
					break;
			    case 4:	smooth_scroll=1;
					break;
					
			    case 5:	if(rev_screen==0){
			                rev_screen = 1;
					pblack_background();
				      }
					break;
			    case 6:	origin_mode=1;	
			    		pos(vleft,scroll_top);
					break;
			    case 7:	wrap = 1;	break;
			    case 8:	repeat = 1;	break;
			    case 9:	interlace=1;	break;
			    default: break;
			    }
			state = ALPHA;
			    break;
		    case 'l':
		        switch(acinit){
			    case 1:	curs_key = 0;	break;
                            case 2:     vt52mode = 1;
					wrap = 1;
					scroll_top = vtop;
					scroll_bottom = vbottom;
					break;
			    case 3:	big_screen=0;
					vright = 79;
					clearMarks(vtop,vbottom);
					del_lines(vtop,vbottom);
					ac_num = 0;
					scroll_top = vtop;
					scroll_bottom = vbottom;
					pos(0,0);
					set_font(5);
					break;
			    case 4:	smooth_scroll=0;
					break;
			    case 5:	if(rev_screen==1){
			                 rev_screen = 0;
				 	pwhite_background();
				       }
					break;
			    case 6:	origin_mode=0;	
			    		pos(vleft,vtop);
					break;
			    case 7:	wrap = 0;	break;
			    case 8:	repeat = 0;	break;
			    case 9:	interlace = 0;	break;
			    default:	break;
				      }
			state = ALPHA;
			break;
	            default: 	/* esc[? something unknown? */		    
			state = ALPHA;
			    break;
		    }		/* end of esc[? switch */
			state = ALPHA;
	    }			/* end of esc[? conditional */
      }				/* ends esc[ and esc[? section */
		ac = 0;
		ac0 = 0;
		acinit = 0;
		acinit0 = 0;

	    state = ALPHA;	/* following the ansi sequence, return to */
		break;		/* out of state switch */


	      case ESC52Y:	/* gather next 2 chars */
		if(vl) {
		  pos( c - '\040',vl - '\040');
#ifdef DEBUG
		  if(debug1)printf("vt52 position %d %d\n",vl-'\40',c-'\40');
#endif DEBUG
		  vl = 0;
		  state = ALPHA;
		  break;
		}
		else {
		  vl = c;
		  break;
		}
		break;

	      case ESCAPE:	
#ifdef DEBUG
			if(debug)printf("escape-%c\n",c);
#endif DEBUG
		if(vt52mode) {
		             scroll_top = vtop;
			     scroll_bottom = vbottom;
		  switch (c) {
		  case 'A':  pos(curscol,--cursrow); break;
                  case 'B':  pos(curscol,++cursrow); break;
		  case 'C':  pos(++curscol,cursrow); break;
		  case 'D':  pos(--curscol,cursrow); break;
                  case 'F':  graph_52 = 1;           break;
		  case 'G':  graph_52 = 0;           break;
		  case 'H':  pos(0,0);               break;
		  case 'I':  Rindex();               break;
                  case 'J':  cleol(cursrow);
			     del_lines(cursrow+1,vbottom);
			     break;
                  case 'K':  cleol(cursrow);
			     break;
		  case 'Y':  state = ESC52Y;
			     vl = 0;
			     break;
                  case 'Z':  ttysw_input(_ttysw,"\033/Z",3);
			     break;
                  case '<':  vt52mode = 0;
			     break;
                  case '>':  alt_keypad_52 = 0;
			     break;
                  case '=':  alt_keypad_52 = 1;
  			     break;
                  case '1':  graph_52 = 1;
			     break;
                  case '2':  graph_52 = 0;
			     break;
			   }
		  if (state == ESCAPE) state = ALPHA;
		  break;
		}
		else

		switch (c) { 
			/* detect esc left bracket */
			case '[':	
					state = ESCBRKT;
					break;

			case 'D':       Index();		break;
			case 'E':	pos(0,cursrow);
					Index();
								break;
			case 'H':	tabArray[curscol] = 1;
#ifdef DEBUG
				if(debug)printf("tab set at %d\n",curscol);
#endif DEBUG
								break;
			case 'M':	Rindex();		break;
			case 'Z':       ttysw_input(_ttysw,"\033[?1;0c",7);
					break;
			case '7':	Save_cursor();		break;
			case '8':	Restore_cursor();	break;
			case '(': 	/* G0 char set designator */
				state = ESCAPELPRN;
				    break;

			case ')':  /* G1 char set designator */
				    state = ESCAPERPRN;	
				    break;
				    
			case '#':  /*  large char controls */
			         state = ESCAPESHARP;
					break;

			case '1':	/* set graphics option */
					break;
					
			
			case '2':	/* graphics off */
					break;
					
			case '=':	appl_key_ansi = 1;
					break;
					
			case '>':	appl_key_ansi = 0;
					break;
					
			case 'c':	/* reset power up */
					reset();
					break;
					
					

	/* By default, ignore the character after the ESC */
			default:	state = ALPHA;
					break;
		}		/* end plain escape sequence */
		ac = 0;
		ac0 = 0;
		acinit = 0;
		acinit0 = 0;
		if(state == ESCAPE) state = ALPHA;
		break;
		  
	      default:		/* for state switch */
				/* don't think we ever get here */
			state = ALPHA;
			cursor = BLOCKCURSOR;
			break;
	      }			/* end of state switch */
      loopback: continue;

      }				/* end of while loop */
	    			/* get next character from input */
/* Finished all input. If we have been asked for a  report return */
/* appropriate message as function value (hope not much has followed) */
/* EndOuterLoop: */
	drawCursor( cursrow, curscol );
	return(len0);

}


pos(col, row)
	int col, row;
{
  int orow;
	if (col <= vleft)
		col = vleft;
	if (col >= vright)
		col = vright;
			/* mimic vt100 scrolling region anomaly */
	if (row <= scroll_top && cursrow >=  scroll_top)
		row = scroll_top;
	if (row >=scroll_bottom && cursrow <=  scroll_bottom)
		row = scroll_bottom;
	orow = cursrow;
#ifdef DEBUG
  if(row < 0) trap();
  if(debug7) if (col == 0) printf("POS %d: %s\n",row, image[row]);
#endif
  if(row < 0)row = 0;

	cursrow = row;
	curscol = col;
	vpos(row, col); 	/* assume 0,0 upper left for SunW */
	check_marks(row,orow);
}




set_scroll_region()
{
#ifdef DEBUG
if(debug3) printf("entering set_scroll_region %d %d %d\n", acinit, ac, ac0);
#endif DEBUG
if (ac_num == 0 || ac_num == 1 ||(ac_num == 2 && ac == ac0 && ac == 1)) {
  
/* default :: reset to whole screen */
	scroll_top = vtop;
	scroll_bottom = vbottom;
	pos(0,0);
	return; 
	 }
else				/* set scroll region from pars */
if(ac > ac0) {
	scroll_top = ac0-1;	/* scroll region start */
	scroll_bottom = ac-1;	/*  "       "    end */
	pos( 0,origin_mode?scroll_top:0);
	}
#ifdef DEBUG
if (debug3) printf("scroll region %d  %d", scroll_top, scroll_bottom);
#endif DEBUG
}

Index()
{

/* if not at bottom of region, then just move down one line */
#ifdef DEBUG
    if(debug)printf("Index called %d %d\n",curscol,cursrow);
#endif DEBUG
    if (cursrow != scroll_bottom) {
    	pos(curscol,cursrow+1);
	return;
	}
    if (cursrow == scroll_bottom) {
      scroll_up();
      shiftMarksUp();
      check_marks(cursrow,cursrow-1);
    }
}

Rindex()
{
#ifdef DEBUG
    if(debug)printf("Rindex called %d %d\n",curscol,cursrow);
#endif DEBUG
/* if not at top of scroll region, then just move up a line */
    if (cursrow !=  scroll_top) {
    	pos(curscol,cursrow-1);
	return;
	}
    if (cursrow == scroll_top) {
      scroll_down();
      shiftMarksDown();
      check_marks(cursrow,cursrow+1);
    }

}

/* Scroll up one line at the cursor position -- happens on LF at
bottom of the screen or when ESC-E or D commands step at bottom 
margin -- also linefeed at bottom margin if margins set */
scroll_up()
{
  int otop = top;
  int obottom = bottom;
  top = scroll_top;
  bottom = scroll_bottom;
  scroll1up(top,bottom);
  top = otop;
  bottom = obottom;

}

scroll_down()			/* Same as up except that we have to  */
{				/* get rid of lines off bottom of screen */
  int otop = top;
  int obottom = bottom;
  top = scroll_top;
  bottom = scroll_bottom;
  scroll1dn(top, bottom);
  top = otop;
  bottom = obottom;

}

int ocursrow, ocurscol, obold, ocharset,og0,og1;
    
Save_cursor()
{
    ocursrow = cursrow;
    ocurscol = curscol;
    obold = fillfunc;
    ocharset = activeCharset;
    og0 = g0;
    og1 = g1;
    
}

Restore_cursor()
{
    cursrow = ocursrow;
    curscol = ocurscol;
    fillfunc = obold;
    if(fillfunc & BOLD) Bold_on();
    if(fillfunc & UNDER) Under_on();
    if(fillfunc & REVERSE)  Reverse_on();
    activeCharset = ocharset;
    grafon();
    g0 = og0;
    g1 = og1;
    set_font(5);
    
}

char * estring = "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";


Escreen()
{
    int i = 0;
    newline = 0;
    origin_mode = 0;
    wrap = 1;
    curscol = 0;
    cursrow = 0;
    scroll_top = 0;
    scroll_bottom = 23;
    del_lines(vtop,vbottom);
    clearMarks(vtop,vbottom);
    estring[vright+1] = '\0';
    while (i <= vbottom){
          pos(0,i);
	  writePartialLine(estring,0);
	  i++;
      }
    estring[vright+1] = 'E';
    pos(0,0);
  }
  


answerback()
{
  ttysw_input(_ttysw,answer_message,strlen(answer_message));
}


reset()
{
  newline = 0;
  wrap = 1;
  origin_mode = 0;
  big_screen = 0;
  curscol = 0;
  cursrow = 0;
  fillfunc = 0;
  scroll_top = 0;
  scroll_bottom = 23;
  del_lines(vtop,vbottom);
  clearMarks(vtop,vbottom);
  vt52mode = 0;
  set_font(5);
  pos(curscol,cursrow);
}
char* blanks = "                                                                                                                                       ";
del_lines(from,to)
int from,to;
{
  int i, inverse;
  int savec = curscol;
  int savel = cursrow;
#ifdef DEBUG
  if(debug) printf("deleting from %d to %d\n",from, to);
#endif DEBUG
  inverse = fillfunc & REVERSE;
  nobold();
  blanks[vright+1] = '\0';
  for(i= from; i<=to; i++)
    {
     pos(0,i);
/*     writePartialLine(blanks,0); */
     vsetlinelength(image[i],0);
   }
  pclearscreen(from, to+1);
  blanks[vright +1] = ' ';
  if(inverse) bold();
  pos(savec,savel);
}

char buf[MAX_SCREEN_WIDTH];	    


/* Writes blanks into line and keeps cursor at original position */

del_char(from,to,row)
int from,to,row;
{
  int savec,savel,inverse;
  register i = to - from +1;
  savec = curscol;
  savel = cursrow;
  nobold();
  cursrow = row;
  blanks[i] = '\0';
  writePartialLine(blanks, from);
  blanks[i] = ' ';
  if(fillfunc & REVERSE) bold(); /* restore reverse if set */
  pos(savec,savel);		
}

cleol(row)
int row;
{
  int i;
  del_char(curscol, vright, row);
  vsetlinelength(image[row],curscol);

}


      

/*special routine for vt100 abs cursor position  */

abs_pos(col,row)
int row, col;
{
  int srow;
  int orow;
  orow = cursrow;
  if (origin_mode){
    srow = row + scroll_top;
    if(srow > scroll_bottom) srow = scroll_bottom;
  }
  else srow = row;
  curscol = col;
  cursrow = srow;

#ifdef DEBUG
  if(cursrow < 0 ) trap();
  if(debug)printf("moving to col %d, row %d \n",col,srow);
#endif DEBUG
  vpos(srow,col);
  check_marks(cursrow,orow);
}

trap(){}
  
pwhite_background()
{
/*  pw_blackonwhite(csr_pixwin, 0,1); */
  pw_writebackground(csr_pixwin, 0 , 0,
		     winwidthp, winheightp, PIX_NOT(PIX_DST)); 


      }

pblack_background()
{
/*  pw_whiteonblack(csr_pixwin,0,1); */
  pw_writebackground(csr_pixwin, 0, 0,
		     winwidthp, winheightp, PIX_NOT(PIX_DST));
}

mark_top(row)			/* top of double height row */
int row;
{
  int i;
  marks[row] &= ~ BOTTOM_F;
  marks[row] |= TOP_F|WIDE_F;
  for (i = 0; i <= 131; i++){
    reflections[row][i] &= ~ BOTTOM_F;
    reflections[row][i] |= TOP_F|WIDE_F ;
  }
  if(length(image[row]) > big_screen ? 66 : 40)
    vsetlinelength(image[row],big_screen ? 66 : 40);
  rewrite(row);
}


mark_bottom(row)		/* bottom of double height row */
int row;
{
  int i;
  marks[row] &= ~ TOP_F;
  marks[row] |= BOTTOM_F|WIDE_F;
  for (i = 0; i <= 131; i++)
    {
      reflections[row][i] &= ~TOP_F;
      reflections[row][i] |= BOTTOM_F|WIDE_F;
    }
  if(length(image[row]) > big_screen ? 66 : 40)
    vsetlinelength(image[row],big_screen ? 66 : 40);
  rewrite(row);
}


clear_width(row)		/* set row to non-widte ype */
int row;
{
  int i;
  marks[row] &= ~(WIDE_F|TOP_F|BOTTOM_F);
  for (i = 0; i <= 131; i++)
    {
      reflections[row][i] &= ~(WIDE_F|TOP_F|BOTTOM_F);
    }
  
  rewrite(row);		
}


mark_wide(row)		/* double width -single height row */
int row;
{
  int i;
  marks[row] &= ~(BOTTOM_F|TOP_F);
  marks[row] |= WIDE_F;
  for (i = 0; i <= 131; i++)
    {
      reflections[row][i] &= ~(BOTTOM_F|TOP_F);
      reflections[row][i] |= WIDE_F;
    }
  if(length(image[row]) > big_screen ? 66 : 40)
    vsetlinelength(image[row],big_screen ? 66 : 40);
  rewrite(row);
}


G0_select()
{
#ifdef DEBUG
  if(debug4) printf("selecting G0 font which is %d\n",G0);
#endif
  activeCharset = G0;
  set_font(5);
  return;
}

G1_select()
{
#ifdef DEBUG
  if(debug4) printf("selecting G1 font which is %d\n", G1);
#endif
  activeCharset = G1;
  set_font(5);
  return;
}









shiftMarksUp()			/* LF or Index has dropped top line */
{
  int i,j;			
  for(i = scroll_top; i < scroll_bottom; i++)
    {
      marks[i] = marks[i+1];
      for (j = 0; j < 132; j++) reflections[i][j] = reflections[i+1][j];
    }
  marks[scroll_bottom] = big_screen ? NARROW_F : NORMAL_F;/* New line is 80 or 132 cols */
  for (j = 0 ; j < 132; j++) reflections[scroll_bottom][j] = marks[scroll_bottom];
}
	
shiftMarksDown()	/* scroll down has removed bottom line */
{	
  int i,j;
  for ( i = scroll_bottom; i > scroll_top; i--)
    {
      marks[i] = marks[i-1];
      for ( j = 0; j < 132; j++) reflections[i][j] = reflections[i-1][j];
    }
  marks[scroll_top] = big_screen ? NARROW_F : NORMAL_F;
  for(j = 0; j < 132; j++) reflections[scroll_top][j] = marks[scroll_top];
}

clearMarks(from,to)
int from,to;
{
  int i,j;
  for(i = from; i <= to ; i++)
    {
      marks[i] =big_screen ? NARROW_F : NORMAL_F;
      for (j = 0; j< 132; j++) reflections[i][j] = marks[i];
    }

}

  
All_off()			/* turn off all attributes */
{
#ifdef DEBUG
  if(debug4) printf("All_off entered\n");
#endif
  fillfunc = 0;
  underscore = 0;
  nobold();
  set_font(5);
}

Bold_on()
{
#ifdef DEBUG
  if(debug4) printf("Bold_on entered\n");
#endif
  fillfunc |= BOLD;  
  set_font(5);

}

Reverse_on()
{
#ifdef DEBUG
  if(debug4) printf("Reverse_on entered\n");
#endif
  fillfunc |= REVERSE;
  bold();
}
Blink_on()			/* can't figure out how to do this */
{
  fillfunc |= BLINK;
}

Under_on()
{
  underscore = 1;
  fillfunc |= UNDER;
}


scroll1up(tp,bm)
int tp,bm;
{
  int i;
  char *line;
  swapregions ( tp , tp + 1, bm - tp);
  line = image[bm];
  for(i = 0; i <= right; i++) line[i] = ' ';
  vsetlinelength(image[bm],0);
/*  pcopyscreen(tp+1,tp,bm-tp);  */
  pw_copy(csr_pixwin, col_to_x(vleft), row_to_y(tp), winwidthp,
	  row_to_y(bm -tp), PIX_SRC,
	  csr_pixwin, col_to_x(vleft), row_to_y(tp + 1));
  pclearline(left,right, bm);
  
}
scroll1dn(tp,bm)
int tp,bm;
{
  int i,k;
  char *line;
  swapnregions(bm, bm - 1,bm - tp);
  line = image[tp];
  for(i = 0; i <= right; i++) line[i] = ' ';
  vsetlinelength(image[tp],0);
/*  pcopyscreen(tp, tp+1, bm-tp);*/
  pw_copy(csr_pixwin, col_to_x(vleft), row_to_y(tp+1), winwidthp,
	  row_to_y(bm -tp), PIX_SRC,
	  csr_pixwin, col_to_x(vleft), row_to_y(tp));
  
  pclearline(left,right,tp);
}



swapnregions(a, b, n)		/* move block down */
	int a, b, n;
{
	while (n--)
		swap(a--, b--);
}

int
setupfullgraycolormap()

{


	register u_char	red[CMS_GRAYSSIZE], green[CMS_GRAYSSIZE],
	    blue[CMS_GRAYSSIZE]; 
	register i;

	/*
	 * Initialize to gray cms.
	 */
/*	pw_setcmsname(csr_pixwin, CMS_GRAYS); */
	cms_grayssetup(red,green,blue);
	pw_putcolormap(csr_pixwin, 0, CMS_GRAYSSIZE, red, green, blue);
	return(CMS_GRAYSSIZE);
}


