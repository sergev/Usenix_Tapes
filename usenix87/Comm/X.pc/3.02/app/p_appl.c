/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/
/* -------------------------------------------------------------------------
   This file is the main driver for 'P_APPL.EXE', an application program that
   makes use of all of the functions of the X.PC Driver.  It calls routines
   from 'p_applcl.c', and 'p_applio.c', 'p_applpt.c', 'p_applut.c'.
	 filename = 'p_appl.c'
   ------------------------------------------------------------------------- */

#include <ctype.h>
#include <p_padfnc.h>		/*  This the only include useful to the PAD invoker. */
#include <app.h>

#define   chans 16		/* character channel, 15 packet chans. */
CHNL_STR ca[chans];
PPB_STR params;
DSS_STR device;
PCB_STR port;


int     dev_num = 0;
int     appl_ds;		/* holds data segment of application. */
int     devst_char = '?';	/* char. showing mode driver is in.  */
int     debug_mode = FALSE;	/* when true, I/O calls show questat. */

extern int  xpcinvok (), xpccheck ();

void delay (count)
int     count;
{
    for (; count > 0; count--);
    return;
}



void setp_xpc (achan, p1, p2, p3, p4)
int     achan;
int     p1,
        p2,
        p3,
        p4;
{
    ca[achan].param1 = p1;
    ca[achan].param2 = p2;
    ca[achan].param3 = p3;
    ca[achan].param4 = p4;
    return;
}



int     call_xpc (achan, funct)
int     achan,
        funct;
{
    ca[achan].func_code = funct;
    params.param_seg = (int *) appl_ds;
    params.func_code = &ca[achan].func_code;
    params.device = &dev_num;
    params.channel = &ca[achan].channel;
    params.status = &ca[achan].status;
    params.param1 = (int *) ca[achan].param1;
    params.param2 = (int *) ca[achan].param2;
    params.param3 = (int *) ca[achan].param3;
    params.param4 = (int *) ca[achan].param4;
    xpcinvok (&params);
    return;
}



main ()
{
    extern int  get_ds();
    extern int  sc_clrscr();
    extern int  sc_clrlin();
    extern void fonline();
    extern void fio_chan();

    int     i,
            okchar;

    if (xpccheck () == FALSE)
	cprintf ("The X.PC Driver is not loaded...\n\r");
    else {
	appl_ds = get_ds ();
	cprintf
	    ("!!! IF YOU CAN SEE THIS, YOU NEED TO LOAD THE 'ANSI.SYS' DRIVER !!!\n\r");
	sc_clrscr ();
	setp_xpc (SYS_CH, &device, 0, 0, 0);
	call_xpc (SYS_CH, READ_DEVST);
	for (i = 0; i < chans; i++) {
	    ca[i].channel = i;
	    ca[i].firstrow = 5;
	    ca[i].lastrow = 23;
	    ca[i].firstcol = 1;
	    ca[i].lastcol = 80;
	}
	while (TRUE) {
	    sc_clrlin (1, 2, 3);
	    cprintf
		("D(ev, M(ode, O(nline, C(all, I(O, L(ink, T(imer, P(AD, Q(uit:");
	    okchar = inchar (SHOW_IT);
	    if (okchar == Q_CHR)
		break;
	    switch (okchar) {
		case 'D': 
		    fdevice ();
		    break;
		case 'M': 
		    fmode_set ();
		    break;
		case 'O': 
		    fonline ();
		    break;
		case 'C': 
		    fcalls ();
		    break;
		case 'I': 
		    fio_chan ();
		    break;
		case 'L': 
		    fport_op ();
		    break;
		case 'T': 
		    ftimer_op ();
		    break;
		case 'F': 
		    flip_dbug ();
		    break;
		case 'P': 
		    fpd_param ();
		    break;
		case ' ': 
		    sc_clrscr ();
		    break;
	    }
	}
	if (yenterd ("\n\rclear device"))
	    call_xpc (SYS_CH, CLEAR_DEV);
	sc_clrscr ();
    }
    _exit (0);			/* MUST USE _exit, OR WILL NOT RELEASE PROGRAM'S MEMORY!!! */
}



int     fdevice ()
{
    extern int sc_clrlin();
    int     achar;

    sc_clrlin (1, 3, 4);
    while (TRUE) {
	sc_clrlin (1, 3, 3);
	cprintf ("Device changes  [%c]  C(lear, R(eset, S(tatus :", devst_char);
	achar = inchar (SHOW_IT);
	if (achar == SPC_CHR)
	    break;
	switch (achar) {
	    case 'C': 
		if (yenterd ("    are you sure ")) {
		    call_xpc (SYS_CH, CLEAR_DEV);
		    dev_status (SHOW_IT);
		}
		break;
	    case 'R': 
		device.port_numb = get_int (60, 3, "port # ", 0, 2, 1);
		setp_xpc (SYS_CH, &device, 0, 0, 0);
		call_xpc (SYS_CH, RESET_DEV);
		dev_status (SHOW_IT);
		break;
	    case 'S': 
		dev_status (SHOW_IT);
		break;
	}
	if (REQST_DONE (SYS_CH))
	    sh_chanstat (SYS_CH);
    }
    sc_clrlin (1, 3, 4);
    return;
}



void dev_status (disp_stat)
int     disp_stat;
{
    extern int sc_clrlin();

    setp_xpc (SYS_CH, &device, 0, 0, 0);
    call_xpc (SYS_CH, READ_DEVST);
    if (disp_stat)
	sh_chanstat (SYS_CH);
    if (REQST_DONE (SYS_CH)) {
	switch (device.state) {
	    case WAIT_RESET: 
		devst_char = 'W';
		break;
	    case CHARACTER: 
		devst_char = 'C';
		break;
	    case PACKET: 
		devst_char = 'P';
		break;
	    default: 
		devst_char = '?';
		break;
	}
	sc_clrlin (1, 5, 5);
	if (disp_stat)
	    cprintf
		("state= %c  ver= %2d  rev= %2d  pad avail= %2d  port #= %1d  PVCs= %2d",
		    devst_char, device.version, device.revision, device.pad_avail,
		    device.port_numb, device.PVC_channels);
	if ((device.state != WAIT_RESET) && (device.port_numb != 0))
	    ca[SYS_CH].is_open = TRUE;
	else
	    ca[SYS_CH].is_open = FALSE;
    }
    return;
}



int     fmode_set ()
{
    extern int sc_clrlin();

    int     done;

    done = FALSE;
    while (!done) {
	sc_clrlin (1, 3, 4);
	cprintf ("[%s] C(haracter,  P(acket,  W(ait for packet, S(tart packet :",
		(port.DTE_port == PORT_DTE) ? "DTE" : "DCE");
	switch (inchar (SHOW_IT)) {
	    case 'C': 
		call_xpc (SYS_CH, SET_CHRMOD);
		sh_chanstat (SYS_CH);
		break;
	    case 'P': 
		call_xpc (SYS_CH, SET_PKTMOD);
		sh_chanstat (SYS_CH);
		break;
	    case 'W': 
		if (ca[SYS_CH].is_open)
		    wait_forit ();
		break;
	    case 'S': 
		if (ca[SYS_CH].is_open)
		    startpkt ();
		break;
	    case SPC_CHR: 
		done = TRUE;
		break;
	}
    }
    dev_status (NO_SHOW);
    sc_clrlin (1, 3, 5);
    return;
}



void wait_forit ()
{
    extern void get_iostat();
    extern int  sc_clrlin();
    extern int  sc_beep();

    int     done,
            esc_count,
            maxlen,
            rdcnt,
            fmt;

    esc_count = 0;
    done = FALSE;
    timr_run (SYS_CH, 20);
    sc_clrlin (1, 4, 4);
    cprintf ("waiting for packet mode, hit key to abort...");
    while (!done) {
	get_iostat (SYS_CH);
	if (ca[SYS_CH].recv_status) {
	    maxlen = LINE_LEN - 1;
	    setp_xpc (SYS_CH, (int *) & ca[SYS_CH].in_arr[0], &maxlen, &rdcnt, &fmt);
	    call_xpc (SYS_CH, READ_DATA);
	    if (REQST_DONE (SYS_CH))
		for (maxlen = 0; maxlen < rdcnt; maxlen++) {
		    if ((ca[SYS_CH].in_arr[maxlen] & 0x7f) == ESC_CHR)
			++esc_count;
		    if (esc_count >= 4) {
			done = TRUE;
			break;
		    }
		}
	}
	if (key_hit ())
	    done = TRUE;
	else
	    if (ca[SYS_CH].timer_upd == FALSE) {
		done = TRUE;
		cprintf ("\n\rTIMEOUT waiting");
		inchar (NO_SHOW);
	    }
    }
    if (esc_count >= 4) {
	call_xpc (SYS_CH, SET_PKTMOD);
	if (REQST_DONE (SYS_CH)) {
	    cprintf (" Packet mode...");
	    sc_beep ();
	    delay (SHORT);
	}
	else {
	    cprintf (" ERROR, not Packet mode...");
	    sh_chanstat (SYS_CH);
	}
    }
    if (ca[SYS_CH].timer_upd == TRUE)
	timr_hlt (SYS_CH);
    dev_status (SHOW_IT);
    return;
}



void startpkt ()
{
    extern int sc_clrlin();

    int     wrcnt,
            fmt,
            maxwrt;

    sc_clrlin (1, 4, 4);
    cprintf ("sending escapes...");
    for (wrcnt = 0; wrcnt < 6; wrcnt++)
	ca[SYS_CH].out_arr[wrcnt] = ESC_CHR;
    wrcnt = fmt = 0;
    maxwrt = 6;
    setp_xpc (SYS_CH, (int *) & ca[SYS_CH].out_arr[0], &maxwrt, &wrcnt, &fmt);
    call_xpc (SYS_CH, WRITE_DATA);
    sh_chanstat (SYS_CH);
    call_xpc (SYS_CH, SET_PKTMOD);
    sh_chanstat (SYS_CH);
    if (REQST_DONE (SYS_CH)) {
	cprintf ("packet mode set...");
	delay (SHORT);
    }
    else {
	cprintf ("packet mode NOT set...");
	inchar (NO_SHOW);
    }
    sc_clrlin (1, 4, 4);
    dev_status (SHOW_IT);
    return;
}
