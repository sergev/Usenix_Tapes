/* -------------------------------------------------------------------------
   This file contains call logic routines as used by 'P_APPL.EXE'.
	    filename = 'p_applcl.c'
   ------------------------------------------------------------------------- */

#include <ctype.h>
#include <p_padfnc.h>		/*  This the only include useful to the PAD invoker. */
#include <app.h>


char   *state_desc[] = {
    "Disconnect", "Call Acc Pndg", "Call Acc", "Connect", "Clear Cnf Pndg",
    "Inc Call Pndg", "Inc Call Arr", "Inc Acc Pndg", "Call Cleared", "?", "?", "?"
};

char   *clear_desc[] = {
    "Normal Call End", "No Ports", "Host Unavail", "Access Denied",
    "Network Unavailable", "Format Error", "Username Error", "Address Error",
    "Password Error", "Confirm Timeout", "Reset Packet", "Restart Packet",
    "Restart Ending", "Remote X.PC Lost", "MODEM Status Lost"
};



void fcalls ()
{
    extern int sc_clrlin();
    extern int get_chan();
    extern int inchar();

    int     okchar;

    sc_clrlin (1, 4, 5);
    dev_status (NO_SHOW);
    while (TRUE) {
	sc_clrlin (1, 3, 10);
	cprintf ("N(ET login, P(C login, I(ncoming, O(utgoing, S(tatus : ");
	okchar = inchar (SHOW_IT);
	if (okchar == SPC_CHR)
	    break;
	switch (okchar) {
	    case 'N': 
		netlogin (get_chan ());
		break;
	    case 'P': 
		pc_login (get_chan ());
		break;
	    case 'I': 
		call_income (get_chan ());
		break;
	    case 'O': 
		call_outgo (get_chan ());
		break;
	    case 'S': 
		call_stloop (get_chan ());
		break;
	}
    }
    sc_clrlin (1, 3, 10);
    return;
}



call_income (achan)
int     achan;
{
    extern int sc_clrlin();
    extern int inchar();

    char    okchar;

    while (TRUE) {
	sc_clrlin (1, 5, 10);
	cprintf ("W(ait for call, R(ead request data, A(ccept, C(lear, S(tatus : ");
	okchar = inchar (SHOW_IT);
	if (okchar == SPC_CHR)
	    break;
	switch (okchar) {
	    case 'W': 
		call_wait (achan);
		break;
	    case 'R': 
		rdcall_data (achan);
		break;
	    case 'A': 
		call_accept (achan);
		break;
	    case 'C': 
		call_clr (achan);
		break;
	    case 'S': 
		call_stloop (achan);
		break;
	}
    }
    return;
}


call_outgo (achan)
int     achan;
{
    extern int sc_clrlin();
    extern int inchar();

    char    okchar;

    while (TRUE) {
	sc_clrlin (1, 5, 10);
	cprintf ("R(equest, I(nput accept data, C(lear, S(tatus : ");
	okchar = inchar (SHOW_IT);
	if (okchar == SPC_CHR)
	    break;
	switch (okchar) {
	    case 'R': 
		call_req (achan);
		break;
	    case 'I': 
		rdcall_data (achan);
		break;
	    case 'C': 
		call_clr (achan);
		break;
	    case 'S': 
		call_stloop (achan);
		break;
	}
    }
    return;
}



call_stloop (achan)
int     achan;
{
    extern void delay();
    extern int  key_hit();

    int     orig_state;

    call_state (achan, SHOW_IT, NO_WAIT);
    orig_state = ca[achan].call_state;
    while (ca[achan].call_state == orig_state) {
	call_state (achan, SHOW_IT, NO_WAIT);
	if (key_hit ())
	    orig_state = 99;
	else
	    delay (MEDIUM);
    }
    return;
}



void netlogin (achan)
{
    extern void	delay();
    extern void tty_chan();
    extern void sh_chanstat();
    extern int  sc_clrlin();
    extern int  sc_beep();
    extern int  get_str();
    extern int  inchar();
    extern int  key_hit();

    int     logged_in,
            linlen,
            rdcnt,
            fmt;

    sc_clrlin (1, 7, 15);
    cprintf ("Enter Request data (up to ^Z):");
    linlen = get_str (EOT_CHR, &ca[achan].out_arr[0], 60);
    fmt = 0;
    logged_in = FALSE;
    setp_xpc (achan, &ca[achan].out_arr[0], &linlen, &fmt, 0);
    call_xpc (achan, CALLRQ_SND);
    if (!REQST_DONE (achan))
	sh_chanstat (achan);
    else {
    /* call request sent, now wait for accept. */
	timr_run (achan, 40);
	while (TRUE) {
	    if (ca[achan].timer_upd == FALSE) {
		cprintf ("\n\rTIMEOUT waiting...");
		inchar (NO_SHOW);
		break;
	    }
	    if (key_hit ()) {
		cprintf ("\n\rABORTING login...");
		inchar (NO_SHOW);
		break;
	    }
	    call_state (achan, SHOW_IT, NO_WAIT);
	    if (ca[achan].call_state == CALL_ACCEPT) {
		sc_clrlin (1, 9, 9);
		linlen = LINE_LEN - 1;
		setp_xpc (achan, (int *) & ca[achan].in_arr[0], &linlen, &rdcnt, &fmt);
		call_xpc (achan, READ_CALDAT);
		call_state (achan, SHOW_IT, NO_WAIT);
		if ((REQST_DONE (achan)) && (ca[achan].call_state == CONNECTED)) {
		    sc_beep ();
		    delay (MEDIUM);
		    logged_in = TRUE;
		}
		else
		    sh_chanstat (achan);
		break;
	    }
	    else
		if (ca[achan].call_state == CALL_CLEARED) {
		    call_state (achan, SHOW_IT, WAIT_IT);
		    break;
		}
		else
		    delay (MEDIUM);
	}
    /* 
     At this point either got logged in or failed.  If logged in, do TTY.
     */
	timr_hlt (achan);
	if (logged_in)
	    tty_chan (achan);	/* will clear screen when done */
    }				/* else call request sent, now wait for accept. */
    if (!logged_in)
	sc_clrlin (1, 7, 15);
    return;
}



void pc_login (achan)
{
    extern void	delay();
    extern void tty_chan();
    extern void sh_chanstat();
    extern int  sc_clrlin();
    extern int  sc_beep();
    extern int  get_str();
    extern int  yenterd();
    extern int  inchar();
    extern int  key_hit();

    int     accpt_call,
            linlen,
            rdcnt,
            fmt,
            auto_acc;

    sc_clrlin (1, 7, 15);
    accpt_call = FALSE;
    call_xpc (achan, SET_CALLANS);
    if (!REQST_DONE (achan))
	sh_chanstat (achan);
    else {
	auto_acc = yenterd ("Auto-accept");
	cprintf ("     Waiting for a call");
	call_state (achan, SHOW_IT, NO_WAIT);
	timr_run (achan, 40);
	while (TRUE) {
	    if (ca[achan].timer_upd == FALSE) {
		cprintf ("\n\rTIMEOUT waiting...");
		inchar (NO_SHOW);
		break;
	    }
	    if (key_hit ()) {
		cprintf ("\n\rABORTING wait...");
		inchar (NO_SHOW);
		break;
	    }
	    call_state (achan, SHOW_IT, NO_WAIT);
	    delay (MEDIUM);
	    if (ca[achan].call_state == INCOME_CALL) {
		sc_clrlin (1, 9, 9);
		cprintf ("Call Request data read :[");
		while (TRUE) {
		    linlen = LINE_LEN - 1;
		    setp_xpc (achan, (int *) & ca[achan].in_arr[0], &linlen, &rdcnt, &fmt);
		    call_xpc (achan, READ_CALDAT);
		    if (REQST_DONE (achan))
			for (linlen = 0; linlen < rdcnt; linlen++)
			    putch (ca[achan].in_arr[linlen]);
		    call_state (achan, NO_SHOW, NO_WAIT);
		    if (ca[achan].call_state != INCOME_CALL)
			break;
		}
		putch (']');
		call_state (achan, NO_SHOW, NO_WAIT);
		if (ca[achan].call_state == ACCEPT_PND) {
		    if (auto_acc == TRUE)
			accpt_call = TRUE;
		    else
			accpt_call = yenterd ("    accept the call");
		}
		else
		    sh_chanstat (achan);
		break;
	    }
	    else
		if (ca[achan].call_state == CALL_CLEARED) {
		    call_state (achan, SHOW_IT, WAIT_IT);
		    break;
		}
	}
    }
    timr_hlt (achan);
    if (accpt_call) {
	if (auto_acc == TRUE)
	    linlen = 0;
	else {
	    cprintf ("\n\rEnter call accept data (up to ^Z):");
	    linlen = get_str (EOT_CHR, &ca[achan].out_arr[0], LINE_LEN - 1);
	}
	fmt = 0;
	setp_xpc (achan, &ca[achan].out_arr[0], &linlen, &fmt, 0);
	call_xpc (achan, CALLAC_SND);
	sh_chanstat (achan);
	if (REQST_DONE (achan))
	    call_state (achan, SHOW_IT, NO_WAIT);
	if (ca[achan].call_state == CONNECTED) {
	    sc_beep ();
	    delay (MEDIUM);
	    tty_chan (achan);
	}
	else {
	    cprintf ("\n\rERROR after call accept");
	    inchar (NO_SHOW);
	}
    }
    sc_clrlin (1, 7, 15);
    return;
}



void call_req (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  get_str();
    extern void sh_chanstat();

    int     fmt,
            linlen;

    call_state (achan, SHOW_IT, NO_WAIT);
    sc_clrlin (1, 7, 10);
    cprintf ("Enter Request data (up to ^Z) :");
    sc_clrlin (1, 8, 8);
    linlen = get_str (EOT_CHR, &ca[achan].out_arr[0], 50);
    fmt = 0;
    setp_xpc (achan, &ca[achan].out_arr[0], &linlen, &fmt, 0);
    call_xpc (achan, CALLRQ_SND);
    sh_chanstat (achan);
    if (REQST_DONE (achan))
	call_state (achan, SHOW_IT, WAIT_IT);
    sc_clrlin (1, 4, 10);
    return;
}




void call_clr (achan)
int     achan;
{
    extern int sc_clrlin();
    extern int get_int();
    extern int yenterd();
    extern int key_hit();

    int     clear_code,
            timeout;

    call_state (achan, SHOW_IT, NO_WAIT);
    while (TRUE) {
	sc_clrlin (1, 7, 8);
	clear_code = get_int (1, 7, "Enter clear cause code ", 0, 7, 1);
	cprintf ("   %s", clear_desc[clear_code]);
	timeout = get_int (1, 8, "Enter timeout in seconds ", 1, 60, 2);
	timeout *= EVENT_PSEC;
	if (yenterd ("     OK to clear")) {
	    setp_xpc (achan, &clear_code, &timeout, 0, 0);
	    call_xpc (achan, CALLCL_SND);
	    if (REQST_DONE (achan)) {
		sc_clrlin (1, 7, 8);
		cprintf ("\n\rHit key to abort wait...");
		while (!key_hit ())
		    call_state (achan, SHOW_IT, NO_WAIT);
	    }
	    break;
	}
	else
	    if (!yenterd ("   try again"))
		break;	
    }
    return;
}



void call_wait (achan)
int     achan;
{
    extern void sh_chanstat();

    call_xpc (achan, SET_CALLANS);
    sh_chanstat (achan);
    if (REQST_DONE (achan))
	call_state (achan, SHOW_IT, WAIT_IT);
    return;
}



void call_accept (achan)
int     achan;
{
    extern int  sc_clrlin();
    extern int  get_str();
    extern void sh_chanstat();

    int     linlen,
            fmt;

    sc_clrlin (1, 7, 10);
    cprintf ("Enter call accept data (up to ^Z):");
    linlen = get_str (EOT_CHR, &ca[achan].out_arr[0], LINE_LEN - 1);
    fmt = 0;
    setp_xpc (achan, &ca[achan].out_arr[0], &linlen, &fmt, 0);
    call_xpc (achan, CALLAC_SND);
    sh_chanstat (achan);
    if (REQST_DONE (achan))
	call_state (achan, SHOW_IT, WAIT_IT);
    return;
}



void rdcall_data (achan)
int     achan;
{
    extern void sh_chanstat();
    extern int  sc_clrlin();

    int     maxlen,
            rdcnt,
            fmt;

    call_state (achan, SHOW_IT, NO_WAIT);
    sc_clrlin (1, 8, 10);
    cprintf ("Call Request/Accept data read :[");
    while (TRUE) {
	maxlen = LINE_LEN - 1;
	setp_xpc (achan, (int *) & ca[achan].in_arr[0], &maxlen, &rdcnt, &fmt);
	call_xpc (achan, READ_CALDAT);
	sh_chanstat (achan);
	if (REQST_DONE (achan))
	    for (maxlen = 0; maxlen < rdcnt; maxlen++)
		putch (ca[achan].in_arr[maxlen]);
	call_state (achan, NO_SHOW, NO_WAIT);
	if ((ca[achan].call_state != INCOME_CALL)
		&& (ca[achan].call_state != CALL_ACCEPT))
	    break;
    }
    putch (']');
    call_state (achan, SHOW_IT, WAIT_IT);
    return;
}



void call_state (achan, disp_state, wait_state)
int     achan,
        disp_state,
        wait_state;
{
    extern int sc_clrlin();
    extern int sc_savscr();
    extern int sc_rstscr();
    extern int sc_beep();
    extern int inchar();

    int     clear_code;

    setp_xpc (achan, &ca[achan].call_state, &clear_code, 0, 0);
    call_xpc (achan, REP_CHNSTAT);
    if (disp_state) {
	sc_savcsr ();
	sc_clrlin (1, 6, 6);
	cprintf ("state of chan %2d = %2d, %s",
		achan, ca[achan].call_state, state_desc[ca[achan].call_state]);
	if (ca[achan].call_state == CALL_CLEARED) {
	    sc_beep ();
	    cprintf (" cause=%2d, %s", clear_code, clear_desc[clear_code]);
	    inchar (NO_SHOW);
	    wait_state = FALSE;
	    setp_xpc (achan, &ca[achan].call_state, &clear_code, 0, 0);
	    call_xpc (achan, REP_CHNSTAT);
	    ca[achan].is_open = FALSE;
	}
	sc_rstcsr ();
    }
    if ((ca[achan].call_state == CALL_CLEARED) && (!disp_state)) {
	sc_savcsr ();
	sc_clrlin (1, 1, 1);
	cprintf ("state of chan %2d = %2d, %s",
		achan, ca[achan].call_state, state_desc[ca[achan].call_state]);
	sc_beep ();
	cprintf (" cause=%2d, %s", clear_code, clear_desc[clear_code]);
	inchar (NO_SHOW);
	wait_state = FALSE;
	setp_xpc (achan, &ca[achan].call_state, &clear_code, 0, 0);
	call_xpc (achan, REP_CHNSTAT);
	ca[achan].is_open = FALSE;
	sc_rstcsr ();
    }
    else
	if (ca[achan].call_state == CONNECTED)
	    ca[achan].is_open = TRUE;
    if (wait_state)
	inchar (NO_SHOW);
    return;
}
