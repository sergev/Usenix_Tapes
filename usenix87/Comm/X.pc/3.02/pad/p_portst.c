/* -------------------------------------------------------------------------
	    Port Status Reporting and Signal Setting Routines
		     filename = p_portst.c

   The routines in this file are used by 'p_driver.c' to set and report port
   parameters, configure the 8250 device from the current port parameters,
   send a break signal, or report the status of the modem signals.

   set_prtprm    uses the caller's port parameters block to set the values in
		 the PAD's port parameters block, after some error checking.
   rep_prtprm    returns the PAD's port parameters to callers Port param blk.

   config_8250   uses the PAD's port parameters, sets the 8250 to those values.

   modem_signal  uses the 'func_code' value to call the appropriate async.
		 function.  Makes sure port is enabled before calling async.
   pad_mdmchg    called by the link layer when a modem status change interrupt
		 occurs.  May do auto-update of application field value.

   break_signal  Accepts a channel ^. PVC channels, sends Q-bit logical break.
		 For Virtual Call channels, sends protocol logical break.
		 Character channel, sends N-millisecond physical break signal.

   init_trmchar  Initializes the 'terminal characteristics' portion of a
		 channel's control block to values from 'p_config.d'.
   -------------------------------------------------------------------------
	 Date   Change   By   Reason
      05/31/84    01    curt  Initial Generation of Code (from 'p_driver.c').
      08/31/84    02    curt  Major changes for release 1.02.
      09/26/86    03    Ed M  Reformat and recomment
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "pad.h"

PCB_STR tport_prm;              /* this must be static to work OK. */



/* -------------------------------------------------------------------------
   This routine copies in the application's version of the Port parameter
   structure.  If validating checks pass, the application's version replaces
   the version currently in the dcb.port structure, and the 8250 is set to
   the values specified.
   ------------------------------------------------------------------------- */
void set_prtprm ()
{
    extern void mvbyt_fr(),
		link_write();
    extern int  ret_code,       /* from p_driver.c */
		net_xoffprc(),
		pad_xoffprc();

    mvbyt_fr (params.param_seg, params.param1, &tport_prm, PORT_PRMLEN);
 /* 
  When in packet mode, user cannot change from 8 bits data, no parity.
  */
    if (dcb.device.state == PACKET)
	if ((tport_prm.data_bits != 8) || (tport_prm.parity != NO_PARITY))
	    ret_code = ILL_PACKRQ;
 /* 
  If the above check didn't find error, check all parameters to make sure in
  the proper ranges.  This will prevent 8250 being set different that believe.
  */
    if (ret_code == FUNC_COMPLETE) {
	if ((tport_prm.baud < BAUD_110)
		|| (tport_prm.baud > BAUD_9600))
	    ret_code = ILL_BAUD;
	else if ((tport_prm.data_bits < DATA_5BITS) ||
		(tport_prm.data_bits > DATA_8BITS))
	    ret_code = ILL_BTDATA;
	else if ((tport_prm.parity < NO_PARITY) ||
		(tport_prm.parity > SPAC_PARITY))
	    ret_code = ILL_PARITY;
	else if ((tport_prm.stop_bits < STOP_1BIT) ||
		(tport_prm.stop_bits > STOP_2BIT))
	    ret_code = ILL_BTSTOP;
	else if ((tport_prm.stop_bits == STOP_15BIT) &&
		(tport_prm.data_bits != DATA_5BITS))
	    ret_code = ILL_BTSTOP;
	else if ((tport_prm.DTE_port != PORT_DCE) &&
		(tport_prm.DTE_port != PORT_DTE))
	    ret_code = ILL_DTEDCE;
    }
 /* 
  If didn't set error code to non-zero, copy in values and call 8250-setter.
  */
    if (ret_code == FUNC_COMPLETE) {
	dcb.port.baud = tport_prm.baud;
	dcb.port.data_bits = tport_prm.data_bits;
	dcb.port.parity = tport_prm.parity;
	dcb.port.stop_bits = tport_prm.stop_bits;
	if ((dcb.port.xon_xoff == TRUE) && (tport_prm.xon_xoff == FALSE))
	    if (dcb.device.port_numb != 0) {
		/* 
		 if active port, clean up xoff stuff
		 */
		net_xoffprc (FLUSH);
		if (dcb.port.pad_xoffed == TRUE)
		    pad_xoffprc (XON);
		else
		    link_write (LWRIT_NUL, NULL);
	    }
	dcb.port.pad_xoffed = dcb.port.net_xoffed = FALSE;
	dcb.port.xon_xoff = tport_prm.xon_xoff;
	dcb.port.DTE_port = tport_prm.DTE_port;
	if (dcb.device.port_numb != 0)
	    config_8250 ();
    }
    return;
}



/* -------------------------------------------------------------------------
   Simply copy the current port parameter block over to the application's
   data area indicated in the Parameter pointer block of the call. There
   are 5 integer fields that the user can have copies of, move 10 bytes.
   ------------------------------------------------------------------------- */
void rep_prtprm ()
{
    extern void mvbyt_to();

    mvbyt_to (params.param_seg, params.param1, &dcb.port, PORT_PRMLEN);
    return;
}



/* -------------------------------------------------------------------------
   This routine takes the current settings of the port configuration from
   the port control block, and calls the ASM routine that puts it into effect.
   ------------------------------------------------------------------------- */
void config_8250 ()
{
    extern void set_lkch();
    int     lin_char;
 /* 
  Convert PAD version of port to 8250 bitmap version, then shift into field.
  */
    switch (dcb.port.parity) {
	case NO_PARITY: 
	    lin_char = 0;
	    break;
	case ODD_PARITY: 
	    lin_char = 1;
	    break;
	case EVEN_PARITY: 
	    lin_char = 3;
	    break;
	case MARK_PARITY: 
	    lin_char = 5;
	    break;
	case SPAC_PARITY: 
	    lin_char = 7;
	    break;
    }
    lin_char <<= 3;
 /* 
  8250 field is set to 1 if 1.5 or 2 stop bits: OR in bit if NOT STOP_1BIT.
  Convert PAD version of data bits into 8250 bitmap version, shift into field.
  Shift PAD version of baud rate into left byte, OR into the line char field.
  */
    if (dcb.port.stop_bits != STOP_1BIT)
	lin_char |= 0x04;
    lin_char |= (dcb.port.data_bits - DATA_5BITS);
    lin_char |= (dcb.port.baud << 8);
    set_lkch (lin_char);
    return;
}



/* -------------------------------------------------------------------------
   This routine first verifies that a port is enabled for the device.  If not
   it returns an error code.  If a port is enabled, it sets the 8250 modem
   signals as specified.  If a signal report is requested, returns setting.
   ------------------------------------------------------------------------- */
void modem_signal ()
{
    extern void mvwrd_to(),
		setofdtr(),
		setondtr(),
		setofrts(),
		setonrts();
    extern int  func_code,              /* from p_driver.c */
		ret_code,               /* ditto */
		rep_mdst();
    int     mask;

    if (dcb.device.port_numb == 0)
	ret_code = ILL_PORTNUM;
    else {
	switch (func_code) {
	    case SET_DTR_ON: 
		setondtr ();
		break;
	    case RESET_DTR: 
		setofdtr ();
		break;
	    case SET_RTS_ON: 
		setonrts ();
		break;
	    case RESET_RTS: 
		setofrts ();
		break;
	    case TEST_DSR: 
	    case TEST_DCD: 
	    case TEST_RING: 
		/* 
		 Get the modem signals from the Modem Status Register, masking
		 out out the right bit for the signal indicated (only one). 
		 If result of AND is greater than 0, the bit is ON, return
		 TRUE; else FALSE. Output result directly into caller's
		 location specified.      
		 */
		if (func_code == TEST_DSR)
		    mask = 0x20;
		else if (func_code == TEST_RING)
		    mask = 0x40;
		else if (func_code == TEST_DCD)
		    mask = 0x80;
		mvwrd_to (params.param_seg, params.param1,
			((rep_mdst () & mask) > 0) ? TRUE : FALSE);
		break;
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine is called by the link layer when a change occurs to one of
   the modem status signals (DCD, RING, DSR, CTS).
   ------------------------------------------------------------------------- */
void pad_mdmchg (mdmstat)
int     mdmstat;
{
    extern void mvwrd_to();

    if (dcb.seg_mdmupd != NULL)
	mvwrd_to (dcb.seg_mdmupd, dcb.off_mdmupd, mdmstat);
    return;
}

/* ------------------------------------------------------------------------
   break_time() sets the value "t_brkflg" (back) to FALSE when called by
   T/O
   ------------------------------------------------------------------------ */
static int t_brkflg;

void break_time()
{
    t_brkflg = FALSE;
}

/* -------------------------------------------------------------------------
   Send the appropriate form of break signal, given state of device.  If in
   character state, send a physical BREAK over wire.  If in packet state, use
   channel number to build a logical BREAK (Q-bit) packet and output it.
   ------------------------------------------------------------------------- */

void break_signal (ptr_chcb)
CHCB_PTR ptr_chcb;
{
    long    count;
    if (dcb.device.state == CHARACTER) {
	/*
	 In character mode, set a timeout for 2 events later on
	 break_time(), turn on break, and wait for the timeout to end.
	 This gives a break length of 333 to 389 ms, as compared with the
	 variable length of the software timing loop we used before.
	 */
	int_off ();
	if (t_brkflg != TRUE) {
	    t_brkflg = TRUE;
	    count = 0;
	    set_time (2, break_time, 1);
	    int_on ();
	    setonbrk ();

	    while (t_brkflg != FALSE) {
		count = (count + 1);
	    }
	    setofbrk ();
	}
	else {
	    int_on ();
	}
    }
    else {
	snd_qbitpkt (ptr_chcb, QBRK_BEG);
	snd_qbitpkt (ptr_chcb, QBRK_END);
    }
    return;
}

/* -------------------------------------------------------------------------
   This routine initializes the terminal characteristics to values found in
   the 'p_config.d' system configuration file.
   ------------------------------------------------------------------------- */
void init_trmchar (chan)
int     chan;
{
    dcb.ch[chan].tc.echo_ok = DEF_ok_echo;
    dcb.ch[chan].tc.echo_enb = DEF_enb_echo;
    dcb.ch[chan].tc.echomci_on = DEF_echo_mci;
    dcb.ch[chan].tc.editmci_on = DEF_edit_mci;
    dcb.ch[chan].tc.def_echo = DEF_def_echo;
    dcb.ch[chan].tc.ech_ctli = DEF_ctli_ech;
    dcb.ch[chan].tc.ech_ctlh = DEF_ctlh_ech;
    dcb.ch[chan].tc.ech_escp = DEF_escp_ech;
    dcb.ch[chan].tc.ech_lfoncr = DEF_lfoncr_ech;
    dcb.ch[chan].tc.ech_cronlf = DEF_cronlf_ech;
    dcb.ch[chan].tc.forwrd_chr = DEF_chr_forwrd;
    dcb.ch[chan].tc.forwrd_tim = DEF_tim_forwrd;
    dcb.ch[chan].tc.parity_enb = DEF_parity_enb;
    dcb.ch[chan].tc.green_out = FALSE;
    dcb.ch[chan].tc.red_out = FALSE;
    dcb.ch[chan].tc.io_active = FALSE;
    dcb.ch[chan].tc.backspc = DEF_bspace;
    dcb.ch[chan].tc.linedel = DEF_lindel;
    dcb.ch[chan].tc.linagin = DEF_linagn;
    return;
}
