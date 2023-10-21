/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/

/* -------------------------------------------------------------------------
   This file includes the port setting routines, and the PAD settings routines.
   Also included are the timer routines.  The code was moved out of p_appl.c.
   ------------------------------------------------------------------------- */

#include <ctype.h>
#include <p_padfnc.h>		/*  This the only include useful to the PAD invoker. */
#include <app.h>

int     mdm_stat = 0;		/* current modem status, look for chg. */



void fport_op ()
{
    extern int  sc_clrlin();
    extern int  inchar();
    int     again,
            which;

    while (TRUE) {
	sc_clrlin (1, 3, 5);
	cprintf ("M(odem,  C(har. :");
	which = inchar (SHOW_IT);
	switch (which) {
	    case 'M': 
		cprintf ("odem");
		again = TRUE;
		break;
	    case 'C': 
		cprintf ("har.");
		again = TRUE;
		break;
	    case SPC_CHR: 
		sc_clrlin (1, 3, 5);
		return;
	    default: 
		again = FALSE;
		break;
	}
	while (again) {
	    sc_clrlin (30, 3, 3);
	    cprintf ("S(et, R(eq :");
	    switch (inchar (SHOW_IT)) {
		case 'S': 
		    cprintf ("et");
		    switch (which) {
			case 'M': 
			    st_mdmstat ();
			    break;
			case 'C': 
			    st_lnkchr ();
			    break;
		    }
		    break;
		case 'R': 
		    cprintf ("eq");
		    switch (which) {
			case 'M': 
			    rq_mdmstat ();
			    break;
			case 'C': 
			    rq_lnkchr ();
			    break;
		    }
		    break;
		case SPC_CHR: 
		default: 
		    again = FALSE;
		    break;
	    }
	}
    }
    return;
}



void st_mdmstat ()
{
    extern int  sc_clrlin();
    extern int  get_chan();
    extern int  yenterd();
    extern int  inchar();
    extern void sh_chanstat();
    int     which,
            event,
           *where;

    sc_clrlin (1, 4, 4);
    cprintf ("D(TR, R(TS, B(reak, M(dm upd :");
    which = inchar (SHOW_IT);
    switch (which) {
	case 'D': 
	    call_xpc (SYS_CH,
		    (yenterd ("     Y (on), N (off)")) ? SET_DTR_ON : RESET_DTR);
	    break;
	case 'R': 
	    call_xpc (SYS_CH,
		    (yenterd ("     Y (on), N (off)")) ? SET_RTS_ON : RESET_RTS);
	    break;
	case 'B': 
	    call_xpc (((device.state == CHARACTER) ? 0 : get_chan ()), SEND_BREAK);
	    break;
	case 'M': 
	    event = MODEM_CHG;
	    where = &mdm_stat;
	    setp_xpc (SYS_CH, &event, &where, 0, 0);
	    call_xpc (SYS_CH,
		    (yenterd ("     Y (on), N (off)")) ? SET_UPDT : RESET_UPDT);
	    break;
    }
    sh_chanstat (SYS_CH);
    return;
}



void rq_mdmstat ()
{
#define  DELTA	0x7F
#define  BIT(x) (((t_status & x) != 0) ? 'Y' : 'N')

    extern int  sc_clrlin();
    extern int  sc_beep();
    extern int  inchar();
    extern int  key_hit();
    extern void delay();
    int     which,
            line_flag,
            t_status;

    sc_clrlin (1, 4, 4);
    cprintf ("D(SR, C(arrier, R(ing, M(dm upd :");
    which = inchar (SHOW_IT);
    if (which == 'M') {
	while (key_hit () == FALSE) {
	    t_status = mdm_stat;
	    mdm_stat &= 0xF0;
	    sc_clrlin (1, 4, 4);
	    cprintf (" %cCarrier %c  %cRING %c  %cDSR %c  %cCTS %c",
		    DELTA, BIT (8),
		    DELTA, BIT (4),
		    DELTA, BIT (2),
		    DELTA, BIT (1));
	    cprintf ("  Carrier %c  RING %c  DSR %c  CTS %c",
		    BIT (0x80), BIT (0x40),
		    BIT (0x20), BIT (0x10));
	    if ((t_status & 0x0F) != 0) {
		delay (LONG);
		sc_beep ();
	    }
	    delay (MEDIUM);
	}
	sc_clrlin (1, 4, 4);
    }
    else {
	setp_xpc (SYS_CH, &line_flag, 0, 0, 0);
	switch (which) {
	    case 'D': 
		cprintf ("SR");
		call_xpc (SYS_CH, TEST_DSR);
		break;
	    case 'C': 
		cprintf ("arrier");
		call_xpc (SYS_CH, TEST_DCD);
		break;
	    case 'R': 
		cprintf ("ing");
		call_xpc (SYS_CH, TEST_RING);
		break;
	    default: 
		return;
	}
	cprintf (" signal is %s", (line_flag) ? "ON " : "OFF");
    }
    return;
}



void st_lnkchr ()
{
    extern int  sc_clrlin();
    extern int  get_int();
    extern int  yenterd();
    extern void sh_chanstat();

    setp_xpc (SYS_CH, &port, 0, 0, 0);
    call_xpc (SYS_CH, REP_PORTPRM);
    sh_chanstat (SYS_CH);
    sc_clrlin (1, 6, 6);
    if (yenterd (" defaults")) {
	port.baud = 4;
	port.data_bits = 8;
	port.parity = 0;
	port.stop_bits = 0;
	port.xon_xoff = TRUE;
	port.DTE_port = 1;
    }
    else {
	sc_clrlin (1, 6, 6);
	sc_clrlin (1, 7, 7);
	cprintf ("Baud = %d", port.baud);
	if (yenterd ("   change"))
	    port.baud = get_int (30, 7, "ok ", 0, 7, 1);
	sc_clrlin (1, 8, 8);
	cprintf ("Data Bits= %d", port.data_bits);
	if (yenterd ("   change"))
	    port.data_bits = get_int (30, 8, "ok ", 5, 8, 1);
	sc_clrlin (1, 9, 9);
	cprintf ("Parity = %d", port.parity);
	if (yenterd ("   change"))
	    port.parity = get_int (30, 9, "ok ", 0, 4, 1);
	sc_clrlin (1, 10, 10);
	cprintf ("Stop bits = %d", port.stop_bits);
	if (yenterd ("   change"))
	    port.stop_bits = get_int (30, 10, "ok ", 0, 2, 1);
	sc_clrlin (1, 11, 11);
	cprintf ("XOFF enabled = %c", (port.xon_xoff == TRUE) ? 'Y' : 'N');
	if (yenterd ("   change")) {
	    sc_clrlin (30, 11, 11);
	    port.xon_xoff = yenterd ("ok ");
	}
	sc_clrlin (1, 12, 12);
	cprintf ("dxe (1=DTE) = %d", port.DTE_port);
	if (yenterd ("   change"))
	    port.DTE_port = get_int (30, 12, "ok ", 0, 1, 1);
    }
    setp_xpc (SYS_CH, &port, 0, 0, 0);
    call_xpc (SYS_CH, SET_PORTPRM);
    sh_chanstat (SYS_CH);
    sc_clrlin (1, 6, 12);
    rq_lnkchr ();
    return;
}



char   *baud_rate (n)
int     n;
{
    static char *baud_tbl[] = {
	" 110 ", " 150 ", " 300 ", " 600 ", "1200 ", "2400 ", "4800 ", "9600 "
    };
    return (baud_tbl[n]);
}



void rq_lnkchr ()
{
    extern int  sc_clrlin();
    extern void sh_chanstat();

    int     lkchar;

    setp_xpc (SYS_CH, &port, 0, 0, 0);
    call_xpc (SYS_CH, REP_PORTPRM);
    sh_chanstat (SYS_CH);
    sc_clrlin (1, 24, 24);
    if (REQST_DONE (SYS_CH)) {
	cprintf ("LINE char: baud %s", baud_rate (port.baud));
	cprintf ("  len= %1d", port.data_bits);
	switch (port.parity) {
	    case 0: 
		lkchar = 'N';
		break;
	    case 1: 
		lkchar = 'O';
		break;
	    case 2: 
		lkchar = 'E';
		break;
	    case 3: 
		lkchar = 'M';
		break;
	    case 4: 
		lkchar = 'S';
		break;
	    default: 
		lkchar = '?';
		break;
	}
	cprintf ("  parity= %c", lkchar);
	cprintf ("  stop= %1d", port.stop_bits);
	cprintf ("  xon_xoff= %c", (port.xon_xoff) ? 'Y' : 'N');
	cprintf ("  dxe= %s", (port.DTE_port == PORT_DTE) ? "DTE" : "DCE");
    }
    return;
}



void ftimer_op ()
{
    extern int  sc_clrlin();
    extern int  get_chan();
    extern int  get_int();
    extern int  inchar();
    int     done;

    done = FALSE;
    while (!done) {
	sc_clrlin (1, 3, 5);
	cprintf ("S(tart a timer, E(nd a timer, C(heck on timer :");
	switch (inchar (SHOW_IT)) {
	    case 'S': 
		timr_run ((device.state == CHARACTER) ? 0 : get_chan (),
			get_int (1, 5, "enter length in seconds", 0, 120, 3));
		break;
	    case 'E': 
		timr_hlt ((device.state == CHARACTER) ? 0 : get_chan ());
		sc_clrlin (1, 5, 5);
		cprintf ("ENDED...");
		break;
	    case 'C': 
		timr_check ();
		break;
	    case SPC_CHR: 
		done = TRUE;
		break;
	}
    }
    sc_clrlin (1, 3, 6);
    return;
}





void timr_run (achan, seconds)
int     achan,
        seconds;
{
    extern void sh_chanstat();
    int     upd_event,
            time_events,
           *temp_addr;		/* must have address of a variable containing
				   the address that is to be updated upon
				   timeout.
				 */
    ca[achan].timer_upd = TRUE;
    upd_event = TIMER_DONE;
    time_events = seconds * EVENT_PSEC;
    temp_addr = &ca[achan].timer_upd;
    setp_xpc (achan, &upd_event, &temp_addr, &time_events, 0);
    call_xpc (achan, SET_UPDT);
    sh_chanstat (achan);
    return;
}



void timr_hlt (achan)
int     achan;
{
    extern void sh_chanstat();
    int     upd_event;

    upd_event = TIMER_DONE;
    setp_xpc (achan, &upd_event, 0, 0, 0);
    call_xpc (achan, RESET_UPDT);
    sh_chanstat (achan);
    return;
}



void timr_check ()
{
    extern int  sc_clrlin();
    extern int  get_chan();
    extern int  key_hit();
    extern void delay();
    int     done,
            achan;

    achan = (device.state == CHARACTER) ? 0 : get_chan ();
    done = FALSE;
    sc_clrlin (1, 5, 5);
    cprintf ("Hit any key to stop check loop.");
    while (!done) {
	sc_clrlin (1, 6, 6);
	if (ca[achan].timer_upd == TRUE) {
	    cprintf ("RUNNING...");
	    delay (LONG);
	}
	else {
	    done = TRUE;
	    cprintf ("STOPPED...");
	    inchar (NO_SHOW);
	}
	if ((!done) && (key_hit ()))
	    done = TRUE;
    }
    sc_clrlin (1, 5, 6);
    return;
}



void fpd_param ()
{
    extern int  sc_clrlin();
    extern int  get_chan();
    extern int  inchar();
    int     done;

    dev_status (NO_SHOW);
    done = FALSE;
    while (!done) {
	sc_clrlin (1, 3, 7);
	cprintf ("PAD parameters:  S(et,  R(ead:");
	switch (inchar (SHOW_IT)) {
	    case 'S': 
		pdset_prm (get_chan ());
		break;
	    case 'R': 
		if (device.state == PACKET)
		    pdread_prm (get_chan ());
		break;
	    case SPC_CHR: 
		done = TRUE;
		break;
	}
    }
    sc_clrlin (1, 3, 7);
    return;
}



void pdread_prm (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  inchar();
    int     prm_parity,
            enb_parity;

    sc_clrlin (1, 6, 14);
    prm_parity = PARITY_ENABLE;
    setp_xpc (achan, &prm_parity, &enb_parity, 0, 0);
    call_xpc (achan, REP_PADPARM);
    if (REQST_DONE (achan)) {
	cprintf ("parity = %c\n\r", (enb_parity) ? 'Y' : 'N');
	prm_parity = ask_echo (achan, ECHO_ENABLE);
	cprintf ("echo enabled = %c\n\r", (prm_parity) ? 'Y' : 'N');
	if (prm_parity) {
	    cprintf ("TAB = %c\n\r", (ask_echo (achan, CTRL_I_ECHO)) ? 'Y' : 'N');
	    cprintf ("BACKSP = %c\n\r", (ask_echo (achan, CTRL_H_ECHO)) ? 'Y' : 'N');
	    cprintf ("ESC = %c\n\r", (ask_echo (achan, ESC_ECHO)) ? 'Y' : 'N');
	    cprintf ("LF on CR = %c\n\r", (ask_echo (achan, LFONCR_ECHO)) ? 'Y' : 'N');
	    cprintf ("CR on LF = %c\n\r", (ask_echo (achan, CRONLF_ECHO)) ? 'Y' : 'N');
	    prm_parity = FRWRD_CHAR;
	    setp_xpc (achan, &prm_parity, &enb_parity, 0, 0);
	    call_xpc (achan, REP_PADPARM);
	    cprintf ("FORWARD CHAR = %02x\n\r", enb_parity);
	    prm_parity = FRWRD_TIME;
	    setp_xpc (achan, &prm_parity, &enb_parity, 0, 0);
	    call_xpc (achan, REP_PADPARM);
	    cprintf ("FORWARD TIME = %02d\n\r", enb_parity);
	}
    }
    cprintf ("SPACE to continue");
    inchar (NO_SHOW);
    sc_clrlin (1, 6, 15);
    return;
}



void pdset_prm (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  yenterd();
    extern int  inchar();
    extern void sh_chanstat();
    int     enb_parity,
            prm_parity,
            input_chr;

    while (TRUE) {
	sc_clrlin (1, 6, 7);
	cprintf ("E(cho enable, P(arity enable, C(har forward, T(imeout forward :");
	input_chr = inchar (SHOW_IT);
	if (input_chr == SPC_CHR)
	    break;
	switch (input_chr) {
	    case 'E': 
		pad_echo (achan);
		break;
	    case 'C': 
		pad_cfor (achan);
		break;
	    case 'T': 
		pad_tfor (achan);
		break;
	    case 'P': 
		prm_parity = PARITY_ENABLE;
		sc_clrlin (1, 7, 7);
		setp_xpc (achan, &prm_parity, &enb_parity, 0, 0);
		call_xpc (achan, REP_PADPARM);
		if (REQST_DONE (achan)) {
		    cprintf ("parity = %c", (enb_parity) ? 'Y' : 'N');
		    if (yenterd ("   change")) {
			enb_parity = (yenterd ("   Y enables, N disables")) ? YES : NO;
			setp_xpc (achan, &prm_parity, &enb_parity, 0, 0);
			call_xpc (achan, SET_PADPARM);
			sh_chanstat (achan);
		    }
		}
		else
		    sh_chanstat (achan);
		break;
	}
    }
    return;
}



void pad_echo (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  yenterd();
    extern int  inchar();
    int     echo_enb;

    sc_clrlin (1, 7, 7);
    echo_enb = ask_echo (achan, ECHO_ENABLE);
    if (REQST_DONE (achan)) {
	cprintf ("echo enabled = %c", (echo_enb) ? 'Y' : 'N');
	if (yenterd ("   change"))
	    set_echo (achan, ECHO_ENABLE);
	if (ask_echo (achan, ECHO_ENABLE)) {
	    sc_clrlin (1, 8, 8);
	    cprintf ("echo ^I = %c", (ask_echo (achan, CTRL_I_ECHO)) ? 'Y' : 'N');
	    if (yenterd ("   change"))
		set_echo (achan, CTRL_I_ECHO);
	    sc_clrlin (1, 9, 9);
	    cprintf ("echo ^H = %c", (ask_echo (achan, CTRL_H_ECHO)) ? 'Y' : 'N');
	    if (yenterd ("   change"))
		set_echo (achan, CTRL_H_ECHO);
	    sc_clrlin (1, 10, 10);
	    cprintf ("echo ESC = %c", (ask_echo (achan, ESC_ECHO)) ? 'Y' : 'N');
	    if (yenterd ("   change"))
		set_echo (achan, ESC_ECHO);
	    sc_clrlin (1, 11, 11);
	    cprintf ("echo LF on CR = %c", (ask_echo (achan, LFONCR_ECHO)) ? 'Y' : 'N');
	    if (yenterd ("   change"))
		set_echo (achan, LFONCR_ECHO);
	    sc_clrlin (1, 12, 12);
	    cprintf ("echo CR on LF = %c", (ask_echo (achan, CRONLF_ECHO)) ? 'Y' : 'N');
	    if (yenterd ("   change"))
		set_echo (achan, CRONLF_ECHO);
	    cprintf ("\n\rEnter SPACE to continue...");
	    inchar (NO_SHOW);
	}
    }
    sc_clrlin (1, 7, 13);
    return;
}



int     ask_echo (achan, which_echo)
int     achan,
        which_echo;
{
    extern void sh_chanstat();
    int     prm_value;

    setp_xpc (achan, &which_echo, &prm_value, 0, 0);
    call_xpc (achan, REP_PADPARM);
    if (!REQST_DONE (achan))
	prm_value = NO;
    sh_chanstat (achan);
    if (prm_value == YES)
	return (TRUE);
    else
	return (FALSE);
}






void set_echo (achan, which_echo)
int     achan,
        which_echo;
{
    extern int  yenterd();
    extern void sh_chanstat();
    int     set_value;

    set_value = (yenterd ("   Y enables, N disables")) ? YES : NO;
    setp_xpc (achan, &which_echo, &set_value, 0, 0);
    call_xpc (achan, SET_PADPARM);
    sh_chanstat (achan);
    return;
}



void pad_cfor (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern void sh_chanstat();
    int     for_fld,
            for_char;

    for_fld = FRWRD_CHAR;
    setp_xpc (achan, &for_fld, &for_char, 0, 0);
    call_xpc (achan, REP_PADPARM);
    if (REQST_DONE (achan)) {
	sc_clrlin (1, 7, 7);
	cprintf ("forward char = %02x", for_char);
	if (yenterd ("   change")) {
	    sc_clrlin (40, 7, 7);
	    cprintf ("ok (enter hex) :");
	    cscanf ("%2x", &for_char);
	    for_fld = FRWRD_CHAR;
	    setp_xpc (achan, &for_fld, &for_char, 0, 0);
	    call_xpc (achan, SET_PADPARM);
	    sh_chanstat (achan);
	}
    }
    return;
}




void pad_tfor (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  get_int();
    extern void sh_chanstat();
    int     for_fld,
            for_time;

    for_fld = FRWRD_TIME;
    setp_xpc (achan, &for_fld, &for_time, 0, 0);
    call_xpc (achan, REP_PADPARM);
    if (REQST_DONE (achan)) {
	sc_clrlin (1, 7, 7);
	cprintf ("forward timeout = %02d", for_time);
	if (yenterd ("   change")) {
	    for_time = get_int (40, 7, "ok (time events) ", 0, 60, 2);
	    for_fld = FRWRD_TIME;
	    setp_xpc (achan, &for_fld, &for_time, 0, 0);
	    call_xpc (achan, SET_PADPARM);
	    sh_chanstat (achan);
	}
    }
    return;
}
