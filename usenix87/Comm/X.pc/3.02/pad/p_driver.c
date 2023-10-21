/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		   X.PC PAD: Interface to packet for Application
			  file name = p_driver.c

   This file contains the code that 'connects' the application layer to the
   packet layer.  The routines in the file implement device status changes,
   channel status changes, input/output from/to channels, formatting of link
   and modem values (report or set).  Basically all routines that support the
   X.PC product that are not utility or part of packet/link layers.
   -------------------------------------------------------------------------
	 Date   Change   By   Reason
      05/25/84    01    curt  Initial Generation of Code.
      06/14/84    02    curt  Version ready for testing completed.
      07/04/84    03    curt  Major cleanup of the Calling Logic routines.
      04/09/85    04    curt  Added i/o calls for MCI pad edit/echo.
			      Added driver function to report link statistics.
      07/31/85    05    curt  Completion of MCI echo code
      06/30/86    06    bobc  Stopgap change to "bpnd_pkout" curtailing WRITE's
			     usage of resources during FILESEND (w/RNR status.)
			     This change temporary pending design/define fixes.
      09/26/86    07    Ed M  Reformat and recomment
      10/29/86    08    Ed M  Change to data_read()'s call to mvpkt2buf() to 
			      compensate for amount of data potentially read
			      in previous call to mvpkt2buf() in the loop.
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "pkt.h"
#include "pad.h"

DCB_STR dcb;                    /* allocation of Device Control Blk */
PPB_STR params;                 /* allocation of Param. Pointer Blk */
int     func_code;              /* function code this call.          */
int     ret_code;               /* return status value this call.  */
int     call_chan;              /* used by routines this file only. */

DSS_STR temp_dss;               /* used by device state machine.   */
CHCB_PTR ptr_chcb;              /* pointer to channel control block */

#define     REP   1             /* For use within Driver for the   */
#define     SET   2             /* set/report pad parameters funct. */
#define     offclc(offset,index) ((unsigned) (offset) + (unsigned) (index))

extern  io_active ();


/* -------------------------------------------------------------------------
   This routine is main driver for application services.  Function code drives
   which routine to run directly.  Caller has moved from ES the 'inppb_seg'
   value, and from BX the 'inppb_off' value.
   ------------------------------------------------------------------------- */
int     x_driver (inppb_seg, inppb_off)
int    *inppb_seg;
PPB_PTR inppb_off;
{
    extern void set_update(),
		rst_update(),
		set_intrup(),
		rst_intrup(),
		chstat_rep(),
		call_ctrl(),
		pad_param(),
		set_prtprm(),
		rep_prtprm(),
		modem_signal(),
		break_signal(),
		in_flush(),
		out_flush(),
		mvbyt_fr(),
		mvwrd_fr(),
		mvwrd_to();             /* called from dispatch switch */

 /* 
  First copy in parameter address block, then copy in channel number if
  not character mode.  Check channel #, if not valid skip all calls.
  */
    mvbyt_fr (inppb_seg, inppb_off, &params, sizeof (PPB_STR));
    call_chan = 0;
    if (dcb.device.state == PACKET)
	mvwrd_fr (params.param_seg, params.channel, &call_chan);
    if ((call_chan < 0) || (call_chan > 15))
	ret_code = ILL_CHANNEL;
    else {
	ret_code = FUNC_COMPLETE;
	mvwrd_fr (params.param_seg, params.func_code, &func_code);
	ptr_chcb = &dcb.ch[call_chan];

	switch (func_code) {
	    case CLEAR_DEV: 
		clear_dev ();
		break;
	    case READ_DEVST: 
		rep_devstat ();
		break;
	    case RESET_DEV: 
		reset_dev ();
		break;
	    case SET_CHRMOD: 
		chr_statset ();
		break;
	    case SET_PKTMOD: 
		pck_statset ();
		break;
	    case SET_PORTPRM: 
		set_prtprm ();
		break;
	    case REP_PORTPRM: 
		rep_prtprm ();
		break;
	    case SET_DTR_ON: 
	    case RESET_DTR: 
	    case SET_RTS_ON: 
	    case RESET_RTS: 
	    case TEST_DSR: 
	    case TEST_DCD:      /* Fall through to modem signal handler */
	    case TEST_RING: 
		modem_signal ();
		break;
	    case SEND_BREAK: 
		break_signal (ptr_chcb);
		break;
	    case REP_IOSTAT: 
		iostat_rep ();
		break;
	    case READ_DATA: 
		data_read ();
		break;
	    case WRITE_DATA: 
		data_write ();
		break;
	    case REP_CHNSTAT: 
		chstat_rep (ptr_chcb);
		break;
	    case CALLRQ_SND: 
	    case CALLCL_SND: 
	    case SET_CALLANS: 
	    case CALLAC_SND:    /* fall through to call control function */
	    case READ_CALDAT: 
		call_ctrl (call_chan);
		break;
	    case SET_INTRP: 
		set_intrup (ptr_chcb);
		break;
	    case RESET_INTRP: 
		rst_intrup (ptr_chcb);
		break;
	    case SET_UPDT: 
		set_update (ptr_chcb);
		break;
	    case RESET_UPDT: 
		rst_update (ptr_chcb);
		break;
	    case REP_PADPARM: 
		pad_param (ptr_chcb, REP);
		break;
	    case SET_PADPARM: 
		pad_param (ptr_chcb, SET);
		break;
	    case INPUT_FLUSH: 
		in_flush (ptr_chcb);
		break;
	    case OUTPT_FLUSH: 
		out_flush (ptr_chcb);
		break;
	    default: 
		ret_code = ILL_FUNCTION;
		break;
	}
    }
    mvwrd_to (params.param_seg, params.status, ret_code);
    return;
}



/* -------------------------------------------------------------------------
   This routine puts the pad device back into the 'wait reset' state.  If the
   pad is already in wait reset state, the routine simply returns.  If the pad
   is in the 'char' or 'packet' state, the routine implements these actions:
   zero all data structures, drop DTR to off on the current port, disconnect
   the timer and the 8250 currently active, and any other cleanup tasks.
   The 'init_device' routine will set the device state to 'WAIT_RESET'.
   ------------------------------------------------------------------------- */
void clear_dev ()
{
    extern void mak_quepool(),
		mak_bufpool(),
		init_port(),
		init_device(),
		unld_tim(),
		unld_com(),
		term_825(),
		zero_mem();
    int         i;

    if (dcb.device.state != WAIT_RESET) {
	int_off ();
	if (dcb.device.port_numb != 0) {
	    unld_tim ();
	    term_825 ();
	    for (i = 1; i < 1000; i++);
	    unld_com (dcb.device.port_numb);
	}
	zero_mem ((char *) & dcb, sizeof (DCB_STR));
	zero_pkt ();
	mak_quepool ();
	mak_bufpool ();
	init_port ();
	init_device ();
	int_on ();
    }
    return;
}



/* -------------------------------------------------------------------------
   This function gives the caller a copy of the device status block that is
   maintained by the pad device. 'param1' is offset of caller's status block.
   ------------------------------------------------------------------------- */
void rep_devstat ()
{
    extern void mvbyt_to();

    mvbyt_to (params.param_seg, params.param1, &dcb.device, sizeof (DSS_STR));
    return;
}



/* -------------------------------------------------------------------------
   This routine is used to change the device state to 'char', from either the
   'wait reset' or 'packet' states.  If change is from 'wait', current port #
   WILL be zero.  If from 'packet', current port # MAY be zero.
   ------------------------------------------------------------------------- */
void reset_dev ()
{
    extern void load_tim(),
		load_com(),
		init_825(),
		strt_chrmode(),
		config_8250(),
		init_trmchar(),
		mvbyt_fr();

    mvbyt_fr (params.param_seg, params.param1, &temp_dss, sizeof (DSS_STR));
    if (dcb.device.port_numb != 0) {
	if (temp_dss.port_numb != 0)
	    ret_code = PORT_ACTIVE;
	else {
	    /*
	     Had previous port active, user wants to deactivate it.  Note that
	     the same as clear device, but the PAD stays in CHARACTER mode.
	     There will be no I/O until the user sets up new port number, and
	     resets again.
	     */
	    clear_dev ();

	    dcb.device.state = CHARACTER;
	    dcb.ch[0].call_state = DISCONNECT;
	    dcb.chan_cnt = 0;
	}
    }
    else {
	if ((temp_dss.port_numb == 1) || (temp_dss.port_numb == 2)) {
	    /* 
	     Had a previous port of zero, current port is valid, enter char
	     mode.  First set up Async. chip to handle interrupts, then set
	     up the link layer to handle character I/O.  strt_chrmode must
	     set up I/O process.  'Logch_map' will be zeros, this handles
	     updating on character chan.
	     */
	    dcb.device.port_numb = temp_dss.port_numb;
	    dcb.device.state = CHARACTER;
	    dcb.ch[0].call_state = CONNECTED;
	    rsrc_adjust (OPEN_CHAN);

	    int_off ();
	    load_tim ();
	    load_com (dcb.device.port_numb);
	    init_825 ();
	    config_8250 ();
	    init_trmchar (0);
	    int_on ();

	    strt_chrmode ();
	}
	else
	    ret_code = ILL_PORTNUM;
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine is used to exit PACKET state and return to the CHARACTER state,
   or restart CHARACTER state after 'no ques/buffs' abort in strt_inframe.
   If restarting CHARACTER state, call link layer input startup routine, but
   only if active port, and receive has been disabled due to overflows.
   If no channels open in PACKET state: terminate packet layer processing,
   reset data_bits and parity to character state settings, start CHARACTER
   state processing at the link level.
   ------------------------------------------------------------------------- */
void chr_statset ()
{
    extern int  recv_on();
    extern void strt_chrmode(),
		term_pktmode(),
		config_8250(),
		init_trmchar(),
		zero_mem();
    int     tchan;

    if (dcb.device.state == CHARACTER) {
	if ((dcb.device.port_numb != 0) && (recv_on () == FALSE))
	    link_init ();
    }
    else {
	if (dcb.device.state == WAIT_RESET)
	    ret_code = ILL_WAITRQ;
	else
	    if (dcb.chan_cnt > 0)
		ret_code = PCHANS_OPEN;
	    else {
		send_pktterm ();
		send_active ();
		int_off ();
		term_pktmode ();
		for (tchan = 0; tchan < 15; tchan++) {
		    zero_mem ((char *) & dcb.ch[tchan], sizeof (CHCB_STR));
		    dcb.logch_map[tchan] = NULL;
		}
		rsrc_adjust (INIT_VALS);
		zero_pkt ();
	    /* 
	     mak_quepool ();     * zero out the queue pools, and rebuild.*
	     mak_bufpool ();     * zero out the buffer pools, and rebuild*
	     */
		dcb.port.data_bits = dcb.port.char_databits;
		dcb.port.parity = dcb.port.char_parity;
		config_8250 ();
		dcb.device.state = CHARACTER;
		dcb.ch[0].call_state = CONNECTED;
		rsrc_adjust (OPEN_CHAN);
		init_trmchar (0);
		int_on ();
		strt_chrmode ();
	    }
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine is called to terminate CHARACTER state and enter PACKET state.
   It is not valid to call this while in WAIT_RESET state.  If in PACKET state,
   the function takes no action.  If called in CHARACTER state (proper),
   terminate the link level processing of characters, save databits/parity that
   is in force, reset both to values from X.PC Specification, reconfigure the
   8250 chip, then startup packet layer processing.
   ------------------------------------------------------------------------- */
void pck_statset ()
{
    extern void term_chrmode(),
		strt_pktmode(),
		config_8250(),
		init_trmchar(),
		writ_wrpkt(),
		zero_mem();
    int     i;

    if (dcb.device.state == WAIT_RESET)
	ret_code = ILL_WAITRQ;
    else {
	if (dcb.device.state == CHARACTER) {
	    if (dcb.device.port_numb == 0)
		ret_code = ILL_PORTNUM;
	    else {
		/* 
		 If there is data to send, send it out and wait for it to
		 finish.
		 */
		if (ptr_chcb -> apwr_que != NULL) {
		    writ_wrpkt (ptr_chcb);
		    send_active ();
		}
		int_off ();
		term_chrmode ();
	    /* 
	     zero_mem ((char *) &dcb.ch[0], sizeof (CHCB_STR));
	     zero_pkt ();        * zero LCB and initialize all CCBs *
	     mak_quepool ();     * zero out the queue pools, and rebuild *
	     mak_bufpool ();     * zero out the buffer pools, and rebuild *
	     */
		dcb.port.char_databits = dcb.port.data_bits;
		dcb.port.char_parity = dcb.port.parity;
		dcb.port.parity = NO_PARITY;
		dcb.port.data_bits = DATA_8BITS;
		config_8250 ();

		for (i = 1; i <= 15; i++)
		    init_trmchar (i);
		dcb.device.state = PACKET;
		rsrc_adjust (INIT_VALS);
		int_on ();
		strt_pktmode ((dcb.port.DTE_port == 0) ? PDCE_MODE : PDTE_MODE);
	    }
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine returns the status of the channel in terms of Input/Output
   processing.  It informs the caller the number of bytes waiting for input,
   and the input and output status of the channel.
   ------------------------------------------------------------------------- */
void iostat_rep ()
{
    extern void prc_qbitpkt(),
		net_xoffprc(),
		pad_read(),
		pad_write(),
		mvwrd_to();
    extern int  low_rsrc();
    int     in_stat = 0,        /* default to recv FALSE, no errors.     */
	    in_count = 0,       /* default to no bytes avail. to read. */
	    out_stat = 2;       /* default to xmit FALSE, no errors.     */

 /* 
  If anything but disconnected, try to read and write with the packet layer.
  */
    if (ptr_chcb -> call_state != DISCONNECT) {
	pad_read (ptr_chcb);
	pad_write (ptr_chcb);
	if (dcb.device.state == PACKET)
	    prc_qbitpkt (ptr_chcb);
	else {
	    int_off ();
	    net_xoffprc (CHECK);
	    int_on ();
	}
    }
    if (ptr_chcb -> call_state == CONNECTED) {
	/* 
	 If get to this point: have channel that it is connnected.  For the read
	 status, check data error and break received in the que entry. For
	 write, if the bufferlet/que entry pool is running low, xmit FALSE
	 (leave as is); if none queued, return 0;  If have data queued,
	 depends upon how much data being held by PAD: will be held due to
	 writing too fast, or blocked.
	 */
	if (ptr_chcb -> aprd_que != NULL)
	    if (dcb.device.state == CHARACTER) {
		if ((ptr_chcb -> aprd_que -> CHRERR_FLD & DATERR) != 0)
		    in_stat += 1;
		if ((ptr_chcb -> aprd_que -> CHRERR_FLD & BRKRCV) != 0)
		    in_stat += 2;
	    }
	    else
		if (ptr_chcb -> break_recvd)
		    in_stat += 2;
	if (ptr_chcb -> in_bytecnt < 0)
	    ptr_chcb -> in_bytecnt = 0;
	in_count = ptr_chcb -> in_bytecnt;
	if (low_rsrc () == FALSE) {
	/* 
	 &&  ((ptr_chcb->out_que.ent_cnt + ptr_chcb->in_que.ent_cnt)
	 < dcb.qmax_xoff))
	 let xmit write, when input is full       
	 */
	    if (ptr_chcb -> out_bytecnt <= 0) {
		out_stat = ptr_chcb -> out_bytecnt = 0;
	    }
	    else
		out_stat = (ptr_chcb -> out_bytecnt > dcb.bpnd_pkout) ? 2 : 1;
	}
    }
    mvwrd_to (params.param_seg, params.param1, in_count);
    mvwrd_to (params.param_seg, params.param2, in_stat);
    mvwrd_to (params.param_seg, params.param3, out_stat);
    return;
}



/* -------------------------------------------------------------------------
   Take bytes from the bufferlets in the aprd_que entry, the in_que, or the
   packet layer (in that order).  Terminate Read processing when:
      a) the application's buffer becomes full of input characters;
      b) No data is left in any of the queued bufferlets, and pack_read has
	 no data queued up in the packet layer.
      c) A Q-bit packet is found in the input queue (while in packet state, and
	 have already moved some data into the user's buffer).
   ------------------------------------------------------------------------- */
void data_read ()
{
    extern void rcv_qbitpkt(),
		next_rdpkt(),
		mv_pkt2buf(),
		mvwrd_to();
    extern int  set_time(),
		end_time(),
		gtwrd_fr();
    int     pos_iobuff,
	    max_iobuff;
 /* 
  Get basic info set up.  User only specifies channel # in packet state. If
  channel is not connected, or if no data waiting, return 'invalid function'.
  */
    ptr_chcb -> aprd_tot = 0;
    if (ptr_chcb -> call_state != CONNECTED)
	ret_code = ILL_CALSTAT;
    else
	if ((ptr_chcb -> recv_ready == FALSE) && (ptr_chcb -> in_bytecnt == 0))
	    ret_code = ILL_FUNCTION;

    if (ret_code == FUNC_COMPLETE) {
	if ((dcb.device.state == PACKET) && (TYMDFR_ECHO (ptr_chcb))) {
	    int_off ();
	    end_time (&io_active, ptr_chcb);
	    int_on ();
	}
    /* 
     Seems to be a valid call, with data available.  Set up maximum size of
     caller's buffer, then enter loop that reads what it can from input que.
     */
	pos_iobuff = 0;
	max_iobuff = gtwrd_fr (params.param_seg, params.param2);

	while (pos_iobuff < max_iobuff) {
	    /* 
	     If que entry is empty, ask for next.
	     */
	    if (ptr_chcb -> aprd_que == NULL)
		next_rdpkt (ptr_chcb);
	    /* 
	     If no current que entry, have run out of data.  Exit WHILE loop to
	     caller.
	     */
	    if (ptr_chcb -> aprd_que == NULL)
		break;
	    /* 
	     Now have a packet with bufferlets.  Action depends upon
	     mode + packet type.  If the que entry is a Q-bit packet, process
	     it.  If got Q-bit packet, and 'rcv' routine returns TRUE, must exit
	     because Q-bit upset state of chan.  If not a Q-bit, move as much of
	     the packet into the buffer as possible.
	     */
	    if (dcb.device.state == PACKET) {
		if ((ptr_chcb -> aprd_que -> u.cmd.p_gfilci & Q_BIT) == Q_BIT) {
		    if (rcv_qbitpkt (ptr_chcb) == MUST_EXIT)
			break;
		}
		else
		    pos_iobuff += mv_pkt2buf (ptr_chcb, params.param_seg,
			    offclc (params.param1, pos_iobuff),
			    max_iobuff - pos_iobuff);
	    }
	    else
		pos_iobuff += mv_pkt2buf (ptr_chcb, params.param_seg,
			offclc (params.param1, pos_iobuff), 
			max_iobuff - pos_iobuff);

	}
	ptr_chcb -> in_bytecnt -= pos_iobuff;
	mvwrd_to (params.param_seg, params.param3, pos_iobuff);
	mvwrd_to (params.param_seg, params.param4, 0);
	if ((dcb.device.state == PACKET) && (TYMDFR_ECHO (ptr_chcb)))
	    if (NO_BALLOUT (ptr_chcb)) {
		int_off ();
		set_time (NO_IOTIMER, &io_active, ptr_chcb);
		int_on ();
	    }
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine implements the transfer of data from the users buffer to
   bufferlets and packets as maintained by the PAD/X.PC driver.  All bytes must
   be moved in one call, so routines called must completely handle user data
   in one pass.  Style of data move depends upon state, and given state, which
   pad parameters are in effect for the channel. The # bytes moved is returned.
   ------------------------------------------------------------------------- */
void data_write ()
{
    extern void snd_qbitpkt(),
		prc_qbitpkt(),
		frwd_timr(),
		snd_asis(),
		mvbyt_fr(),
		mvwrd_to();
    extern int  set_time(),
		end_time(),
		gtwrd_fr(),
		echo_mci(),
		echo_tym();
    int     write_tot = 0,
	    max_iobuff;

    if (ptr_chcb -> call_state != CONNECTED)
	ret_code = ILL_CALSTAT;
    else
	if (gtwrd_fr (params.param_seg, params.param4) != 0)
	    ret_code = ILL_DATATYP;
    if (ret_code == FUNC_COMPLETE) {
	if (ptr_chcb -> tc.forwrd_tim > 0) {
	    int_off ();
	    end_time (&frwd_timr, ptr_chcb);
	    int_on ();
	}
	/* 
	 If in deferred echo, have appl input, so stop no activity timer.
	 If have sent a green ball, cancel it with a red ball.
	 */
	if (TYMDFR_ECHO (ptr_chcb)) {
	    int_off ();
	    end_time (&io_active, ptr_chcb);
	    int_on ();
	    if (GREEN_OUT (ptr_chcb))
		snd_qbitpkt (ptr_chcb, QRED_BALL);
	}

	/* 
	 If in char mode, no echo, or deferred TYmnet echo, move user's buffer
	 contents directly to packet for output.
	 */
	max_iobuff = gtwrd_fr (params.param_seg, params.param2);
	if ((dcb.device.state == CHARACTER) ||
		(!(ECHO_ON (ptr_chcb))) ||
		(TYMDFR_ECHO (ptr_chcb)))
	    write_tot = snd_asis (ptr_chcb,
		    params.param_seg, params.param1, max_iobuff);
	else {
	    /* 
	     Packet mode, and either tymnet or mci echo to get here.  If doing
	     echo, need to look at each byte so copy into io_buff to process.
	     Call proper echo, assuming only mci and local are possibilities.
	     */
	    max_iobuff = min (max_iobuff, BUF_MAX);
	    mvbyt_fr (params.param_seg, params.param1, &dcb.io_buff, max_iobuff);
	    if (MCI_ECHO (ptr_chcb))
		write_tot = echo_mci (ptr_chcb, max_iobuff);
	    else
		if (TYMLCL_ECHO (ptr_chcb))
		    write_tot = echo_tym (ptr_chcb, max_iobuff);
		else
		    write_tot = 0;
	}
    }
 /* 
  process all input qbit packets that are waiting in the input que.
  */
    prc_qbitpkt (ptr_chcb);
    mvwrd_to (params.param_seg, params.param3, write_tot);
    if (dcb.device.state == PACKET) {
	if ((write_tot > 0) && (ptr_chcb -> tc.forwrd_tim > 0)) {
	    int_off ();
	    set_time (ptr_chcb -> tc.forwrd_tim, &frwd_timr, ptr_chcb);
	    int_on ();
	}

	if ((TYMDFR_ECHO (ptr_chcb)) && (NO_BALLOUT (ptr_chcb))) {
	    int_off ();
	    set_time (NO_IOTIMER, &io_active, ptr_chcb);
	    int_on ();
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine re-computes the number of bytes outstanding that a channel
   can use, in the following XOFF/XON situations: when transmit is disabled
   at the packet or link layer, and bytes are being written; when characters
   have come in and have not been read by the application.  The number is also
   used for packet channels when the packet layer has blocked output.
   ------------------------------------------------------------------------- */
void rsrc_adjust (open_or_close)
int     open_or_close;
{
    extern int percent();
    int     byt_cnt,
	    que_cnt;

    if (open_or_close != INIT_VALS)
	dcb.chan_cnt += open_or_close;

    if ((dcb.chan_cnt == 0) || (open_or_close == INIT_VALS)) {
	dcb.chan_cnt = dcb.bpnd_pkout =
	    dcb.bmax_xoff = dcb.bmin_xoff = dcb.qmax_xoff = dcb.qmin_xoff = 0;
    }
    else {
	/* 
	 PAD gets 70% of que entries in character mode.  In Packet mode, PAD
	 only gets 50% of total, to split up between all channels that are open.
	 Both modes set counts to show total resources available, character is
	 percent of total resources available, packet is percent of PAD's
	 portion of total resources available.  XOFF/XON thresholds use both que
	 and buf counts.
	 */
	if (dcb.device.state == CHARACTER) {
	    que_cnt = percent (QUEPLCNT, 70);
	    byt_cnt = percent ((BUFPLCNT * BFLT_DATA_SIZE), 60);
	}
	else {
	    byt_cnt = dcb.chan_cnt * 2;
	    que_cnt = percent (QUEPLCNT, 50) / byt_cnt;
	    byt_cnt = percent ((BUFPLCNT * BFLT_DATA_SIZE), 50) / byt_cnt;
	}
	/* 
	 For XOFF/XON thresholds, use percentage of byt_cnt from above, for both
	 character/packet.  Character sends XOFF, while packet stops reading
	 from the packet layer, thus causing packet layer to fill up window.
	 Fix up bpnd_pkout to keep application from flooding bufferlets with
	 writes.
	 */
	dcb.bmax_xoff = percent (byt_cnt, 80);
	dcb.qmax_xoff = percent (que_cnt, 80);
	dcb.bmin_xoff = percent (byt_cnt, 60);
	dcb.qmin_xoff = percent (que_cnt, 60);
    /* 
     dcb.bpnd_pkout = percent (byt_cnt, 30);    xx    max queued w/no packwrite. */
    /* 
     filesend hybred queues/bytes.
     */
	dcb.bpnd_pkout = percent (byt_cnt, 03);
    }
    return;
}
