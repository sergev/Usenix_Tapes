/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/
/* -------------------------------------------------------------------------
   This file contains utility routines as used by 'P_APPL.EXE'.
	   filename = 'p_applut.c'
   ------------------------------------------------------------------------- */
#include <ctype.h>
#include <app.h>
#include <p_padfnc.h>		/*  This the only include useful to the PAD invoker. */


char   *error_msg[] = {
    "Illegal Function", "Illegal Device", "Illegal Channel", "Illegal Wait RQ",
    "Illegal Char RQ", "Illegal Packet RQ", "Illegal Port", "Illegal Baud",
    "Illegal Parity", "Illegal # databits", "Illegal # stopbits",
    "Illegal DTE/DCE", "Port Active", "I/O Overflow", "I/O Blocked",
    "Illegal Data Type", "Illegal Call Format", "Illegal Clearcode",
    "Illegal Call State", "No chans avail", "Illegal Pad Parameter",
    "Pkt Chans Open", "Illegal Event", "Timer Running", "No Resources",
    "?", "?", "?", "?", "?", "?", "?", "?"
};

void sh_chanstat (achan)
int     achan;
{
    extern int  sc_savcsr();
    extern int  sc_rstcsr();
    extern int  sc_clrlin();
    sc_savcsr ();
    sc_clrlin (1, 1, 1);
    if (ca[achan].status == FUNC_COMPLETE)
	cprintf ("success");
    else {
	if (ca[achan].status == 99)
	    cprintf ("Driver is not loaded, SPACE");
	else
	    cprintf ("error = %s  (%2d),  SPACE",
		    error_msg[(ca[achan].status - 1)], ca[achan].status);
	inchar (NO_SHOW);
    }
    sc_rstcsr ();
    return;
}



int     get_str (term_char, astr, str_len)
char    term_char,
        astr[];
int     str_len;
{
    extern int  sc_bspac();
    extern int  nbrk_inp();
    extern void nbrk_out();
    int     value,
            i;

    for (i = 0; i < str_len; i++)
	astr[i] = '\0';
    i = 0;
    while (i < str_len) {
	value = nbrk_inp ();
	if (value == term_char)
	    break;
	else {
	    if (value == BSPAC_CHR)
		if (i > 0) {
		    sc_bspac (1);
		    --i;
		}
		else
		    nothing;
	    else
		if (isascii (value)) {
		    astr[i++] = value;
		    nbrk_out (value);
		    if (value == CR_CHR)
			nbrk_out (LF_CHR);
		}
	}
    }
    return (i);
}



int     flip_dbug ()
{
    extern int  sc_beep();
    extern int  debug_mode;	/* defined in p_appl.c */
    int     i;

    debug_mode = !debug_mode;
    sc_beep ();
    if (debug_mode) {
	for (i = 1; i < 500; i++);
	sc_beep ();
    }
    return;
}



int     get_int (col, line, prompt, minv, maxv, digits)
int     col,
        line,
        minv,
        maxv,
        digits;
char   *prompt;
{
    extern int  sc_clrlin();
    extern int  sc_beep();
    extern void nbrk_out();
    int     temp,
            count;
    int     achar;

    while (TRUE) {
	sc_clrlin (col, line, line);
	switch (digits) {
	    case 1:
		cprintf ("%s [%1d-%1d]:", prompt, minv, maxv);
		break;
	    case 2:
		cprintf ("%s [%2d-%2d]:", prompt, minv, maxv);
		break;
	    case 3:
		cprintf ("%s [%3d-%3d]:", prompt, minv, maxv);
		break;
	}
	count = temp = 0;
	while (TRUE) {
	    achar = inchar (NO_SHOW);
	    if ((achar == CR_CHR) || (achar == SPC_CHR))
		break;
	    if (isdigit (achar)) {
		nbrk_out (achar);
		temp = (temp * 10) + (achar - '0');
		if (++count > digits)
		    break;
	    }
	    else {
		temp = minv - 1;
		break;
	    }
	}
	if ((temp >= minv) && (temp <= maxv))
	    break;
	else
	    sc_beep ();
    }
    return (temp);
}



int     get_chan ()
{
    extern int sc_clrlin();
    extern int sc_beep();
    int     achar,
            retvalue;

    while (TRUE) {
	sc_clrlin (1, 4, 4);
	cprintf ("Enter Channel Number [0-F] :");
	retvalue = -1;
	achar = inchar (NO_SHOW);
	if (isxdigit (achar))
	    if (isdigit (achar))
		retvalue = achar - '0';
	    else
		retvalue = achar - 'A' + 10;
	if (retvalue >= 0)
	    break;
	sc_beep ();
    }
    cprintf ("%2d", retvalue);
    return (retvalue);
}



int     yenterd (prompt)
char   *prompt;
{
    cprintf ("%s?:", prompt);
    return ((inchar (SHOW_IT) == 'Y') ? TRUE : FALSE);
}



int     inchar (echo_char)
int     echo_char;
{
    extern int  nbrk_inp();
    extern void nbrk_out();
    int     achar;

    while (TRUE) {
	achar = nbrk_inp ();
	if (achar <= 0xFF)
	    break;
    }
    achar &= 0xFF;
    if ((isalpha (achar)) && (islower (achar)))
	achar = toupper (achar);
    if (echo_char)
	nbrk_out (achar);
    return (achar);
}



int     key_hit ()
{
     extern int nbrk_inp();
            return ((nbrk_inp () <= 0xFF) ? TRUE : FALSE);
}




void wnd_char (dspchar)
int     dspchar;
{
    extern int  sc_beep();
    extern int  debug_mode;	/* declared in p_appl.c */
    extern void wn_dchar();

    dspchar &= 0x7F;		/* AND out ascii value */
    if (dspchar == NULL_CHR)
	if (debug_mode)
	    wn_dchar ('&');
	else
	    nothing;
    else
	if (dspchar == BELL_CHR)
	    sc_beep ();
	else
	    wn_dchar (dspchar);
    return;
}
