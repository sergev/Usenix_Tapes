/* Scrn.c replaces screen.c that was in the net distribution. Module contains
 * PC screen routines plus whatever was in screen.c that had to be kept for
 * linking with the other modules at link time.
 */
 
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "scrn.h"

#define CNTL_H		0x08
#define CNTL_U		0x15
#define NL		0x0a
#define CR		0x0d
#define TAB		0x09
#define BELL		0x07
#define ROWS		25
#define COLUMS		80

#define	NOT_QUIET	0	/* Ring bell at eof and for errors */
#define	LITTLE_QUIET	1	/* Ring bell only for errors */
#define	VERY_QUIET	2	/* Never ring bell */

extern int quiet;		/* If VERY_QUIET, use visual bell for bell */
extern int scrn_in_color;	/* When using color monitor */
int erase_char, kill_char;	/* The user's erase and line-kill chars */
int sc_height, sc_width, se_width, ue_width, ul_width, so_width;
int auto_wrap, ignaw;
void init(), dinit(), get_term(), home(), bell(), vbell(), add_line();
void lower_left(), clear(), clear_eol(), so_enter(), so_exit(), ul_enter();
void ul_exit(), backspace(), putbs(), raw_mode();

cls()         /* move cursor home and clear the screen  */
{
	union REGS REG;
	int mode;
    
	gotoxy(0, 0);
	REG.x.ax = 0x0600;
	if (scrn_in_color == 1)
		REG.h.bh = WHITE_ON_BLUE;
	else
		REG.h.bh = BW;
	REG.x.cx = 0x0000;
	REG.x.dx = 0x184f;
	int86(0x10, &REG, &REG);
}

era_eol()
{
	union REGS REG;
	int hold[4];
	int column;

	getxy(hold);
	column = hold[1];
	if (scrn_in_color == 1)
		REG.x.bx = WHITE_ON_BLUE;
	else
		REG.x.bx = BW;
	REG.x.ax = 0x0900;	/* ah = 10; al = null char to write */
	REG.x.cx = 80 - column;	/* cx = no. of nulls to write */
	int86(0x10, &REG, &REG);
	restorxy(hold);		/* retore cursor to original position */
	return;
}

gotoxy(row, col)  /* Position cursor at x,y on screen */
int	row, col;
{
	union REGS REG;

	REG.h.ah = 02;
	REG.h.bh = 00;
	REG.h.dh = row;
	REG.h.dl = col;
	int86(0x10, &REG, &REG);
}

getxy(hold) /* Get cursor coordinates */
int *hold;
{
	union REGS REG;

	REG.h.ah = 03;
	REG.h.bh = 00;
	int86(0x10, &REG, &REG);
	hold[0] = REG.h.dh;
	hold[1] = REG.h.dl;
	hold[2] = REG.h.ch;
	hold[3] = REG.h.cl;
}


restorxy(hold)  /* Restore cursor gotten above */
int *hold;
{
	union REGS REG;

	gotoxy(hold[0], hold[1]);
	REG.h.ah = 01;
	REG.h.bh = 00;
	REG.h.ch = hold[2];
	REG.h.cl = hold[3];
	int86(0x10, &REG, &REG);
}

curs_r(n)	/* move cursor right n places */
int n;
{
	int hold[4];
	int row, column;
		
	getxy(hold);
	row = hold[0];
	column = hold[1];
	if (column < 0)
		if (n < 0)
			return(0);
	if (column > 79)
		if (n > 0)
			return(0);
	column = column + n;
	gotoxy(row, column);
}						

curs_l(n)	/* move cursor left n places */
int n;
{
	curs_r(-n);
}

scroll_up(n)
int n;
{
	union REGS REG;

	REG.h.ah = 0x06;
	if (scrn_in_color == 1)
			REG.h.bh = WHITE_ON_BLUE;
	else
		REG.h.bh = BW;
	REG.h.al = n;
	REG.x.cx = 0x0000;
	REG.x.dx = 256 * 24 + 79;
	int86(0x10, &REG, &REG);
	return(1);
}

get_mode()   /* Check for Monochrome mode 7 */
{
	union REGS REG;

	REG.h.ah = 15;
	int86(0x10, &REG, &REG);
	return(REG.h.al);
}


/*
 * Set cursor checking for current cursor size parameters.
 */

set_cur()
{
	union REGS INREG, OUTREG;

	if (get_mode() == 7)
	{
		INREG.h.ah = 1;
		INREG.h.bh = 0x00;
		INREG.h.ch = 12;
		INREG.h.cl = 13;
		int86(0x10, &INREG, &OUTREG);
	}
	else
	{
		INREG.h.ah = 0x03;
		INREG.h.bh = 0x00;
		int86(0x10, &INREG, &OUTREG);
		INREG.h.ah = 0x01;
		INREG.h.bh = 0x00;
		INREG.h.ch = OUTREG.h.ch;
		INREG.h.cl = OUTREG.h.cl;
		int86(0x10, &INREG, &OUTREG);
	}
}

chr_put(c, attribute)
int c;
int attribute;
{
    union REGS REG;
    int hold[4];
    int i, row, column;
            
    if (c == CR)
    {
    	getxy(hold);
    	row = hold[0];
    	column = 0;
    	gotoxy(row, column);
        return(1);
    }
    if (c == TAB)
    {
    	for (i = 0;i <= 7;++i)
    		chr_put(' ', attribute);
        return(1);
    }
    if (c == BELL)
    {
    	putch(7);
    	return(1);
    }
    if (c == NL)
    {
	getxy(hold);
	row = hold[0];
        if (row >= 24)
        	scroll_up(1);
	else
		++row;
	column = 0;
	gotoxy(row, column);
        return(1);
    }    	
    REG.h.ah = 0x9;
    REG.h.al = c;
    REG.h.bl = attribute;
    REG.h.bh = 00;
    REG.x.cx = 1;
    int86(0x10, &REG, &REG);
    curs_r(1);
    return(REG.x.ax);
}

str_put(str, attribute)
char *str;
int attribute;
{
	int i;

	if (scrn_in_color == 1)
		attribute = WHITE_ON_RED;
	else
		attribute = REV_VID;
	for (i = 0;i < strlen(str);++i)
		chr_put(*(str + i), attribute);
}


/*
 * Add a blank line (called with cursor at home).
 * Should scroll the display down.
 */

void
add_line()
{
	union REGS REG;
	int hold[4];	
	int row, column;

	REG.h.ah = 0x07;
	if (scrn_in_color == 1)
		REG.h.bh = WHITE_ON_BLUE;
	else
		REG.h.bh = BW;
	REG.h.al = 1;
	getxy(hold);
	row = hold[0];
	column = hold[1];
	REG.h.ch = row;
	REG.h.cl = 0;
	REG.h.dh = 24;
	REG.h.dl = 79;
	int86(0x10, &REG, &REG);
}	

/*
 * Below are the functions which perform all the "less terminal-specific"
 * screen manipulation functions. They are taken from screen.c that was
 * in the distribution of less on the news.
 */

/*
 * Initialize terminal
 */
void
init()
{
	set_cur();
}

/*
 * Deinitialize terminal
 */
void
deinit()
{
}

void
get_term()
{
	sc_height = ROWS;
	sc_width = COLUMS;
	se_width = 0;
	ue_width = 0;
	ul_width = 0;
	so_width = 0;
	auto_wrap = 0;			/* chr_put doesn't autowrap */
	ignaw = 0;
	/* sneak in kill and erase characters for command line editing */
	kill_char = CNTL_U;		/* use ctrl-u as kill chararcter */
	erase_char = CNTL_H;		/* use ctrl-h as erase character */
}

void
raw_mode(on)
int on;
{
	/* left here in case there is a desire */
	/* to put terminal in raw_mode vs cooked */
}

/*
 * Home cursor (move to upper left corner of screen).
 */

void
home()
{
	gotoxy(0, 0);
}

/*
 * Move cursor to lower left corner of screen.
 */
void
lower_left()
{
	gotoxy(24, 0);
}

/*
 * Ring the terminal bell.
 */
void
bell()
{
	if (quiet == VERY_QUIET)
		vbell();
	else
		putch(BELL);
}

/*
 * Output the "visual bell", if there is one.
 */
void
vbell()
{
	/* there is no visual bell at this time */
	return;
}

/*
 * Clear the screen.
 */
void
clear()
{
	cls();
}

/*
 * Clear from the cursor to the end of the cursor's line.
 * {{ This must not move the cursor. }}
 */
void
clear_eol()
{
	era_eol();
}

/*
 * Begin "standout" (bold, underline, or whatever).
 */
void
so_enter()
{
}

/*
 * End "standout".
 */
void
so_exit()
{
}

/*
 * Begin "underline" (hopefully real underlining, 
 * otherwise whatever the terminal provides).
 */
void
ul_enter()
{
}

/*
 * End "underline".
 */
void
ul_exit()
{
}

/*
 * Erase the character to the left of the cursor 
 * and move the cursor left.
 */
void
backspace()
{
	/* 
	 * Try to erase the previous character by overstriking with a space.
	 */
	curs_l(1);
	putc(' ');
	curs_l(1);
}

/*
 * Output a plain backspace, without erasing the previous char.
 */
void
putbs()
{
	curs_l(1);
}

