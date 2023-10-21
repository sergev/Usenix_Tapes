#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
extern  rstr_time (), call_time (), clr_time (), intr_time (), rset_time ();
int     rr_time (), send_time ();/* internal timeout routine */

/* Corrections:
    3/08/85  Mike  make rnr_time timeout run with interrupts on
    9/08/86  BobC  for NSR#528 related, REJ need not hold back more APP
		   data.
   10/21/86  Ed M  change rr_time to report packet level lost after
		   128 seconds instead of 32.  Prevents the idiocy of having
		   the driver send an RR on channel 0 at the same time as
		   it reports the packet level lost. 
--------------------------------------------------------------------------- */
int     pkt_recvd;		/* set: pbyte_in (good CRC1);
				   reset/test in rr_time */

/* ----------------------------------------------------------------------
   send_time is called by timeout when the DXE does not respond within
   T1 seconds.  Retry N1 times, then reset.
	 (but how do we stop retry from being set again?).
    ans: set retry in pack_write, it is not called to resend.
	 resend involves remove from end of hold_que, and insert to out_que
--------------------------------------------------------------------------- */
send_time (lci)
int	lci;
{
    extern void    link_write(),
		   rel_que();
    extern QUE_PTR last_que();
    CCB_PTR        ccb;
    QUE_PTR        que;
    int            cause,
		   code;

    if (ccb = lcb.ccbp[lci]) {
	ccb -> time_run &= ~TIM_SEND;
	int_off ();
	if ((que = last_que (&ccb -> hold_que)) != NULL) {
	    /*
	     If we have something to retransmit, check to see if the retry
	     count has expired.
	     */
	    int_on ();
	    if (--ccb -> retry == 0) {
		/*
		 Retry count has expired.  If we have a control packet in the
		 queue, let the control packet retry logic take over.
		 */
		ccb -> send_state &= ~SEN_TIME;
		if (que -> u.cmd.p_gfilci & C_BIT)
		    code = NULL;
		else {
		    /*
		     Not a control packet, get rid of the queue.  Set cause to
		     0 for DTE, RES_LOCPE (local procedure error) for DCE, and
		     code to 146 for the timeout, and send a reset with the
		     cause and code we just got.
		     */
		    int_off ();
		    rel_que (que, YES_BUFF);
		    int_on ();
		    if (lcb.driver_mode & PDTE_MODE)
			cause = 0;
		    else
			cause = RES_LOCPE;
		    code = 146; 
		    rset_send (lci, cause, code);
		}
	    }
	    else {
		/*
		 Retry has not expired, set the timeout again, flag it as a
		 retransmission, and tell link_write() to send it.
		 */
		ccb -> send_state |= SEN_TIME;
		que -> u.cmd.b_state |= B_RETRAN;
		link_write (LWRIT_OUT, que);
		int_off ();
		strt_time (lcb.htime << 1, send_time, lci, TIM_SEND);
	    }
	}
	int_on ();
    }
    return;
}

/* ----------------------------------------------------------------------
   try_send is called by rel_pktyp (end of retrans due to time/REJ) and
   pack_wreply (end of RNR state) to restart sending of output.
   interrupts are disabled.
--------------------------------------------------------------------------- */
try_send (ccb)
CCB_PTR ccb;
{
    extern void    link_write();
    extern QUE_PTR deque();

    int_off ();
    /*
     If we have some output pending, and there is nothing for us currently
     awaiting output, check to see if it's OK to send something.  If so,
     haul an entry off the write queue and give it to link_write().
     */
    if (ccb->writ_que.ent_cnt) {
	if (ccb->writ_que.ent_cnt + ccb->hold_que.ent_cnt == ccb->qall_out) {
	    if (send_ok (ccb, ccb->writ_que.first, ccb->hold_que.ent_cnt)) {
		link_write (LWRIT_OUT, deque (&ccb->writ_que));
	    }
	}
    }
    int_on ();
    return;
}

/* ----------------------------------------------------------------------
   Rep_send is called by rr_time to send an RR packet, periodically
--------------------------------------------------------------------------- */
rep_send (lci, pktid, seq)
int     lci,
	pktid,
	seq;
{
    extern void    link_write();
    extern QUE_PTR get_que();
    QUE_PTR        tque;

    int_off ();
    /*
     Assuming we manage to get a queue, set it up for a packet with the
     appropriate packet id and sequence.  Since it's a reply packet, we set
     the control bit, and a length of 0.  Then we tell link_write() to send
     it as a reply packet.
     */
    if (tque = get_que (NO_BUFF)) {
	tque -> u.cmd.b_state = 0;
	tque -> u.cmd.p_gfilci = C_BIT + lci;
	tque -> u.cmd.p_seqenc = seq << 4;
	tque -> u.cmd.p_id = pktid;
	link_write (LWRIT_REP, tque);
    }
    int_on ();
    return;
}

/* ------------------------------------------------------------------------
   conf_send is called by clrr_err when an input error is detected.
   will send to the local DXE a Clear Packet, with the given cause and code;
------------------------------------------------------------------------ */
conf_send (lci, pktid)
byte lci, pktid;
{
    extern QUE_PTR get_que();
    QUE_PTR        que;
    BUF_PTR        buf;

    int_off ();
    que = get_que (YES_BUFF);
    int_on ();
    if (que) {
	que -> u.cmd.p_gfilci = XPC_WRITE;
	que -> u.cmd.p_seqenc = 0;
	que -> u.cmd.p_id = pktid;
	buf = que -> u.cmd.bfirst;
	buf -> num_bytes = B_BYTE0 + 1;
	pack_write (lci, que, 1);
    }
    return;
}

/* ---------------------------------------------------------------------------
   send_ok is called by pack_write, pack_start and try_send to grant permission
   to write which it determined by looking at the send_state and packet type.
--------------------------------------------------------------------------- */
send_ok (ccb, que, count)
CCB_PTR ccb;                    /* channel control block address */
QUE_PTR que;                    /* queue entry points to data in bufferlets */
int     count;                  /* pack_write's # queued, or try_send's
				   hold queue entries */
{
    int     granted;
 /*
  pack_write count is data packets qued_out check data/control packets
  or is control packets qall_out try_send count is hold_queue.entry_count
  */
  /*
   Assume that we can send a packet.  Say that we can't if:

   We are retransmitting.
   We want to send a data packet, and we have filled the data window, or we
   aren't in data transfer state, or we got an RNR packet on that channel.
   We have filled the packet window.
   We have sent an Interrupt request, and we're trying to send another
   one before the first one's been confirmed.
   */
    granted = TRUE;
    if (ccb -> send_state & (SEN_TIME + SEN_RTRN * RTR_HOLD))
	granted = FALSE;
    else {
	if ((que -> u.cmd.p_gfilci & C_BIT) == 0) {
	    if (count >= ccb -> dat_window || ccb -> d_state != D1
					   || ccb -> rnr_recv != 0)
		granted = FALSE;
	}
	else {
	    if (count >= ccb -> pkt_window)
		granted = FALSE;
	    else
		if (ccb -> send_state & SEN_INTR) {
		    if ((que -> u.cmd.p_gfilci & C_BIT)
			    && (que -> u.cmd.p_id == INT_PKT))
			granted = FALSE;
		}
	}
    }
    return (granted);
}

/* ----------------------------------------------------------------------
   rnr_time is receive not ready timeout (RNR sent) called by time-out,
   started by discovery of RNR condition, to check for end of RNR state.
   At 1200 baud, this will be checked every 4 seconds.
--------------------------------------------------------------------------- */
rnr_time (lci)
int     lci;
{
    CCB_PTR ccb;
    if (ccb = lcb.ccbp[lci]) {
	ccb -> time_run &= ~TIM_RNR;
	int_off ();
	if (rnr_still (ccb, ccb -> recv_state))
	    strt_time (lcb.htime, rnr_time, lci, TIM_RNR);
	else {
	    strt_time (lcb.htime << 2, rr_time, lci, TIM_RR);
	    rep_send (lci, RR_PKT, ccb -> vr);
	}
	int_on ();
    }
    return;
}

/* ----------------------------------------------------------------------
   Rr_time is receive "idle" timeout, called by time-out to send an RR packet.
   Started by nothing received on a channel.  Canceled by sequenced input,
   which starts rep_time, or by receive "window" timeout.  32 SEC at 1200 baud
--------------------------------------------------------------------------- */
rr_time (lci)
int     lci;
{
    extern int  pkt_recvd;	/* set: pbyte_in (good CRC1);
				   reset/test in rr_time */
    extern  void chanst_upd();
    CCB_PTR ccb;
    static  QUE_PTR oldque;
    static  BUF_PTR oldbuf;
    static  four_times = -1;	/* chan 0: after 4 8-sec timeouts, send RR */
    int     state,
	    rep = 0;		/* no reply on non-zero channels */
    if (ccb = lcb.ccbp[lci]) {
	ccb -> time_run &= ~TIM_RR;
	if (lci == 0) {
	    /*
	    Every 8 seconds, see if input is working; if not, try to recover
	    */
	    if ((oldque != lcb.inq_entry) || (oldbuf != lcb.inb_ptr)) {
		/*
		 Check saved copies of queue and buffer against current.  If
		 they've changed, save them away.
		 */
		oldque = lcb.inq_entry;
		oldbuf = lcb.inb_ptr;
	    }
	    else {
		/*
		 Buffers didn't change, try re-enabling input.
		 */
		int_off ();
		re_enable ();
		int_on ();
	    }
	    four_times += 1;
	    if ((four_times & 3) == 0) {
		/*
		 Every 32 seconds, see if we should send an RR on channel 0.
		 If have not received any packets for 128 seconds, assume that
		 the other end has died, and set the channel status
		 accordingly.
		 */
		rep = RR_PKT;
		if ((pkt_recvd == 0) && ((four_times & 15) == 0))
		    chanst_upd (lci, UPD_PKTLOST, TRUE);
	    }
	}
    /* 
     see if exception condition has ended
     */
	if ((state = ccb -> recv_state) != 0) {
	    if (state & REP_RNR) {/* !!!! SHOULD NEVER GET HERE !!!! */
		if (rnr_still (ccb, state))
		    rep = -RNR_PKT;
		else
		    rep = RR_PKT;
	    }
	    else
		if (state & REP_REJ)
		    rep = REJ_PKT;
		else
		    if (state & REP_RR)
			rep = RR_PKT;
	}
	/*
	 Send RR or other reply.  If not on channel 0 clear errors, check
	 the transmit and receive flags.  If on channel 0, clear
	 pkt_recvd.
	 */
	rep_check (ccb, rep);   
	if (lci)
	    err_fix (); 
	else
	    pkt_recvd = 0;
    }
    return;
}

/* ----------------------------------------------------------------------
   Rep_time is receive "window" timeout, called by time-out to send an RR
   packet, after input.  Started by 1st packet received in a window or by
   rep_time, when input is still active.                    8 SEC at 1200 baud
--------------------------------------------------------------------------- */
rep_time (lci)
int     lci;
{
    CCB_PTR ccb;
    int     rep;
    if (ccb = lcb.ccbp[lci]) {
	ccb -> time_run &= ~TIM_REP;
	/*
	 If we are on channel 0, or an active circuit, check for active
	 input, and no RNR, RR, REJ, or data sent, and no REJ, RNR or RR
	 pending.  If above is true, send an RR.
	 */
	if ((lci == 0) || (ccb -> circ_state & CIR_ACTV)) {
	    if ((ccb -> vrl != ccb -> vr) && (ccb -> recv_state == 0)
		    &&(ccb -> next_reply == 0))
		rep = RR_PKT;
	    else
		rep = 0;
	    rep_check (ccb, rep);
	}
    }
    return;
}

/* ----------------------------------------------------------------------
   Rep_check is called by both (window and idle) receive timeouts, to send
   send the appropriate reply.  If reply state is non-zero, re-send the
   appropriate reply, unless RNR condition is cleared.  If reply state is
   zero send RR, unless RR condition has been set.
--------------------------------------------------------------------------- */
rep_check (ccb, rep)
CCB_PTR ccb;
int     rep;
{
    int     interval;           /* 32 sec on chan 0,
				   lcb.htime * 8 on chans > x */
 /* 
  see if exception condition has begun
  */
    int_off ();
    if (ccb -> recv_state == 0) {
	if ((lcb.xpc_nrdy != 0)
		&& (ccb -> x_lci != 0)
		&& (ccb -> circ_state & CIR_ACTV)) {
	    ccb -> rnr_sent |= RNR_XPC + ccb -> vr;
	    ccb -> recv_state |= REP_RNR;
	    rep = RNR_PKT;
	}
    }
    int_on ();
    int_off ();
    /*
     If RNR start or continue (-RNR_PKT), set 4 second timeout.
     */
    if ((rep == RNR_PKT) || (rep == -RNR_PKT)) {
	strt_time (lcb.htime, rnr_time, ccb -> x_lci, TIM_RNR);
	if (rep < 0)
	    rep = NULL;
    }
    else {
	/*
	 If there's no active input, set timeout to send a new RR.  Timeout
	 value is 8 seconds for channel 0, 16 seconds for the others.

	 If there's active input, set a 4 second timer to check for reply
	 packets.
	 */
	if (ccb -> vrl == ccb -> vr) {
	    interval = (ccb -> x_lci ? (lcb.htime << 2) : EVNT_PSEC * 8);
	    strt_time (interval, rr_time, ccb -> x_lci, TIM_RR);
	}
	else
	    strt_time (lcb.htime, rep_time, ccb -> x_lci, TIM_REP);
    }
    int_on ();
    /*
     Send a reply packet if we need to.
     */
    if (rep)
	rep_send (ccb -> x_lci, rep, ccb -> vr);
    return;
}

/* ------------------------------------------------------------------------
   rnr_still is called by rep_check and rnr_time to see if RNR state may end.
   returns TRUE if recv_state is still RNR, FALSE if recv_state is now normal.
------------------------------------------------------------------------- */
rnr_still (ccb, state)
CCB_PTR ccb;
int     state;
{
    int     sent;
    if ((sent = ccb -> rnr_sent) != 0) {
	if (sent & RNR_XPC) {
	    if (lcb.xpc_nrdy == 0)
		sent &= ~RNR_XPC;
	}
	if (sent & RNR_WIN) {
	    if (ccb -> qued_in < ccb -> dat_window)
		sent &= ~RNR_WIN;
	}
	if ((sent & 0xF0) == 0) {
	    ccb -> rnr_sent = 0;
	    ccb -> recv_state = (state & ~REP_RNR) | REP_RR;
	}
    }
    return (sent);
}

/* -------------------------------------------------------------------------
   strt_time is called by everybody when a timeout needs to be set; but it
   only calls set_time when the timeout is NOT running.  Interrupts are off.

	*** should check for error return from set_time(); ***
   ------------------------------------------------------------------------- */
strt_time (delta, subrt, lci, flag)
fncptr subrt;
int     delta,
	lci,
	flag;
{
    extern int  set_time();

    if (lcb.ccbp[lci] && !(lcb.ccbp[lci] -> time_run & flag)) {
	set_time (delta, subrt, lci);
	lcb.ccbp[lci] -> time_run |= flag;
    }
    return;
}

/* -------------------------------------------------------------------------
   stop_time is called by everybody when a timeout needs to be end; but it
   only calls end_time when the timeout is running.  Interrupts are off.

	*** should check for error return of end_time() ***
   ------------------------------------------------------------------------- */
stop_time (subrt, lci, flag)
fncptr subrt;
int     lci,
	flag;
{
    extern int end_time();

    if (lcb.ccbp[lci] && (lcb.ccbp[lci] -> time_run & flag)) {
	end_time (subrt, lci);
	lcb.ccbp[lci] -> time_run &= ~flag;
    }
    return;
}
