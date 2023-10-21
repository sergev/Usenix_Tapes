/**************************************************************************
*   filename = scr_ctrl.c
*
*   screen control functions:
*     cursor up, down, left, right (number of positions)
*     backspace  (number of backspaces )
*     position (column, line)
*     erase to end of line from current position
*     erase entire screen, home cursor
*     erase from (column) to end of line for (line1, line2)
*     wait for (character)
*     beep at the dumb user
*     save, restore cursor position
*
***************************************************************************/

/* ---------------------------------------------------------------- */
/* these values are used in calls to move_cur function		    */
/* ---------------------------------------------------------------- */
#define up	  0
#define down	  1
#define right	  2
#define left	  3

/* ---------------------------------------------------------------- */
/* Array and values are for routines that output escape sequences.  */
/* First two characters are always escape and '[', for sequence.    */
/* Each routine must set byte after last used to endstr, so sc_doit */
/* routine that does output will know when to stop output.	    */
/*----------------------------------------------------------------- */
#define BDOS_IN   8		/* input function for "sc_wait" */
#define BDOS_OUT  6		/* output function for "sc_outc" */

#define escape	  '\033'
#define leadin	  '['
#define endstr	  '\0'

static char outstr[10] = {
    escape, leadin
};



/*   move cursor function --------------------- */
sc_movcur (move_dir, move_cnt)
int     move_dir,
        move_cnt;
{

    switch (move_dir) {
	case up:
	    outstr[4] = 'A';
	    break;
	case down:
	    outstr[4] = 'B';
	    break;
	case right:
	    outstr[4] = 'C';
	    break;
	case left:
	    outstr[4] = 'D';
	    break;
	default: 
	    return;		/* not allowed to input just anything */
    }
    outstr[2] = (move_cnt / 10) + '0';
    outstr[3] = (move_cnt % 10) + '0';
    outstr[5] = endstr;
    sc_doit ();
    return;
}

/* move cursor up function -------------------- */
sc_up (num_up)
int     num_up;
{
    sc_movcur (up, num_up);
    return;
}


/* move cursor down function ------------------ */
sc_down (num_down)
int     num_down;
{
    sc_movcur (down, num_down);
    return;
}


/* move cursor right function ------------------ */
sc_right (num_right)
int     num_right;
{
    sc_movcur (right, num_right);
    return;
}


/* move cursor left function */
sc_left (num_left)
int     num_left;
{
    sc_movcur (left, num_left);
    return;
}



/* ------   backspace function ------ */
sc_bspac (num_back)
int     num_back;
{
    do {
	sc_left (1);
	sc_outc (' ');
	sc_left (1);
    }
    while (--num_back > 0);
    return;
}



/* ------   wait function ------ */

sc_wait (achar)
int     achar;
{
/* DOS int 21, function 8: registers DX and AL are not used */
    while (achar != bdos (BDOS_IN, 0, 0));/* 2 arguments added */
    return;
}



/* ------- cursor position function ------ */
sc_gotoxy (col, line)
int     col,
        line;
{

    outstr[2] = (line / 10) + '0';
    outstr[3] = (line % 10) + '0';
    outstr[4] = ';';
    outstr[5] = (col / 10) + '0';
    outstr[6] = (col % 10) + '0';
    outstr[7] = 'H';
    outstr[8] = endstr;
    sc_doit ();
    return;
}



/* ------ clear screen function -----------*/
sc_clrscr ()
{

    outstr[2] = '2';
    outstr[3] = 'J';
    outstr[4] = endstr;
    sc_doit ();
    return;
}


/* ------ clear end of line function ------*/
sc_clreol ()
{			/* cursor is assumed to be in position before call */

    outstr[2] = 'K';
    outstr[3] = endstr;
    sc_doit ();
    return;
}



/* ------ clear region of lines function ------ */
sc_clrlin (col, first, last)
int     col,
        first,
        last;
{
    static int  count;

    sc_gotoxy (col, first);	/* position to column on first line */
    sc_clreol ();		/* clear to end of line		   */
    if (first != last) {	/* if more than one line, home cursor */
	for (count = first; count < last; ++count) {
	    sc_movcur (down, 1);/* down is faster than gotoxy       */
	    sc_clreol ();
	}
	sc_gotoxy (col, first);
    }
    return;
}



sc_savcsr ()
{			/* save the current cursor position */

    outstr[2] = 's';
    outstr[3] = endstr;
    sc_doit ();
    return;
}



sc_rstcsr ()
{			/* restore to save cursor position */

    outstr[2] = 'u';
    outstr[3] = endstr;
    sc_doit ();
    return;
}



sc_doit ()
{			/*  doit outputs the array outstr as built by caller  */
    static int  i;

    i = 0;
    while (outstr[i])
	sc_outc (outstr[i++]);
    return;
}



sc_beep ()
{			/* simply output the character that rings the PC's bell  */

    sc_outc ('\007');
    return;
}



sc_outc (c)
char    c;
{
/* DOS int 21, function 6: register DX has char to write, AH is not used */
    bdos (BDOS_OUT, c, 0);	/* 1 argument added */
    return;
}
