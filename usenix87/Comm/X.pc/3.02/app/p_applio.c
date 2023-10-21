/* -------------------------------------------------------------------------
   This file contains the Input/Output routines as used by the 'P_APPL.EXE'.
		  filename = 'p_applio.c'
   ------------------------------------------------------------------------- */
#include <ctype.h>
#include <stdio.h>
#undef	 NULL			/* avoids warning of redefinition from stdio.h */
#undef	 max			/* avoids warning of redefinition from stdio.h */
#undef	 min			/* avoids warning of redefinition from stdio.h */
#include <p_padfnc.h>		/*  This the only include useful to the PAD invoker. */
#include <app.h>



int     fonline ()
{
    extern int  sc_clrlin();
    extern int  yenterd();
    extern int  inchar();
    extern int  key_hit();
    extern void wn_clear();
    extern void delay();

    int     old_frow,
            old_lrow;

    dev_status (SHOW_IT);
    sc_clrlin (1, 6, 6);
    if (yenterd ("Reset link")) {
	device.port_numb = 0;
	setp_xpc (SYS_CH, &device, 0, 0, 0);
	call_xpc (SYS_CH, RESET_DEV);
	dev_status (SHOW_IT);
	sc_clrlin (18, 6, 6);
	cprintf ("1(200, 2(400, 4(800, 9(600 :");
	switch (inchar (SHOW_IT)) {
	    case '2': 
		port.baud = 5;
		break;
	    case '4': 
		port.baud = 6;
		break;
	    case '9':
		port.baud = 7;
		break;
	    default: 
		port.baud = 4;
		break;
	}
	port.data_bits = 8;
	port.parity = 0;
	port.stop_bits = 0;
	port.xon_xoff = TRUE;
	port.DTE_port = (yenterd ("   as DTE")) ? 1 : 0;
	setp_xpc (SYS_CH, &port, 0, 0, 0);
	call_xpc (SYS_CH, SET_PORTPRM);
	device.port_numb = (yenterd ("   using COM1")) ? 1 : 2;
	setp_xpc (SYS_CH, &device, 0, 0, 0);
	call_xpc (SYS_CH, RESET_DEV);
	call_xpc (SYS_CH, RESET_DTR);
	timr_run (SYS_CH, 3);
	if (REQST_DONE (SYS_CH))
	    while (ca[SYS_CH].timer_upd == TRUE)
		if (key_hit ())
		    break;
		else
		    delay (SHORT);
	call_xpc (SYS_CH, SET_DTR_ON);
	dev_status (SHOW_IT);
    }
    old_frow = ca[SYS_CH].firstrow;
    old_lrow = ca[SYS_CH].lastrow;
    ca[SYS_CH].firstrow = 1;
    ca[SYS_CH].lastrow = 24;
    tty_chan (SYS_CH);
    wn_clear ();
    ca[SYS_CH].firstrow = old_frow;
    ca[SYS_CH].lastrow = old_lrow;
    return;
}



int     fio_chan ()
{
    extern int  sc_clrlin();
    extern int  get_chan();
    extern int  inchar();
    extern void sh_chanstat();

    int     achan,
            inp_char;

    while (TRUE) {
	sc_clrlin (1, 3, 23);
	cprintf
	    ("T(ty, O(utput, I(nput, F(ilesend, C(kstatus, D(ump, S(tuff, W(indow :");
	inp_char = inchar (SHOW_IT);
	if (inp_char == SPC_CHR)
	    break;
	achan = (device.state == CHARACTER) ? 0 : get_chan ();
	if (ca[achan].is_open)
	    switch (inp_char) {
		case 'T': 
		    tty_chan (achan);
		    break;
		case 'O': 
		    out_chan (achan);
		    break;
		case 'I': 
		    inp_chan (achan);
		    break;
		case 'S': 
		    stf_chan (achan);
		    break;
		case 'F': 
		    file_send (achan);
		    break;
		case 'C': 
		    chk_iostat (achan);
		    break;
		case 'D': 
		    sc_clrlin (1, 7, 7);
		    cprintf ("Flush:  I(nput or O(output:");
		    switch (inchar (SHOW_IT)) {
			case 'I': 
			    call_xpc (achan, INPUT_FLUSH);
			    break;
			case 'O': 
			    call_xpc (achan, OUTPT_FLUSH);
			    break;
		    }
		    sh_chanstat (achan);
		    break;
		case 'W': 
		    get_window (achan);
		    break;
	    }
    }
    sc_clrlin (1, 3, 23);
    return;
}



int     tty_chan (achan)
int     achan;
{
    extern int  sc_savscr();
    extern int  sc_rstscr();
    extern int  nbrk_inp();
    extern int  debug_mode;
    extern void wn_setsz();
    extern void wn_clear();
    extern void wn_dstrg();
    extern void wn_curcl();
    extern void wnd_char();
    extern void sh_chanstat();
    
    int     done,
            fmt,
            rdcnt,
            maxlen,
            tab_cnt,
            achar,
            oldfirst,
            oldlast;

    oldfirst = ca[achan].firstrow;
    oldlast = ca[achan].lastrow;
    if (!debug_mode)
	wn_setsz (ca[achan].firstcol, ca[achan].firstrow,
		ca[achan].lastcol, ca[achan].lastrow);
    else {
	maxlen = (ca[achan].firstrow == 1) ? 2 : ca[achan].firstrow;
	fmt = (ca[achan].lastrow == 24) ? 23 : ca[achan].lastrow;
	wn_setsz (ca[achan].firstcol, maxlen, ca[achan].lastcol, fmt);
    }
    wn_clear ();
    wn_dstrg ("Type ^Z to exit TTY mode\n\r");
    ca[achan].out_arr[0] = NULL_CHR;
    done = FALSE;
    while (done == FALSE) {
	get_iostat (achan);
	if ((ca[achan].xmit_status)
		&& (ca[achan].out_arr[0] != NULL_CHR)) {
	    maxlen = 1;	
	    fmt = 0;
	    setp_xpc (achan, (int *) & ca[achan].out_arr[0], &maxlen, &rdcnt, &fmt);
	    call_xpc (achan, WRITE_DATA);
	    ca[achan].out_arr[0] = NULL_CHR;
	}
	if (debug_mode) {
	    sc_savcsr ();
	    sh_chanstat (achan);
	    sc_rstcsr ();
	}
	get_iostat (achan);
	if (ca[achan].recv_status) {
	    if (port.baud <= 4)
		maxlen = 10;
	    else
		if (port.baud == 5)
		    maxlen = 30;
		else
		    maxlen = 50;
	    setp_xpc (achan, (int *) & ca[achan].in_arr[0], &maxlen, &rdcnt, &fmt);
	    call_xpc (achan, READ_DATA);
	    if (REQST_DONE (achan))
		for (maxlen = 0; maxlen < rdcnt; maxlen++)
		    if (ca[achan].in_arr[maxlen] != TAB_CHR)
			wnd_char (ca[achan].in_arr[maxlen]);
		    else {
			tab_cnt = how_many (wn_curcl ());
			for (fmt = 1; fmt < tab_cnt; fmt++)
			    wnd_char (SPC_CHR);
		    }
	}
	achar = nbrk_inp ();
	if (achar < NO_CHAR) {
	    ca[achan].out_arr[0] = achar & 0xFF;
	    if (ca[achan].out_arr[0] == NULL_CHR) {
		achar = nbrk_inp ();
		ca[achan].out_arr[0] = achar & 0xFF;
		if (ca[achan].out_arr[0] == 83)
		    ca[achan].out_arr[0] = 0x7F;
		else
		    ca[achan].out_arr[0] = NULL_CHR;
	    }
	}
	call_state (achan, NO_SHOW, NO_WAIT);
	done = (ca[achan].call_state != CONNECTED);
	if (!done)
	    done = (ca[achan].out_arr[0] == EOT_CHR);
    }
    wn_setsz (ca[achan].firstcol, oldfirst, ca[achan].lastcol, oldlast);
    wn_clear ();
    return;
}



int     out_chan (achan)
int     achan;
{
    extern int  nbrk_inp();
    extern void wn_setsz();
    extern void wn_clear();
    extern void wn_dstrg();
    extern void wn_setdf();
    extern void wnd_char();
    extern void sh_chanstat();

    int     maxlen,
            fmt,
            wrcnt,
            value,
            endtext;

    wn_setsz (1, 7, 80, 23);
    wn_clear ();
    wn_dstrg ("OUTgoing characters (enter ^Z+CR to quit) :\n\r");
    endtext = FALSE;

    while (endtext == FALSE) {
	get_iostat (achan);
	if (ca[achan].xmit_status == TRUE) {
	    wnd_char ('>');
	    maxlen = 0;
	    while (maxlen < 74) {
		while (TRUE) {
		    value = nbrk_inp ();
		    if (value <= 0xFF)
			break;
		}
		ca[achan].out_arr[maxlen++] = value;
		wnd_char (value);
		if (value == CR_CHR) {
		    wnd_char (LF_CHR);
		    break;
		}
	    }

	    if (ca[achan].out_arr[0] == EOT_CHR) {
		endtext = TRUE;
		break;
	    }
	    fmt = wrcnt = 0;
	    setp_xpc (achan, (int *) & ca[achan].out_arr[0], &maxlen, &wrcnt, &fmt);
	    call_xpc (achan, WRITE_DATA);
	    if (!REQST_DONE (achan))
		sh_chanstat (achan);
	}

	if (!endtext) {
	    call_state (achan, NO_SHOW, NO_WAIT);
	    endtext = (ca[achan].call_state != CONNECTED);
	}
    }

    wn_clear ();
    wn_setdf ();
    return;
}



int     inp_chan (achan)
int     achan;
{
    extern int  key_hit();
    extern void wn_setsz();
    extern void wn_clear();
    extern void wn_dstrg();
    extern void wn_setdf();
    extern void wnd_char();
    extern void sh_chanstat();

    int     fmt,
            maxlen,
            rdcnt,
            endtext;

    endtext = FALSE;
    wn_setsz (1, 8, 80, 23);
    wn_clear ();
    wn_dstrg ("INcoming characters read (hit any key to quit) :\n\r");
    while (!endtext) {
	get_iostat (achan);
	if (ca[achan].recv_status) {
	    maxlen = LINE_LEN - 1;
	    setp_xpc (achan, (int *) & ca[achan].in_arr[0], &maxlen, &rdcnt, &fmt);
	    call_xpc (achan, READ_DATA);
	    if (REQST_DONE (achan))
		for (maxlen = 0; maxlen < rdcnt; maxlen++)
		    wnd_char (ca[achan].in_arr[maxlen]);
	    else
		sh_chanstat (achan);
	}
	if (key_hit ())
	    endtext = TRUE;
	else {
	    call_state (achan, NO_SHOW, NO_WAIT);
	    endtext = (ca[achan].call_state != CONNECTED);
	}
    }

    wn_clear ();
    wn_setdf ();
    return;
}



void get_window (achan)
int     achan;
{
    extern int sc_clrlin();
    extern int get_int();
    extern int yenterd();

    sc_clrlin (1, 7, 11);
    if (yenterd ("Full [1-80, 1-25]")) {
	ca[achan].firstrow = ca[achan].firstcol = 1;
	ca[achan].lastrow = 24;
	ca[achan].lastcol = 80;
    }
    else {
	sc_clrlin (1, 8, 8);
	cprintf ("first row is :%2d", ca[achan].firstrow);
	if (yenterd ("   change"))
	    ca[achan].firstrow = get_int (30, 8, "ok", 1, 23, 2);
	sc_clrlin (1, 9, 9);
	cprintf ("first col is :%2d", ca[achan].firstcol);
	if (yenterd ("   change"))
	    ca[achan].firstcol = get_int (30, 9, "ok", 1, 79, 2);
	sc_clrlin (1, 10, 10);
	cprintf ("last row is :%2d", ca[achan].lastrow);
	if (yenterd ("  change"))
	    ca[achan].lastrow = get_int (30, 10, "ok", ca[achan].firstrow + 1, 24, 2);
	sc_clrlin (1, 11, 11);
	cprintf ("last col is :%2d", ca[achan].lastcol);
	if (yenterd ("  change"))
	    ca[achan].lastcol = get_int (30, 11, "ok", ca[achan].firstcol + 1, 80, 2);
    }
    return;
}



void get_iostat (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  sc_savscr();
    extern int  sc_rstscr();
    extern int  debug_mode;
    extern void delay();

    int     inok,
            outok,
            incnt;

    inok = outok = incnt = 0;
    setp_xpc (achan, &incnt, &inok, &outok, 0);
    call_xpc (achan, REP_IOSTAT);
    ca[achan].recv_status = (incnt > 0) ? TRUE : FALSE;
    ca[achan].xmit_status = (outok != 2) ? TRUE : FALSE;
    if (debug_mode) {
	sc_savcsr ();
	sc_clrlin (1, 24, 24);
	cprintf ("incnt %4d   inst %1d   outst %1d  recv  %c  xmit %c  ",
		incnt, inok, outok,
		((ca[achan].recv_status) ? 'Y' : 'N'),
		((ca[achan].xmit_status) ? 'Y' : 'N'));
	sc_rstcsr ();
	delay (SHORT);
    }
    return;
}





void chk_iostat (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  key_hit();
    extern int  debug_mode;

    int     old_debug;

    old_debug = debug_mode;
    debug_mode = TRUE;
    while (!key_hit ())
	get_iostat (achan);
    sc_clrlin (1, 24, 24);
    debug_mode = old_debug;
    return;
}



int     get_rdcnt (achan)
int     achan;
{
    int     inok,
            outok,
            incnt;

    inok = outok = incnt = 0;
    setp_xpc (achan, &incnt, &inok, &outok, 0);
    call_xpc (achan, REP_IOSTAT);
    return (incnt);
}



/*-----------------------------------------------------------------------
- This function accepts the current position in a line, and returns the
- number of spaces that, if placed in the line, would cause a tab to the
- next tabstop.  The DOS system only compresses tabs as 8 blanks.
------------------------------------------------------------------------*/
how_many (cur_pos)
int     cur_pos;
{
    static int  tab_stops[18] = {
	0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80,
	88, 96, 104, 112, 120, 128, 132
    };
    int     count;
    for (count = 0; count <= 17; ++count)
	if (cur_pos < tab_stops[count])
	    return (tab_stops[count] - cur_pos);
    return (0);
}
