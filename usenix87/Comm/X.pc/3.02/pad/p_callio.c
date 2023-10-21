
/* -------------------------------------------------------------------------
	      Virtual and Logical 'CALL' Input/Output Logic
			filename = p_callio.c

   This file contains the routines that implement the input/output of calls
   to support both Virtual Circuit calls, and Permanent Virtual Circuit calls.

   prc_qbitpkt accepts a chan ^.  Processes all of the Q-bit input packets it
	       finds, upto a data packet, or no packets left.
   snd_qbitpkt accepts a chan ^ and a Q-bit packet type.  Sends any packet in
	       process. Makes a new packet containing Q-bit info per type.
   rcv_qbitpkt accepts a chan ^.  Assumes 'aprd_que' is pointing to a
	       Q-bit data packet.  Makes the echo, break, or 'call' logic work.
	       Returns TRUE if channel state as altered won't let read go on.

   ball_timr   accepts a chan ^.  When timeout expires, send a Red-Ball
   io_active   accepts a chan ^.  When timer expires, checks for activity on
	       that channel.  If none, sends a green ball for local echo.

   snd_calpkt  accepts a chan ^ (for PVC or VC channel type).  Moves any data
	       data associated into the call packet (request/clear/accept).
   snd_clrpkt  accepts a chan ^ (for PVC or VC channel type).  Moves the clear
	       cause code pointed to in parameter block to packet data area.
   snd_accpkt  accepts a chan ^ (for PVC or VC channel type).  Sends out a
	       call accepted packet in response to incoming call request.

   rcv_accpkt  accepts a chan ^ (for PVC or VC channel type).  Sets the state
	       of the channel to "CALL_ACCEPT", so the caller can read data.
   rcv_clrpkt  accepts a chan ^ (for PVC or VC channel type).  Sets the state
	       of the channel to "CALL_CLEARED", so the caller can read cause.
   rcv_cnfpkt  accepts a chan ^ (for PVC or VC channel type).  Sets the state
	       of the channel to "CALL_CLEARED", so the caller can read cause.
	       A clear confirm is only received as response to clear request.
   clr_cnftmr  Started when 'call clear' written, times out after 30 seconds,
	       will force chan to 'cleared' if still waiting on confirm.
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "pad.h"
#include "pkt.h"

#define  SAVE_PKT  TRUE         /* do not throw away contents of packet. */
#define  TOSS_PKT  FALSE        /* ok to throw away contents of packet.  */
int     io_active (), ball_timr (), clr_cnftmr ();
extern  clr_cnftmr ();


/* -------------------------------------------------------------------------
   Timeout set when Red Ball or Green Ball is sent
   Send Q-bit, data packet with red-ball (cancel any green ball)
   ------------------------------------------------------------------------- */
void ball_timr (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    if (TYMDFR_ECHO (ptr_2chcb))
	snd_qbitpkt (ptr_2chcb, QRED_BALL);
    return;
}



/* -------------------------------------------------------------------------
   This is run on a timer.  When it wakes up, if there is not a green ball
   out, one is sent out to attempt to enter local echo mode.
   ------------------------------------------------------------------------- */
void io_active (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    if (TYMDFR_ECHO (ptr_2chcb) && NOBALL_OUT (ptr_2chcb))
	snd_qbitpkt (ptr_2chcb, QGREEN_BALL);
    return;
}



/* -------------------------------------------------------------------------
   This routine processes all of the q-bit packets in the input que that it
   can find, up to a data packet, or until no packets remain to process.
   If the first packet in the write que is a Q-bit packet, it tries writing it.
   ------------------------------------------------------------------------- */
void prc_qbitpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void pad_write(),
		next_rdpkt();
    int         done;

    if (ptr_2chcb -> aprd_que == NULL)
	next_rdpkt (ptr_2chcb);
    done = FALSE;
    while (done == FALSE) {
	if (ptr_2chcb -> aprd_que == NULL)
	    done = TRUE;
	else {
	    if ((ptr_2chcb -> aprd_que -> u.cmd.p_gfilci & Q_BIT) == Q_BIT)
		if (rcv_qbitpkt (ptr_2chcb) == MUST_EXIT)
		    done = TRUE;
		else
		    next_rdpkt (ptr_2chcb);
	    else
		done = TRUE;
	}
    }
    if (ptr_2chcb -> out_que.ent_cnt != 0)
	if ((ptr_2chcb -> out_que.first -> u.cmd.p_gfilci & Q_BIT) == Q_BIT)
	    pad_write (ptr_2chcb);
    return;
}



/* -------------------------------------------------------------------------
   This routine will output a Q-BIT packet.  If there is currently a data
   packet being built (tot>0), that packet is put in the out_que.  The routine
   then sets up the que entry's data fields in the format of a Q-bit packet.
   The routine will just return if there are no resources to put the packet in.
   ------------------------------------------------------------------------- */
void snd_qbitpkt (ptr_2chcb, qbit_type)
CHCB_PTR ptr_2chcb;
int     qbit_type;
{
    extern void rel_que(),
		writ_wrpkt();
    extern int  set_time(),
		end_time(),
		strt_wrpkt(),
		ret_code;

 /*
  If any data pending, send it on out as a data packet.
  */
    if (ptr_2chcb -> apwr_tot > 0)
	writ_wrpkt (ptr_2chcb);
    if (ptr_2chcb -> apwr_que == NULL)
	if (strt_wrpkt (ptr_2chcb) == FALSE)
	    return;
    ptr_2chcb -> apwr_que -> u.cmd.p_gfilci = Q_BIT;
    ptr_2chcb -> apwr_que -> u.cmd.p_id = NORMAL_DATA;
    ptr_2chcb -> apwr_tot = 1;
    ptr_2chcb -> apwr_buf -> data[B_BYTE0] = qbit_type;
    ptr_2chcb -> apwr_buf -> num_bytes = B_BYTE1;
    switch (qbit_type) {
	case QENT_DEFECH: 
	    ptr_2chcb -> tc.echo_enb = FALSE;
	    break;
	case QLEV_DEFECH: 
	    /*
	     If leaving deferred echo, going to local echo (deferred = FALSE).
	     */
	    ptr_2chcb -> tc.def_echo = FALSE;
	    ptr_2chcb -> tc.echo_enb = TRUE;
	    ptr_2chcb -> tc.echomci_on = FALSE;
	    break;
	case QRED_BALL: 
	    /* 
	     Two reasons to send a red ball: got input while waiting for
	     reflected green ball; got echo disable while waiting for reflected
	     green ball.  If input, stay in deferred echo; if echo disable,
	     set flag false
	     */
	    int_off ();
	    if (ptr_2chcb -> call_state == CONNECTED) {
		ptr_2chcb -> tc.red_out = TRUE;
		ptr_2chcb -> tc.green_out = FALSE;
		end_time (ball_timr, ptr_2chcb);
		set_time (BALL_TIMER, ball_timr, ptr_2chcb);
	    }
	    int_on ();
	    break;
	case QGREEN_BALL: 
	    /* 
	     Will send green ball when there is no input/output activity
	     for 8 seconds.  Once send, check under timeout for lost or
	     unreflected
	     */
	    int_off ();
	    if (ptr_2chcb -> call_state == CONNECTED) {
		ptr_2chcb -> tc.red_out = FALSE;
		ptr_2chcb -> tc.green_out = TRUE;
		end_time (ball_timr, ptr_2chcb);
		set_time (BALL_TIMER, ball_timr, ptr_2chcb);
	    }
	    int_on ();
	    break;
	case QCALL_REQ: 
	    snd_calpkt (ptr_2chcb);
	    break;
	case QCALL_ACC: 
	    snd_accpkt (ptr_2chcb);
	    break;
	case QCALL_CLR: 
	    snd_clrpkt (ptr_2chcb);
	    break;
	/* QBRK_BEG, QBRK_END, QORANGE, and Q_CLEARCNF are already complete. */
    }

    if (ret_code == FUNC_COMPLETE)
	writ_wrpkt (ptr_2chcb);
    else {
	/* 
	 Throw packet away because it has an error in it.
	 */
	int_off ();
	rel_que (ptr_2chcb -> apwr_que, YES_BUFF);
	int_on ();
    }
 /* 
  strt_wrpkt() just exits if it has nothing to do.
  */
    strt_wrpkt (ptr_2chcb);
    return;
}



/* -------------------------------------------------------------------------
   This routine is called when a reader of the input que finds a Q-bit packet
   on its hands.  The routine examines the packet, and may alter the state
   of several aspects of the PAD's concept of the channels situation.  The
   PAD's concept of whether echo is enabled or not, whether echo is deferred
   or local (when echo is enabled), whether a break signal is in progress on
   the channel, may be altered.  'Logical call' packets are handled for the
   the Permanent Virtual Circuit channels. Those routines setting 'force_exit'
   to TRUE come upon a condition that will not allow further reading by the
   caller. On entry, assumed that 'ptr_2chcb->aprd_que' is pointing to the
   packet.

   04/02/85  all TYMNET-type Qbit packets now check to see if MCI-type Qbit
   processing is taking occuring, no TYMNET action taken during MCI process.
   ------------------------------------------------------------------------- */
int     rcv_qbitpkt (ptr_2chcb)
	CHCB_PTR ptr_2chcb;
{
    extern void frwd_timr(),
		next_rdbuf(),
		mvwrd_to(),
		edit_init(),
		edit_end();
    extern int  set_time(),
		end_time();
    int     exit_val,
	    dowith_pkt,
	    i;

    dowith_pkt = TOSS_PKT;
    exit_val = MUST_EXIT;
    exit_val = !exit_val;       /* so not doing a NOT on a constant value.       */

    switch (ptr_2chcb -> aprd_que -> u.cmd.bfirst -> data[B_BYTE0]) {
	case QENB_ECHO: 
	    /*
	     If get an 'echo on' packet, no action if user set 'echo_ok' to
	     FALSE.  Otherwise action depends upon state of red/green balls.
	     If defecho, and no balls out, start timer for I/O activity.
	     If red ball out, just set.
	     */
	    if ((ptr_2chcb -> tc.echo_ok == TRUE) && !MCI_ECHO (ptr_2chcb))
		if (!(TYMLCL_ECHO (ptr_2chcb))) {
		    if (NOBALL_OUT (ptr_2chcb)) {
			ptr_2chcb -> tc.def_echo = TRUE;
			int_off ();
			set_time (NOIO_TIMER, io_active, ptr_2chcb);
			ptr_2chcb -> tc.io_active = FALSE;
			int_on ();
		    }
		    else
			if (RED_OUT (ptr_2chcb))
			    ptr_2chcb -> tc.def_echo = TRUE;
		}
	    break;
	case QDSB_ECHO: 
	    /* 
	     Any receipt of a disable echo moves state to ECHO_OFF.  If red
	     ball out, just keep waiting.  If green, send red ball to cancel.    
	     */
	    if (GREEN_OUT (ptr_2chcb))
		snd_qbitpkt (ptr_2chcb, QRED_BALL);
	    else
		if (!(RED_OUT (ptr_2chcb)))
		    if (TYMDFR_ECHO (ptr_2chcb)) {
			int_off ();
			end_time (io_active, ptr_2chcb);
			int_on ();
		    }
		    else
			if (TYMLCL_ECHO (ptr_2chcb)) {
			    /* 
			     timer runs in local or remote echo, so leave on
			     (if on)
			     int_off ();
			     end_time (frwd_timr, ptr_2chcb);
			     int_on ();
			     */
			    snd_qbitpkt (ptr_2chcb, QENT_DEFECH);
			    echo_end (ptr_2chcb);
			}
	    ptr_2chcb -> tc.def_echo = ptr_2chcb -> tc.echo_enb = FALSE;
	    break;
	case QYELLOW_BALL: 
	    if (!MCI_ECHO (ptr_2chcb)) {
		snd_qbitpkt (ptr_2chcb, QORANGE_BALL);
		break;
	    }
	case QORANGE_BALL: 
	    if ((ptr_2chcb -> seg_ckpupd != NULL) && !MCI_ECHO (ptr_2chcb)) {
		mvwrd_to (ptr_2chcb -> seg_ckpupd, ptr_2chcb -> off_ckpupd, FALSE);
		ptr_2chcb -> seg_ckpupd = ptr_2chcb -> off_ckpupd = NULL;
		break;
	    }
	case QRED_BALL: 
	    /* 
	     RED_BALL in means host ate our GREEN_BALL, or reflected our
	     RED_BALL.  If there was a red out, start the I/O activity timer
	     for LDM later.
	     */
	    int_off ();
	    end_time (ball_timr, ptr_2chcb);
	    /* 
	     only set io active, if local echo is enabled
	     */
	    if (RED_OUT (ptr_2chcb) && TYMDFR_ECHO (ptr_2chcb)) {
		end_time (io_active, ptr_2chcb);
		int_on ();
		int_off ();
		set_time (NOIO_TIMER, io_active, ptr_2chcb);
		ptr_2chcb -> tc.io_active = FALSE;
	    }
	    ptr_2chcb -> tc.red_out = ptr_2chcb -> tc.green_out = FALSE;
	    int_on ();
	    break;
	case QGREEN_BALL: 
	    /* 
	     If no green out, ignore.  If green out, has gone to host and
	     back, means ok to enter local echo: send 'leave deferred echo'
	     packet.
	     */
	    if ((GREEN_OUT (ptr_2chcb)) && !MCI_ECHO (ptr_2chcb)) {
		ptr_2chcb -> tc.green_out = ptr_2chcb -> tc.red_out = FALSE;
		int_off ();
		end_time (ball_timr, ptr_2chcb);
		int_on ();
		if (ptr_2chcb -> tc.echo_ok)
		    snd_qbitpkt (ptr_2chcb, QLEV_DEFECH);
		echo_init (ptr_2chcb);
	    }
	    break;
	case QBRK_BEG: 
	    ptr_2chcb -> break_recvd = exit_val = MUST_EXIT;
	    break;
	case QBRK_END: 
	    ptr_2chcb -> break_recvd = FALSE;
	    break;
	case QCALL_REQ: 
	    dowith_pkt = rcv_calpkt (ptr_2chcb);
	    exit_val = MUST_EXIT;
	    break;
	case QCALL_ACC: 
	    dowith_pkt = rcv_accpkt (ptr_2chcb);
	    exit_val = MUST_EXIT;
	    break;
	case QCALL_CLR: 
	    rcv_clrpkt (ptr_2chcb);
	    exit_val = MUST_EXIT;
	    break;
	case QCLEAR_CNF: 
	    rcv_cnfpkt (ptr_2chcb);
	    exit_val = MUST_EXIT;
	    break;
	case QMCI_PECHO: 
	    if (!(ECHO_ON (ptr_2chcb))) {
		echo_init (ptr_2chcb);
		ptr_2chcb -> tc.echomci_on = ptr_2chcb -> tc.ech_lfoncr = TRUE;
	    }
	    break;
	case QMCI_DECHO: 
	    if (MCI_ECHO (ptr_2chcb))
		echo_end (ptr_2chcb);
	    ptr_2chcb -> tc.echomci_on = ptr_2chcb -> tc.ech_lfoncr = FALSE;
	    break;
	case QCHR_FRWRD: 
	    ptr_2chcb -> tc.forwrd_chr =
		ptr_2chcb -> aprd_que -> u.cmd.bfirst -> data[B_BYTE1];
	    break;
	case QTIM_FRWRD: 
	    if (ptr_2chcb -> tc.forwrd_tim > 0) {
		int_off ();
		end_time (frwd_timr, ptr_2chcb);
		int_on ();
	    }
	    ptr_2chcb -> tc.forwrd_tim =
		ptr_2chcb -> aprd_que -> u.cmd.bfirst -> data[B_BYTE1];
	    if (ptr_2chcb -> tc.forwrd_tim > 0) {
		int_off ();
		set_time (ptr_2chcb -> tc.forwrd_tim, frwd_timr, ptr_2chcb);
		int_on ();
	    }
	    break;
	case QMCI_YEDIT: 
	    /* 
	     must be perm echo, limited # of edit chans. 
	     */
	    if (MCI_ECHO (ptr_2chcb) &&
		    (!MCI_EDIT (ptr_2chcb)) &&
		    (dcb.ed_active < MAX_EDIT))/* pass backspc, linedel, lineagin chars  */
		edit_init (ptr_2chcb,
			ptr_2chcb -> aprd_que -> u.cmd.bfirst -> data[B_BYTE0 + 1],
			ptr_2chcb -> aprd_que -> u.cmd.bfirst -> data[B_BYTE0 + 2],
			ptr_2chcb -> aprd_que -> u.cmd.bfirst -> data[B_BYTE0 + 3]);
	    break;
	case QMCI_NEDIT: 
	    edit_end (ptr_2chcb);
	    break;
    }
 /* 
  Throw away packet if told to, by cycling through with next_rdbuff.
  */
    if (dowith_pkt == TOSS_PKT)
	while (ptr_2chcb -> aprd_que != NULL)
	    next_rdbuf (ptr_2chcb);
    return (exit_val);
}



/* -------------------------------------------------------------------------
   This routine handles the building of q-bit logical call request or Virtual
   Circuit call packets.  The call request has data associated with it that
   may be one byte or more, up to a packets worth.
   ------------------------------------------------------------------------- */
void snd_calpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern int  mv_buf2pkt(),
		gtwrd_fr(),
		ret_code;
    int        max_iobuff;

    max_iobuff = gtwrd_fr (params.param_seg, params.param2);
    if (mv_buf2pkt (ptr_2chcb, params.param_seg,
		params.param1, max_iobuff) != max_iobuff)
	ret_code = ILL_CALFORM;
    else
	ptr_2chcb -> call_state = CALLACC_PND;
    return;
}



/* -------------------------------------------------------------------------
   The call accept has 0 or more bytes associated, the accept string.
   ------------------------------------------------------------------------- */
void snd_accpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern int  mv_buf2pkt(),
		gtwrd_fr(),
		ret_code;
    int        max_iobuff;

    if (ptr_2chcb -> call_state == ACCEPT_PND) {
	max_iobuff = gtwrd_fr (params.param_seg, params.param2);
	if (mv_buf2pkt (ptr_2chcb, params.param_seg,
		    params.param1, max_iobuff) != max_iobuff)
	    ret_code = ILL_CALFORM;
	else
	    ptr_2chcb -> call_state = CONNECTED;
    }
    else
	ret_code = ILL_CALSTAT;
    return;
}



/* -------------------------------------------------------------------------
   This routine is run on a timer by the code that sends a call clear.  If the
   timer expires, and a call clear confirm has not been received, the channel
   is force to call cleared state.   The timer running for this routine will be
   stopped by the receipt of a clear confirm packet.
   ------------------------------------------------------------------------- */
void clr_cnftmr (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    if (ptr_2chcb -> call_state == CLEARCNF_PND) {
	ptr_2chcb -> call_state = CALL_CLEARED;
	ptr_2chcb -> clear_code = CLR_TIMEOUT;
    }
    return;
}



/* -------------------------------------------------------------------------
   The call clear has a single byte associated, the clear cause code. Start
   a timer, in case packet layer screwy, to force clear after a timeout.
   The caller specifies what the timeout is for giving up on clear confirm.
   ------------------------------------------------------------------------- */
void snd_clrpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern int  set_time(),
		gtwrd_fr(),
		ret_code;
    int     clear_code;

    clear_code = gtwrd_fr (params.param_seg, params.param1);
    if ((clear_code >= 0) && (clear_code <= 8)) {
	ptr_2chcb -> apwr_buf -> data[ptr_2chcb -> apwr_buf -> num_bytes++] = clear_code;
	ptr_2chcb -> apwr_tot++;
	ptr_2chcb -> call_state = CLEARCNF_PND;
	clear_code = gtwrd_fr (params.param_seg, params.param2);
	int_off ();
	set_time (clear_code, clr_cnftmr, ptr_2chcb);
	int_on ();
    }
    else
	ret_code = ILL_CLRCODE;
    return;
}



/* -------------------------------------------------------------------------
   Assumes 'aprd_que' pointing to a call request packet. Does not move any
   data (must use 'read call data' function).  Sets the channels state to
   incoming call, so status read by user will detect call came in.  If the
   channel is not in the 'incoming call pending' or 'incoming call' states,
   the packet is thrown away (can't throw away packet in use: incoming call).
   ------------------------------------------------------------------------- */
void rcv_calpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    if (ptr_2chcb -> call_state == INCOMCAL_PND) {
	ptr_2chcb -> call_state = INCOME_CALL;
	ptr_2chcb -> aprd_ind = B_BYTE1;
    }
    return ((ptr_2chcb -> call_state == INCOME_CALL) ? SAVE_PKT : TOSS_PKT);
}



/* -------------------------------------------------------------------------
   Assumes 'aprd_que' pointing to a call accept packet. Does not move any
   data (must use 'read call data' function).  Sets the channels state to
   call accepted, so status read by user will detect call came in.  If the
   channel is not in the 'call accept pending' or 'call accepted' states,
   the packet is thrown away (can't throw away packet in use: call accepted).
   ------------------------------------------------------------------------- */
void rcv_accpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    if (ptr_2chcb -> call_state == CALLACC_PND) {
	ptr_2chcb -> call_state = CALL_ACCEPT;
	ptr_2chcb -> aprd_ind = B_BYTE1;
    }
    return ((ptr_2chcb -> call_state == CALL_ACCEPT) ? SAVE_PKT : TOSS_PKT);
}



/* -------------------------------------------------------------------------
   This routine assumes that the 'aprd_que' entry contains a clear packet.
   If the channel is not DISCONNECT, the channels state is changed to
   CALL CLEARED and the cause code is moved to the channel control block.
   ------------------------------------------------------------------------- */
void rcv_clrpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    if (ptr_2chcb -> call_state != DISCONNECT) {
	ptr_2chcb -> call_state = CALL_CLEARED;
	ptr_2chcb -> clear_code = ptr_2chcb -> aprd_buf -> data[B_BYTE1];
	snd_qbitpkt (ptr_2chcb, QCLEAR_CNF);
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine assumes that the 'aprd_que' entry contains a clear confirm
   packet.  This channel will get a clear confirm when it has issued a clear
   request, so the appropriate action is to set the channel to 'cleared'.
   Stop any timer running to force confirm if a confirm was never received.
   ------------------------------------------------------------------------- */
void rcv_cnfpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern int end_time();

    if (ptr_2chcb -> call_state == CLEARCNF_PND) {
	ptr_2chcb -> call_state = CALL_CLEARED;
	ptr_2chcb -> clear_code = 0;
	int_off ();
	end_time (clr_cnftmr, ptr_2chcb);
	int_on ();
    }
    return;
}
