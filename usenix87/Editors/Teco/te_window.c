/* TECO for Ultrix   Copyright 1986 Matt Fichtenbaum						*/
/* This program and its components belong to GenRad Inc, Concord MA 01742	*/
/* They may be copied if this copyright notice is included					*/

/* te_window.c   window for teco   10/10/86 */
/* This attempts to be a real window, without unecessary redraw */
/* it is very VT-100 specific, and ought to be rewritten to be general */

#include "te_defs.h"

/* maximum screen height and width (horiz and vert, not height and vidth) */
#define W_MAX_V 70
#define W_MAX_H 150
#define MAX 0x7fffffff			/* maximum positive integer, for "last modified" location */
#define W_MARK 0200				/* "this loc is special" in screen image */


/* image of current window */

struct w_line			/* data associated with one screen line */
	{
	int start, end;				/* dot at beginning, at end */
	short n, cflag, col;		/* number of char positions used, line continuation flag, starting col */
	char ch[W_MAX_H];			/* image of line */
	}
	 w_image[W_MAX_V];


/* define "this line is continued" / "this line is a continuation" flags */
#define WF_BEG 1
#define WF_CONT 2

struct w_line *wlp[W_MAX_V];	/* each word points to the corresponding line's data structure */

struct qp w_p1;					/* pointer for window access to buffer */

short curr_x, curr_y;			/* active character position */
short term_x, term_y;			/* current terminal cursor position */
short curs_x, curs_y;			/* current teco dot screen coordinates */
short last_y;					/* last used line in window */
char curs_c;					/* code for char at cursor */
char *curs_p;					/* pointer to cursor loc in window image */
short curs_crflag;				/* flag that cursor is on a CR */
short redraw_sw;				/* forces absolute redraw */


/* fill characters and terminal speeds: 0th entry used when std out is not a terminal */
char win_speeds[] = { 0, 0, B9600, B4800, B2400, B1800, B1200, B600, B300, B200, B150, B134, B110 };
char win_dlye[] =   { 0, 90, 45, 23, 11, 9, 6, 3, 1, 1, 1, 1, 1 };	/* delay for erase-screen */
char win_dlys[] =   { 0, 60, 30, 15, 7, 6, 4, 2, 1, 1, 0, 0, 0 };	/* delay for scroll ops */
char win_dlyl[] =   { 0, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		/* delay for erase line */
char win_dlyc[] =   { 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		/* delay for other control functions */
short win_speed;
/* routine to perform simple scope operations */
/* (an attempt to concentrate VT-100 specific things in one place) */

vt(func)
	int func;
	{
	short t;
	switch (func)
		{
		case VT_CLEAR:			/* clear screen */
			fputs("\033[H\033[J", stdout);
			for (t = 0; t < win_dlye[win_speed]; t++) putchar('\0');	/* fill chars */
			break;

		case VT_EEOL:			/* erase to end of line */
			fputs("\033[K", stdout);
			for (t = 0; t < win_dlyl[win_speed]; t++) putchar('\0');	/* fill chars */
			break;

		case VT_EBOL:			/* erase from beginning of line */
			fputs("\033[1K", stdout);
			for (t = 0; t < win_dlyl[win_speed]; t++) putchar('\0');	/* fill chars */
			break;

		case VT_SETSPEC1:		/* reverse video */
			fputs("\033[7m", stdout);
			break;

		case VT_SETSPEC2:		/* bright reverse video */
			fputs("\033[1;7m", stdout);
			break;

		case VT_CLRSPEC:		/* normal video */
			fputs("\033[0m", stdout);
			break;

		case VT_BS1:			/* backspace 1 spot */
			fputs("\b \b", stdout);
			break;

		case VT_BS2:			/* backspace 2 spots */
			fputs("\b \b\b \b", stdout);
			break;

		case VT_LINEUP:			/* up one line */
			fputs("\033[1A", stdout);
			break;
		}
	}
/* routine to set window parameters */

/* 0: scope type, 1: width, 2: height, 3: seeall, 4: mark position,		*/
/* 5: hold mode, 6: upper left corner position, 7: scroll region size	*/

/*				   0   1		2		 3  4	  5   6	   7	*/
int win_min[]  = { 4,  20,		4,		 0, 0,	 -1,  1,   0 } ;	/* min values for window parameters */
int win_max[]  = { 4,  W_MAX_H,	W_MAX_V, 1, MAX, 12, -1,  20 } ;	/* max values */
int win_data[] = { 4,  132,		24,		 0, 0,	  0,  0,   0 } ;	/* window parameters	*/

int window_size;								/* # of lines in a window */

do_window(ref_flag)
	int ref_flag;					/* nonzero forces "refresh" operation */
	{
	int i;

	if (colonflag && !ref_flag)
		{
		i = get_value(0);	/* get sub-function */
		if ((i < 0) || (i > 7)) ERROR(E_IWA);
		if (!esp->flag2)	/* it's a "get" */
			{
			esp->val1 = win_data[i];
			esp->flag1 = 1;
			}
		else
			{
			if ((esp->val2 < win_min[i]) || (esp->val2 > win_max[i]))	/* check range */
				ERROR(E_IWA);
			if (i == 7)
				{
				if (esp->val2)
					{
					WN_scroll = esp->val2;
					window_size = WN_height - WN_scroll;	/* define size of window area */
					window(WIN_INIT);			/* turn on window */
					}
				else window(WIN_OFF);			/* turn off window */
				}
			win_data[i] = esp->val2;			/* redundant for ~0,7:w, but no harm */
			esp->flag2 = 0;
			window(WIN_REDRAW);					/* redraw window */
			}
		}

	else								/* no colon, or ^W command */
		{
		if (esp->flag1 || ref_flag)
			{
			if (!ref_flag && (esp->val1 == -1000)) redraw_sw = 0;	/* -1000W: "forget that output was done" */
			else window(WIN_DISP);		/* nW or ^W refreshes window */
			}
		 esp->flag2 = esp->flag1 = 0;		/* no colon, consume args */
		}
	colonflag = 0;
	esp->op = OP_START;
	}
/* routine to update screen size with numbers obtained from environment	*/
/* (called by main program's initialization)							*/

set_term_par(lines, cols)
	int lines, cols;
	{
	if ((lines >= win_min[2]) && (lines <= win_max[2])) window_size = win_data[2] = lines;
	if ((cols >= win_min[1]) && (cols <= win_max[1])) win_data[1] = cols;
	}


/* window routine.  performs function as indicated by argument						*/
/* WIN_OFF:		disables split-screen scrolling										*/
/* WIN_SUSP:	disables split-screen scrolling temporarily							*/
/* WIN_INIT:	sets up display support if split-screen scrolling enabled, else nop	*/
/* WIN_RESUME:	re-enables display support											*/
/* WIN_REDRAW:	causes window to be redrawn on next refresh call					*/
/* WIN_REFR:	if scrolling enabled, redoes window, else if ev or es enabled, does	*/
/*				that, else nop														*/
/* WIN_LINE:	does WIN_REFR unless that wouldn't do anything, in which case		*/
/*				it does effective 1EV output										*/

int last_dot = -1;				/* last dot location */

window(arg)
	int arg;
	{
	int i;

	switch (arg)
		{
		case WIN_OFF:				/* final window off */
		case WIN_SUSP:				/* temp window off */
			if (WN_scroll)			/* if reset/clean up */
				{
				/* full margins, cursor to last line, erase line */
				printf("\033[r\033[%d;0H\033[K", WN_height);
				}
			break;

		case WIN_INIT:				/* initialize window - find output speed */
			if (out_noterm) win_speed = 0;		/* std out is not a terminal */
			else
				{
				for (win_speed = 1; (win_speeds[win_speed] != ttybuf.sg_ospeed) && (win_speed < 13); win_speed++);
				if (win_speed == 13) win_speed = 1;
				}
			w_init();				/* set up screen image buffer */
			if (WN_scroll) vt(VT_CLEAR);		/* if split-screen is enabled, clear screen */
									/* (fall through to "resume") */

		case WIN_RESUME:			/* re-enable window */
			if (WN_scroll)			/* set scroll region, cursor to bottom */
				printf("\033[%d;%dr\033[%d;0H", WN_height - WN_scroll + 1, WN_height, WN_height);
			break;
		case WIN_REDRAW:		/* force redraw of window */
			redraw_sw = 1;
			break;

		case WIN_LINE:			/* display one line unless window enabled or ev */
			if (WN_scroll || ev_val) window(WIN_REFR);		/* if a real window is set, do it */
			else if (w_setptr(dot, &w_p1))		/* set pointer to dot... and if there's a buffer */
				{
				w_lines(0, &w_p1, &w_p1);		/* get to beginning of line */
				window0(1);						/* and type 1 line */
				}
			break;

		case WIN_REFR:			/* if enabled, refresh window; else do ev or es */
			if (WN_scroll) window1();		/* if scrolling enabled, refresh the window */
			else if ((ev_val) || (es_val && search_flag))	/* else if ev or es, do that */
				{
				i = (ev_val) ? ev_val : es_val;
				if (w_setptr(dot, &w_p1))		/* set a pointer at dot... and if there's a buffer */
					window0(i - w_lines(1 - i, &w_p1, &w_p1));	/* go back (i-1) lines and ahead (i) lines */
				}
			break;

		case WIN_DISP:					/* display buffer independent of whether scroll mode is enabled */
			window1();
			break;

		}		/* end of switch */

	fflush(stdout);			/* send output out */
	}					/* end of window() */
/* routine to type n lines with character at "dot" in reverse video			*/
/* used for ev, es, and <BS> or <LF> as immediate commands					*/
/* starting char position is in w_p1; argument is number of lines			*/

window0(num)
	int num;
	{
	int wi;
	char wc;			/* temp char */

	for (wi = w_p1.dot; (num > 0) && (wi < z); wi++)		/* for each character */
		{
		wc = w_p1.p->ch[w_p1.c];		/* get character */

		if ((char_count >= WN_width) && (wc != CR) && !(spec_chars[wc] & A_L))	/* if about to exceed width */
			{
			if (et_val & ET_TRUNC) goto w0_noprint;			/* truncate: don't print this */
			else
				{
				fputs("\033[K\015\012\033(0h\033(B ", stdout);	/* <eeol> "NL space" */
				char_count = 2;
				--num;					/* one fewer line remaining */
				}
			}

		if (wi == dot)					/* if this char is at the pointer */
			{
			vt(VT_SETSPEC2);			/* set reverse video */
			if (wc == TAB)
				{
				type_char(' ');		/* illuminate the first sp of a tab */
				vt(VT_CLRSPEC);		/* clear reverse video */
				if (char_count & tabmask) type_char(TAB);
				}
			else						/* not a tab */
				{
				if ((wc == CR) && (char_count < WN_width))	/* CR at rh margin: don't display cursor */
					{
					type_char(' ');		/* cr: put a space after line */
					vt(VT_EEOL);		/* erase to eol */
					}
				type_char(wc);			/* type the char, or exec CR */
				if (wc == LF)
					{
					fputs("\033(0", stdout);
					type_char('e');
					fputs("\033(B", stdout);
					}
				vt(VT_CLRSPEC);		/* clear reverse video */
				}
			}
		else					/* this is not char at pointer */
			{
			if (wc == CR && curr_x < WN_width) vt(VT_EEOL);		/* erase to EOL */
			type_char(wc);
			}
		if ((wc == FF) || (wc == VT))		/* FF & VT end a line */
			{
			vt(VT_EEOL);			/* erase rest of this line */
			crlf();					/* and leave a blank one */
			if (!(ez_val & EZ_NOVTFF)) --num;		/* if FF and VT count as line sep's, count them */
			}

	  w0_noprint:
		if (++w_p1.c > CELLSIZE-1) w_p1.p = w_p1.p->f, w_p1.c = 0;	/* next char */
		if (wc == LF) --num;	/* if this is a line feed, count lines */
		}

	if (dot == z) fputs("\033[1;7m \033[0m\033[0K", stdout);	/* type one space and erase rest of line */
	else fputs("\033[0K", stdout);			/* else just erase to EOL */
	}
/* routine to maintain the screen window										*/
/* if scroll mode is enabled, the VT100 screen is split and only the upper part */
/* is used by this routine; else the whole screen is used.						*/

window1()
	{
	int i, j, m, lflag;

	if (!redraw_sw && (dot == last_dot) && (buff_mod == MAX)) return;		/* return if nothing has changed */

	block_inter(1);							/* disable ^C interrupts */
	if (WN_scroll) printf("\033[1;%dr", window_size);		/* scroll mode: redefine scroll region */
	printf("\033[H");						/* home */
	term_y = term_x = 0;					/* indicate cursor is at home */

	if ((redraw_sw) || (z <= wlp[0]->start)) window1_abs();		/* forced redraw, or z before start of screen */


/* check whether pointer is before modified buffer location */

	else if (buff_mod >= dot)	/* yes */
		{

		if (dot < wlp[0]->start)			/* if dot is before screen */
			{
			w_setptr(wlp[0]->start, &w_p1);	/* get to beginning of screen */

		/* check whether screen begins with the last part of a continued line */
			for (j = 0; (wlp[j]->cflag & WF_CONT) && (j < window_size/2); j++);
			if (j < window_size/2)			/* if so, does it continue less than halfway down the screen? */
				{

				if (j)						/* is there a partial line? */
					{
					w_lines(0, &w_p1, &w_p1);		/* 0L */
					j -= w_lines(1, &w_p1, NULL);		/* now j is number of display lines before screen */
					}
		/* now look for how many lines back "dot" is: if screen starts with partial line, w_p1 has already been moved */
		/* to beginning of the line and j equals the count of extra lines to scroll */

				for (i = 0; (dot < w_p1.dot) && (i < window_size/2); ) i -= w_lines(-1, &w_p1, &w_p1);
				if ((dot >= w_p1.dot) && (i < window_size))			/* found point within reason */
					{
					w_scroll(j - i);			/* scroll screen down that many lines */
					curr_y = wlp[0]->cflag = wlp[0]->col = curr_x = 0;		/* start from top of screen */
					wlp[0]->start = w_p1.dot;	/* save starting char position */
					window2(0);					/* and rewrite screen */
					}

				else window1_abs();				/* farther back than that - redraw */
				}

			else window1_abs();					/* continuation was too long: give up and redraw */
			}				/* end of "dot is before screen" */

		else if (dot <= wlp[last_y]->end) window1_inc(dot);		/* on screen - redraw incrementally */

		else window1_after();				/* dot is after screen: scroll or redraw */
		}				/* end of "dot is before modified point" */


/* the modified point in the buffer is before dot */

	else
		{
		if (buff_mod < wlp[0]->start) window1_abs();	/* modified point before screen - redraw fully */

		else if (buff_mod <= wlp[last_y]->end)			/* modified point on screen */
			{
			for (m = 0; buff_mod > wlp[m]->end; m++);	/* find line with buff_mod */
			w_setptr(wlp[m]->start, &w_p1);				/* set a pointer to start of line with buff_mod */
			j = (m < window_size/2) ? window_size - 1 - m : window_size/2;	/* maximum # of lines between buff_mod & dot */
			for (i = 0; (dot >= w_p1.dot) && (w_p1.dot < z) && (i <= j); )
				i += (lflag = w_lines(1, &w_p1, &w_p1) ) ? lflag : 1;	/* count lines from buff_mod to first line after dot */
			if (i > j) window1_abs();					/* too far - redraw */
			else
				{
				if (lflag && (dot == z)) i++;			/* if at end, following a LF */
				w_setptr(wlp[m]->start, &w_p1);			/* pointer to start of area to redraw */
				if (i >= window_size - m)				/* if there are not enough blank lines on screen */
					w_scroll(i = i - window_size + m), curr_y = m - i, curs_y -= i;	/* scroll up the difference */
				else curr_y = m;
				curr_x = (wlp[curr_y]->cflag & WF_CONT) ? 2 : wlp[curr_y]->col;		/* line starts at left unless continuation */
				if ((curr_y > curs_y) && (curs_y >= 0)) w_rmcurs();		/* remove old cursor if it won't be written over */
				window2(0);							/* rewrite newly cleared region */
				for (curr_x = 0; ++curr_y < window_size; )		/* clear rest of screen if needed */
					{
					wlp[curr_y]->cflag = 0;
					if (wlp[curr_y]->n) wlp[curr_y]->n = 0, vtm(VT_EEOL);
					}
				}
			}			/* end "modified point on screen */

		else window1_after();		/* modified point after screen: scroll or redraw as appropriate */
		}
/* done redrawing: do cleanup work */

	if (WN_scroll)
		{
		printf("\033[%d;%dr", window_size+1, WN_height);	/* reset margins */
		printf("\033[%d;0H", WN_height);		/* cursor to bottom */
		}
	else printf("\033[H");		/* no split screen: set home */

	fflush(stdout);				/* flush output */
	WN_origin = wlp[0]->start;	/* save first char pos on screen */
	redraw_sw = 0;				/* mark screen as updated */
	buff_mod = MAX;
	last_dot = dot;
	block_inter(0);				/* reenable interrupts */
	}
/* routine to redraw screen absolutely */

window1_abs()
	{
	int i, j;

	curr_y = wlp[0]->col = curr_x = 0;				/* indicate where refresh starts */
	set_pointer(dot, &w_p1);						/* make a text buffer, if none, and refresh the display */
	w_lines(0, &w_p1, &w_p1);						/* do 0L */
	if ((i = w_lines(window_size/2, &w_p1, NULL)) == 0) i = 1;		/* check how many lines after dot */
	if (i > window_size/2) i = window_size/2;		/* limit amount after dot */
	for (j = 0; (j < window_size - i) && (w_p1.dot > 0); )		/* find start of display area */
		j -= w_lines(-1, &w_p1, &w_p1);
	if (j > window_size - i) w_lines(1, &w_p1, &w_p1);			/* if too far back, move up one line */

	wlp[0]->start = w_p1.dot;		/* indicate where first window line starts */
	window2(0);						/* refresh the whole display */

	for (curr_x = 0; ++curr_y < window_size; )		/* blank out lines not written by window2 */
		if (wlp[curr_y]->n || redraw_sw) wlp[curr_y]->n = 0, vtm(VT_EEOL);
	}




/* redraw screen incrementally */

window1_inc(wd)
	int wd;						/* argument is earliest change */
	{
	short temp_y;

/* find the line containing the character at wd */

	for (temp_y = 0; wd > wlp[temp_y]->end; temp_y++);

	if ((curs_y != temp_y) || (buff_mod == MAX) || curs_crflag)		/* if the cursor line won't be rewritten */
		w_rmcurs();					/* remove the old cursor */
	curr_y = temp_y;				/* and go to work on the beginning of the line with dot */
	curr_x = (wlp[curr_y]->cflag & WF_CONT) ? 2 : wlp[curr_y]->col;		/* line starts at left unless continuation */

	w_setptr(wlp[curr_y]->start, &w_p1);		/* make a pointer there */
	window2(buff_mod == MAX);		/* if buffer not modified, redraw only the line with dot */

	if (buff_mod < MAX)			/* if buffer has changed, erase display lines beyond end of buffer */
		for (curr_x = 0; ++curr_y < window_size; )
			if ( ((wlp[curr_y]->start >= z) || (wlp[curr_y]->start <= wlp[curr_y-1]->end)) && (wlp[curr_y]->n || redraw_sw) )
				wlp[curr_y]->n = 0, vtm(VT_EEOL), wlp[curr_y]->cflag = 0;
	}
/* routine to move window downwards: scroll up or redraw as appropriate */

window1_after()
	{
	int i, lflag;

	w_rmcurs();						/* remove old cursor */
	w_setptr(wlp[window_size-1]->start, &w_p1);		/* set pointer to start of last line on screen */

	for (i = 0; (dot >= w_p1.dot) && (w_p1.dot < z) && (i <= window_size/2); )
		i += (lflag = w_lines(1, &w_p1, &w_p1)) ? lflag : 1;	/* fwd one line at a time until > dot or end of buffer */

	if (i <= window_size/2)			/* found within n lines */
		{
		if (lflag && (dot == z)) ++i;				/* if dot is at end of buffer after a LF */
		if (i >= window_size - last_y)				/* if there are not enough blank lines on screen */
			w_scroll(i - window_size + last_y), curr_y = window_size - i;	/* scroll up the difference */
		else curr_y = last_y;

		while (curr_y && (wlp[curr_y]->cflag & WF_CONT)) --curr_y;		/* get to start of cont'd lines */
		w_setptr(wlp[curr_y]->start, &w_p1);		/* pointer to start of area to redraw */
		curr_x = wlp[curr_y]->col;					/* redraw starts at line's first column */
		window2(0);									/* rewrite newly cleared region */
		}

	else window1_abs();						/* move down is too far: redraw fully */
	}



/* routine to remove the existing cursor */

w_rmcurs()
	{
	if (curs_c)			/* if there was a cursor */
		{
		w_move(curs_y, curs_x);		/* go remove the old cursor */
		if (curs_c & W_MARK) fputs("\033(0", stdout);		/* if prev char was a spec char */
		putchar(*curs_p = curs_c);	/* put back the char that was there */
		if (curs_c & W_MARK) fputs("\033(B", stdout);
		++term_x;					/* and keep the terminal cursor loc. happy */
		}
	}
/* routine to do actual display refresh												*/
/* called with w_p1 at starting char, curr_y, curr_x at starting coordinate			*/
/* rewrites to end of screen if arg = 0, or only until line with cursor if arg = 1	*/

window2(arg)
	int arg;
	{
	register int wdot;
	register char wc;			/* temp char */
	register short dflag;		/* nonzero if this is char at dot */
	short cr_found;				/* indicates a cr found on this line */

	cr_found = 0;				/* clear "cr" flag in first line written */
	for (wdot = w_p1.dot; (curr_y < window_size) && (wdot < z); wdot++)		/* for each character */
		{
		wc = w_p1.p->ch[w_p1.c] & 0177;		/* get character */
		if (dflag = (wdot == dot)) if (arg) arg = -1;		/* save "this is char at dot", "on line with dot" */

		if (wc < ' ') switch (wc)				/* dispatch control characters */
			{
			case CR:
				if (dflag)			/* if cursor on this CR */
					{
					if (curr_x < WN_width) w_makecurs(' ', 1), w_type(' ', 1);	/* display a space, unless at end */
					else curs_crflag = curs_c = 0;				/* else set "no cursor displayed" */
					}
				/* trim remainder of line if this is first cr and old line was longer */
				if (!cr_found && ((curr_x < wlp[curr_y]->n) || redraw_sw))
					{
					wlp[curr_y]->n = curr_x;
					if (curr_x < WN_width) vtm(VT_EEOL);
					}
				cr_found = 1;			/* set cr flag */
				wlp[curr_y]->cflag &= ~WF_BEG;		/* this line is not continued */
				while (curr_y && (wlp[curr_y]->cflag & WF_CONT)) --curr_y;		/* if line is a continuation, scan up */
				curr_x = 0;
				break;

			case TAB:
				if (curr_x >= WN_width)
					{
					if (et_val & ET_TRUNC) goto noprint;
					if (w_overflow(wdot)) goto w2_exit;				/* extend line */
					}
				if (dflag) w_makecurs(' ', 0);
				w_type(' ', dflag);						/* type one space */
				if (dflag)
					{
					vt(VT_CLRSPEC);		/* end reverse video */
					dflag = 0;
					}
				while ((curr_x & tabmask) && (curr_x < WN_width)) w_type(' ', 0);		/* finish tab */
				break;
			case LF:
				while ((curr_y < window_size) && (wlp[curr_y]->cflag & WF_BEG)) ++curr_y;	/* last screen row of this line */
				wlp[curr_y]->end = wdot;		/* save char position that ended this line */
				if (dflag)		/* if this LF is at dot */
					{			/* put cursor there, save char that was there */
					w_makecurs( (curr_x < wlp[curr_y]->n) ? wlp[curr_y]->ch[curr_x] : ' ', 0);
					fputs("\033(0", stdout);			/* put in a "LF" char */
					w_type('e', 1);
					fputs("\033(B", stdout);
					}			/* if no cr found and not in last column, erase rest of line */
				if (!cr_found && (curr_x < wlp[curr_y]->n))
					{
					wlp[curr_y]->n = curr_x;
					if (curr_x < WN_width) vtm(VT_EEOL);
					}
				if (dflag) --curr_x;			/* put the cursor back before the artificial LF char, if any */
				if (curr_y >= window_size-1)	/* if at end of screen, exit, but... */
					{
					if (dflag) vt(VT_CLRSPEC);	/* if cursor is here, clear reverse video first */
					goto w2_exit;
					}

				if ((wlp[curr_y]->cflag & WF_CONT) && (wlp[curr_y]->end - wlp[curr_y]->start == 1))	/* if a now-empty cont. line, */
					{															/* flush it */
					if (curr_y > 0) wlp[curr_y-1]->cflag &= ~WF_BEG;			/* remove "cont'd" flag from prev line */
					arg = 0;													/* and force redraw of rest of screen */
					if (curs_y == curr_y) curs_c = 0;							/* if cursor was on this line, it will disappear */
					}
				else ++curr_y;				/* down one line if not absorbing blank contin. line */

				wlp[curr_y]->start = wdot + 1;				/* assume line starts with next char */
				wlp[curr_y]->col = curr_x;					/* save starting column */
				cr_found = wlp[curr_y]->cflag = 0;			/* clear line continuation flags */
				if (curr_x) w_ebol();						/* if not at left margin, erase beginning of line */
				if (arg == -1)								/* finished line with dot... quit if spec'd */
					{
					if (dflag)						/* but first, if at cursor, clear reverse video */
						{
						vt(VT_CLRSPEC);
						dflag = 0;
						}
					return;
					}
				break;

			case ESC:
				if (curr_x >= WN_width)
					{
					if (et_val & ET_TRUNC) goto noprint;
					if (w_overflow(wdot)) goto w2_exit;				/* extend line */
					}
				if (dflag) w_makecurs('$', 0);
				w_type('$', dflag);
				break;
			default:					/* all other control chars print as ^X */
				if (curr_x >= WN_width - 1)
					{
					if (et_val & ET_TRUNC) goto noprint;
					if (w_overflow(wdot)) goto w2_exit;
					}
				if (dflag) w_makecurs('^', 0);
				w_type('^', dflag);				/* ^ */
				if (dflag)
					{
					vt(VT_CLRSPEC);		/* if at cursor, clear reverse video */
					dflag = 0;
					}
				w_type(wc | 0100, 0);
				break;
			}					/* end "switch" */
		else					/* a printing character */
			{
			if (curr_x >= WN_width)
				{
				if (et_val & ET_TRUNC) goto noprint;
				if (w_overflow(wdot)) goto w2_exit;				/* extend line */
				}
			if (dflag) w_makecurs(wc, 0);
			w_type(wc, dflag);
			}

		if (dflag)
			{
			vt(VT_CLRSPEC);				/* if at cursor, clear reverse video */
			}

		if ((wc == FF) || (wc == VT))			/* these chars leave a display line blank */
			{
			if (redraw_sw || (curr_x < wlp[curr_y]->n))
				{
				wlp[curr_y]->n = curr_x;
				if (curr_x < WN_width) vtm(VT_EEOL);	/* erase rest of line */
				}
			wlp[curr_y]->end = wdot;
			if (curr_y >= window_size-1) goto w2_exit;	/* quit if overflow screen */
			wlp[++curr_y]->start = wdot + 1;
			cr_found = wlp[curr_y]->cflag = 0;		/* init new line */
			if (curr_x -= 2) w_ebol();		/* back up over ^X; if not at left margin, erase beginning of line */
			wlp[curr_y]->col = curr_x;				/* save starting column */
			}
	  noprint:
		if (++ w_p1.c > CELLSIZE - 1) w_p1.p = w_p1.p->f, w_p1.c = 0;	/* next char in buffer */
		}		/* end of "for all characters" */

	if (dot == z)
		{
		if (curr_x < WN_width) w_makecurs(' ', 1), w_type(' ', 1), vt(VT_CLRSPEC);	/* display a space, unless at end */
		else curs_crflag = curs_c = 0;				/* else set "no cursor displayed" */
		}

	/* clear rest of line if needed */
	if (!cr_found && (redraw_sw || (curr_x < wlp[curr_y]->n)))
		{
		wlp[curr_y]->n = curr_x;
		if (curr_x < WN_width) vtm(VT_EEOL);
		}
	wlp[curr_y]->end = wdot;		/* save char at end of last line */
  w2_exit:
	last_y = curr_y;				/* record last used line on screen */
	}
/* routine to move cursor to current location and then call vt */

vtm(arg)
	int arg;
	{
	w_move(curr_y, curr_x);
	vt(arg);
	}




/* routine to set reverse video and save cursor location */
/* first argument is char at cursor, 2nd is value for curs_crflag */

w_makecurs(wc, crflag)
	char wc;
	short crflag;
	{
	curs_y = curr_y, curs_x = curr_x, curs_c = wc;	/* save cursor coord and char */
	curs_p = &wlp[curr_y]->ch[curr_x];				/* save location of cursor spot in window image */
	curs_crflag = crflag;		/* save crflag */
	vt(VT_SETSPEC2);			/* set flag and reverse video */
	}




/* routine to handle line overflow */
/* returns nonzero if at end of screen, zero otherwise */
/* arg is current character position */

int w_overflow(wd)
	{
	wlp[curr_y]->end = wd-1;			/* last character was end of this line */
	if (wlp[curr_y]->n > curr_x)
		{
		wlp[curr_y]->n = curr_x;
		if (curr_x < WN_width) vtm(VT_EEOL);		/* if old line was wider, erase */
		}
	if (curr_y >= window_size-1) return(1);
	wlp[curr_y]->cflag |= WF_BEG;				/* mark this line as "continued" */
	wlp[++curr_y]->cflag = WF_CONT;				/* next line is a continuation line */
	wlp[curr_y]->start = wd;					/* char about to be printed is this line's first */
	wlp[curr_y]->col = curr_x = 0;				/* new line starts at left margin */
	fputs("\033(0", stdout);					/* alternate char set */
	w_type('h', W_MARK);						/* "NL" space */
	w_type(' ', W_MARK);
	fputs("\033(B", stdout);
	return(0);
	}
/* routine to type one character:  arguments are char and a */
/* "mark" bit.  If mark is set, the char is always retyped  */

w_type(c, m)
	char c;
	int m;
	{
	register char *p;

	p = &wlp[curr_y]->ch[curr_x];		/* pointer to char image */
	if ((c != *p) || (m) || (redraw_sw) || (curr_x >= wlp[curr_y]->n))
		{
		w_move(curr_y, curr_x);
		putchar(c);
		*p = (m) ? c | W_MARK : c;
		++term_x;
		}
	++curr_x;
	if (wlp[curr_y]->n < curr_x) wlp[curr_y]->n = curr_x;	/* if we've lengthened the line, record that fact */
	}




/* initialize display image */

w_init()
	{
	short i, j;

	for (i = 0; i < window_size; i++)		/* for each row */
		{
		wlp[i] = &w_image[i];				/* set pointer to this line's data */
		w_image[i].n = w_image[i].cflag = 0;	/* no chars used, cr flag clear */
		for (j = 0; j < W_MAX_H; w_image[i].ch[j++] = ' ');		/* clear line */
		}
	}




/* put character followed by appropriate number of nulls for "other control function" */
/* if argument is 0, output filler chars only */

putchar_d(c)
	char c;
	{
	int i;

	if (c) putchar(c);												/* output character */
	for (i = 0; i < win_dlyc[win_speed]; i++) putchar('\0');		/* output filler */
	}
/* put out appropriate number of filler chars for display function that scrolls (LF, etc.) */

scroll_dly()
	{
	int i;

	for (i = 0; i < win_dlys[win_speed]; i++) putchar('\0');		/* output filler */
	}



/* move terminal cursor to stated y, x position */
/* uses incremental moves or absolute cursor position, whichever is shorter */

w_move(y, x)
	short y, x;
	{
	register short i;

	/* if practical, use CR to get to left margin */
	if ((curr_x == 0) && (term_x != 0)) putchar(CR), term_x = 0;
	if ((y == term_y) && (term_x < WN_width))		/* if term x is beyond last char, use abs positioning */
		{
		if (x == term_x) return;
		if (x > term_x)
			{
			if (x - term_x == 1) fputs("\033[C", stdout);
			else printf("\033[%dC", x - term_x);
			}
		else
			{
			if ((i = term_x - x) < 4) for (; i > 0; i--) putchar('\b');	/* use BS */
			else printf("\033[%dD", term_x - x);		/* use incremental jump */
			}
		term_x = x;
		}
	else
		{
		if ((x == term_x) && (term_x < WN_width))
			{
			if (y > term_y)
				{
				if ((i = y - term_y) < 4) for (; i >0; i--) putchar(LF);	/* use LF */
				else printf("\033[%dB", i);		/* use incremental jump */
				}
			else if ((i = term_y - y) == 1) fputs("\033[A", stdout);	/* move 1 */
			else printf("\033[%dA", i);
			term_y = y;
			}
		else printf("\033[%d;%dH", (term_y = y) + 1, (term_x = x) + 1);		/* absolute jump */
		}
	}
/* scroll screen: argument is count: + up, - down */

w_scroll(count)
	int count;
	{
	register int i, ic;
	struct w_line *p[W_MAX_V];		/* temp copy of pointer array */

	if (count > 0)		/* scrolling up */
		{
		w_move(window_size-1, 0);	/* cursor to bottom of window */
		for (i = 0; i < count; i++)
			{
			putchar(LF), wlp[i]->n = 0;		/* scroll terminal, blank out image line */
			}
		}
	else		/* scroll down */
		{
		w_move(0, 0);		/* cursor to top */
		for (i = 0; i > count; i--)
			{
			fputs("\033M", stdout), wlp[window_size-1+i]->n = 0;
			}
		}
	for (i = 0; i < window_size; i++) p[i] = wlp[(window_size + i + count) % window_size];	/* rearrange */
	for (i = 0; i < window_size; i++) wlp[i] = p[i];
	}



/* clear line to left of curr_x */
/* if some chars nonblank, does erase from start of line */

w_ebol()
	{
	short i, j;

	for (j = i = 0; i < curr_x; i++) if (wlp[curr_y]->ch[i] != ' ') wlp[curr_y]->ch[i] = ' ', j++;
	if (j || redraw_sw) w_move(curr_y, curr_x-1), vt(VT_EBOL);
	}



/* routine to set a pointer to a given location (like set_pointer) */
/* returns nonzero if a text buffer exists, otherwise 0 */

int w_setptr(loc, pp)
	register int loc;				/* location */
	register struct qp *pp;			/* address of pointer */
	{
	register int i;

	if (buff.f)
		{
		for (i = loc / CELLSIZE, pp->p = buff.f; i > 0; i--) pp->p = pp->p->f;
		pp->c = loc % CELLSIZE;
		pp->dot = loc;
		}
	return( (int) buff.f);
	}
/* routine to move N lines (back, forward, or 0)				*/
/* w_lines(n, &source, &dest) where n is the line count, source	*/
/* points to a qp at the current pointer, dest, if nonzero,		*/
/* it points to a qp where the result is to go.					*/
/* routine returns actual number of display lines				*/

struct qp w_lines_p;				/* to compute # of display lines in -N lines */

int w_lines(n, ps, pd)
	int n;							/* number of lines */
	register struct qp *ps, *pd;	/* source, destination qp's */
	{
	register struct buffcell *tp;	/* local copy of the qp */
	register int tc, tdot, tn;
	int tcnt, tl;					/* chars/line and display line count */
	char tch;

	tdot = ps->dot;
	tp = ps->p;
	tc = ps->c;

	if (n > 0)			/* argument is positive */
		{
		for (tcnt = tl = tn = 0; (tn < n) && (tdot < z); tdot++)	/* forward over N line separators */
			{
			if (spec_chars[ tch = tp->ch[tc] ] & A_L) ++tl, ++tn;		/* count separators */
			else if (!(et_val & ET_TRUNC))		/* if text lines can overflow screen lines */
				{
				if (!(tch & 0140))				/* if character is a control char */
					{
					if (tch == CR)										/* CR resets count */
						{
						if (tcnt > WN_width) ++tl;
						tcnt = 0;
						}
					else if (tch == TAB) tcnt = (tcnt | tabmask) +1;	/* tab to next tab stop */
					else if (tch == ESC) ++tcnt;						/* ESC takes one space */
					else tcnt += 2;										/* normal control chars take 2 spaces */
					}
				else ++tcnt;				/* not a control char: takes one space */
				if (tcnt > WN_width) ++tl, tcnt = 2;		/* if overflow, one more line */
				}

			if (++tc > CELLSIZE-1) tp = tp->f, tc = 0;		/* next character position */
			}
		if (tl > tn) tn = tl;		/* if counting display lines and there's more of them, return that */
		}
	else				/* argument is zero or negative */
		{
		for (tn = 0; (tn >= n) && (tdot > 0); )		/* back up over (n+1) line feeds */
			{
			--tdot;
			if (--tc < 0) tp = tp->b, tc = CELLSIZE -1;
			if (spec_chars[tp->ch[tc]] & A_L) --tn;
			}
		if (tn < n)			/* if stopped on a line sep, fwd over it */
			{
			++tn;
			++tdot;
			if (++tc > CELLSIZE-1) tp = tp->f, tc = 0;
			}

		if (!(et_val & ET_TRUNC) && (n != 0))			/* if text line can overflow display line */
			{
			w_lines_p.dot = tdot, w_lines_p.p = tp, w_lines_p.c = tc;		/* then count the number of display */
			tn = -w_lines(-n, &w_lines_p, 0);				/* lines in the N text lines we just backed up over */
			}
		}

	if (pd) pd->dot = tdot, pd->p = tp, pd->c = tc;		/* if an "after" pointer given, update it */
	return(tn);
	}

