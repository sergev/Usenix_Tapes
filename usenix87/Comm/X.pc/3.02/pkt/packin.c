#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
extern  CCB_STR ccbarr[];
extern  QUE_ENTR quepool[];
extern  rep_time (), rnr_time (), rr_time ();/* receive window timeout */
/* -------------------------------------------------------------
    1/04/85  Mike  pack_synch routine, pack_in no RR/RNR/REJ by pack_synch
    9/17/86  Bobc  sseq-check, don't omit RR at low window bound now
---------------------------------------------------------------- */

/* ------------------------------------------------------------------------
   Pack_rdy is called when any packet is accepted, to give it to the applic
   or system/session manager (handles restarts, calls, resets, diagnostics)
------------------------------------------------------------------------- */
pack_rdy (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    extern void enque(),
		chanst_upd();
    BUF_PTR     buf, oldbuf;
    int         num;
 /*
  Application uses buf->bnext and num_bytes, not buf->data[B_BYTES],
  since it runs in both character mode and packet mode
  */
    if (lcb.driver_mode & PDTE_MODE) {
	/*
	 If we're in packet mode, flag the queue as having a buffer in it, then
	 check for the presence of CRC2.  If we have one, chain down the
	 bufferlets till we find the end of the packet, and knock off the last
	 two bytes, which will be CRC2.  The application doesn't need to see
	 them, and the packet's already been verified.
	 */
	que -> u.cmd.b_state |= B_QUED;
	buf = que -> u.cmd.bfirst;
	if (buf -> data[B_SIZE] != 0) {
	    while (buf -> bnext != NULL) {
		oldbuf = buf;
		buf = buf -> bnext;
	    }
	    if ((num = buf -> num_bytes) < 2) {
		buf -> num_bytes = 0;
		buf = oldbuf;
		buf -> num_bytes += num;
	    }
	    buf -> num_bytes -= 2;
	}
    }
    ccb -> qued_in += 1;
    int_off ();
    enque (&ccb -> read_que, que);
    int_on ();
    chanst_upd (ccb -> x_lci, UPD_RECV, TRUE);
    return;
}

/* ------------------------------------------------------------------------
   Pack_sync is called by Link_in, when CRC1 has been received.  The packet is
   in the link-layer's lcb.inq_entry, and its send/receive sequence numbers are
   checked.  P(R) is used to release send packets from the hold queue.  P(S) is
   checked and a zero is returned if OK.  Thus, acknowledged send packets can be
   released sooner, and incorrect sequence number will send a REJ or RESET
   sooner than waiting for CRC2 to be received, so that the input may be
   ignored.  The return code is +1 if input is complete or P(S) is bad.
--------------------------------------------------------------------------- */
pack_sync (lci, que)
byte lci;
QUE_PTR que;
{
    CCB_PTR ccb;
    int     i;
    if (ccb = lcb.ccbp[lci]) {
	if (lcb.driver_mode & PDTE_MODE) {
	    /*
	     If we're in packet mode, check for valid packet.  If we got one,
	     verify sequence number, and if it's got a valid one, and it's a
	     short packet, call pack_wreply() to do something with it.
	     */
	    if ((i = pack_check (que)) < ERR_TYP) {
		if (rseq_check (ccb, que) >= 0) {
		    if (i >= RR_TYP) {
			pack_wreply (ccb, que, i);
		    }
		}
	    }
	}
    }
    return;
}

/* ------------------------------------------------------------------------
   Pack_in is called by Link_in, when data has been received.  The packet
   is in the link-layer's lcb.in_que, and based on the logical channel ID,
   must be moved to the packet layer's ccb.read_que (if it is data/interrupt)
   and set the status bit to indicate that data is ready to read.
--------------------------------------------------------------------------- */
pack_in (lci, que)
byte lci;
QUE_PTR que;
{
    extern void rel_que();
    extern int  chanst_chk();
    BUF_PTR buf;
    CCB_PTR ccb;
    int     i,
	    rep,
	    seq,
	    error,
	    bad;
    error = 0;
    ccb = lcb.ccbp[lci];		/* allow null address, for auto-OPEN */
    if (lcb.driver_mode & PDTE_MODE) {
	i = pack_check (que);
	if (ccb == 0) {
	    /*
	     Channel is not currently open, check for call request or
	     Q-bit packet.  If got one, see if incoming calls pending.
	     If so, try to open the channel.  If we can, set it up.
	     */
	    if ((i == CALLR_TYP)
		    ||((i == DATA_TYP) && (que -> u.cmd.p_gfilci & Q_BIT))) {
		seq = (i == CALLR_TYP) ? VC_CHANNEL + IN_CALLS : PVC_CHANNEL;
		rep = chanst_chk (seq);
		if (rep >= 0)
		    if (lci == pack_open (lci, rep, seq)) {
			ccb = lcb.ccbp[lci];
			rset_conf (ccb, que);
		    }
	    }
	    if (ccb == 0) {
		diag_err (lcb.ccbp[0], &que -> u.cmd.p_gfilci, 36);
		error = IGNORE;
	    }
	}
	if (ccb != 0) {
	    /*
	     The channel is open.  Clear the error flag, set the packet
	     type.  If we got an RR, REJ, or RNR, ignore them.
	     */
	    ccb -> in_error = 0;
	    ccb -> in_pktindex = i;
	    if ((i >= RR_TYP) && (i <= REJ_TYP)) {
		ccb -> in_error = IGNORE;
	    }
	    else {
		/*
		 If erroneous packet type, have data state check it
		 out.
		 */
		if (i >= ERR_TYP)
		    ccb -> in_error = 33;
		if (crc2_chk (ccb, que) == FALSE) {
		    /*
		     A CRC error, set up to send a reject next time
		     around and ignore the packet.
		     */
		    ccb -> next_reply = REP_REJ;
		    ccb -> in_error = IGNORE;
		}
		else {
		    /*
		     The CRC's good, check the P(S).  If it's bad,
		     ignore the packet.
		     */
		    if (sseq_check (ccb, que) != 0) {
			ccb -> in_error = IGNORE;
		    }
		    else {
			/*
			 Sequence number good.  IF pack_sync hasn't
			 said anything bad, copy the packet ID in, and
			 check for packet size.  If too large, set
			 ccb->in_error to 39.
			 */
			if (ccb -> in_error == 0) {
				buf = que -> u.cmd.bfirst;
				buf -> data[B_BYTE0] = buf -> data[B_PKTID];
				if (buf -> data[B_SIZE] > ccb -> pkt_size)
				ccb -> in_error = 39;
				else {
				    /*
				     Packet size is OK, check for Clear
				     Confirm or less on logical channel 0.
				     If the packet's not a reset confirm,
				     complain.
				     */
				    if (i <= CLRC_TYP) {
					if (lci == 0) {
					    if ((i < RSETC_TYP) || (i > RSETC_TYP))
						ccb -> in_error = 36;
					}
				    }
				    else {
					/*
				 	 If not on channel 0, if we got a
				 	 diagnostic packet, ignore it,
				 	 otherwise complain.
					 */
					if (lci != 0) {
					    if (i == DIAG_TYP) {
						ccb -> in_error = IGNORE;
					    }
					    else {
						ccb -> in_error = 41;
					    }
				        }
				    }
				rst_state (ccb, que);
			    }
			}
		    }
		}
	    }
	    if ((i = ccb -> next_reply) != 0) {
		/*
		 If a reply is required, set up the sequence.
		 */
		seq = ccb -> vr;
		if (i & REP_RNR) {
		    int_off ();
		    if ((ccb -> time_run & TIM_RNR) == 0) {
			stop_time (rep_time, ccb -> x_lci, TIM_REP);
			stop_time (rr_time, ccb -> x_lci, TIM_RR);
			int_on ();
			int_off ();
			strt_time (lcb.htime, rnr_time, ccb -> x_lci, TIM_RNR);
		    }
		    int_on ();
		    rep = RNR_PKT;
		/*
		 NOTE: rnr_sent will be cleared internally, then
		 timeout sends RR; after input confirms delivery of
		 RR, recv_state will be cleared
		 */
		}
		else
		    rep = NULL;
		if (i > ccb -> recv_state) {
		    /*
		     If a reply is waiting, check the type.  If not an
		     RR, just set recv_state accordingly.  If a REJ or
		     RR, set the reply packet ID.
			*/
		    if (i != REP_RR)
			ccb -> recv_state |= i;
		    if (i & REP_REJ)
			rep = REJ_PKT;
		    else
			if (i & REP_RR)
				rep = RR_PKT;
		}
		if (rep)
		    rep_send (lci, rep, seq);
		ccb -> next_reply = 0;
	    }
	    error = ccb -> in_error;
	}
    }
    if (error != 0) {
	int_off ();
	rel_que (que, que -> u.cmd.bfirst);
	int_on ();
    }
    return;
}

/* ------------------------------------------------------------------------
   Rseq_check, will verify the input recv seq # P(R) with our send seq #
   and release send packets up to number P(R)-1, if any.
     if out_vsu < out_vs, then out_vsu <= P(S) < out_vs is good
     if out_vsu > out_vs, then out_vs <= P(S) < out_vsu is bad
   when good, call pack_wreply to release output buffers and return TRUE.
   when bad, return FALSE.
-------------------------------------------------------------------------- */
rseq_check (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    int     pr,                 /* our receive sequence P(R) */
	    pkt,                /* packet id */
	    low,                /* oldest send packet # */
	    high,               /* newest send packet # */
	    ans;                /* flag to see what to check */
    pr = que -> u.cmd.p_seqenc >> 4;
    ans = 0;
    pkt = que -> u.cmd.p_id & 0x1F;
    if (((que -> u.cmd.p_gfilci & C_BIT) == 0) || (pkt != RSETR_PKT)) {
	/*
	 Skip this code, if control-packet or restart/reset

	 Otherwise, check to see if p(r) is inside the window.  If p(r) is a
	 next send number, and there are packets outstanding, release some
	 packets.

	 If it isn't the next packet to send, and it's not equal to the oldest
	 packet outstanding, release some packets.
	 */
	low = ccb -> vsl;
	high = ccb -> vs;
	ans = between (low, pr, high);
	if (ans >= 0) { 
	    if (ans == 0) {
		if (low != high) {
		    rel_pktyp (ccb, pr);
		}
	    }
	    else {
		if (low != pr) {
		    rel_pktyp (ccb, pr);
		}
	    }
	    /*
	     check for end of send timeout.  If it happened, reset the flag,
	     and set ans to 1.
	     */
	    if (ccb -> send_state & SEN_TIME) {
		ccb -> send_state &= ~SEN_TIME;
		ans = 1;
	    }
	    if (ccb -> send_state & SEN_RTRN) {
		/*
		 If we are retransmitting, see if we've caught up to the
		 rejected packet.  If we have, say we aren't retransmitting any
		 more and set ans to 1.
		 */
		if (between (ccb -> vsl, ccb -> rej_recv & 0x0F, pr) >= 0) {
		    ccb -> send_state &= ~SEN_RTRN;
		    ccb -> rej_recv = 0;
		    ans = 1;
		}
	    }
	}
	else {
	    /*
	     P(R) is out of bounds, reset saying we have an invalid P(R) and
	     junk the packet.
	     */
	    pr = (lcb.driver_mode == PDTE_MODE) ? RES_DTEORG : RES_LOCPE;
	    rset_send (ccb -> x_lci, pr, 2);
	    ccb -> in_error = 2;
	}
    }
    return (ans);
}

/* ------------------------------------------------------------------------
   Sseq_check, or send sequence # check, will verify the send seq number P(S)
   except when reset or restart packets are received.
   In_ps is the next expected sequence number; in_psu = (in_ps + window - 1),
   the highest possible number for packet missed (due to CRC error).
   If P(S) = in_ps, then send seq number is good, so increment it;
   when good, call pack_acknow to check for state that allows RR
-------------------------------------------------------------------------- */
sseq_check (ccb, que)           /* do not check seq of reset and restart */
CCB_PTR ccb;
QUE_PTR que;
{
    int     ps,                 /* current receive seq # (P(S)) */
	    pkt,                /* packet ID, also flag to say whether to
				   reply */
	    ans;                /* return value saying whether packet is
				   OK*/
    ans = 0;
    pkt = que -> u.cmd.p_id & 0x1F;
    if (((que -> u.cmd.p_gfilci & C_BIT) == 0) || (pkt != RSETR_PKT)) {
	/*
	 Skip this code, if control-packet and restart/reset
	 */
	ps = que -> u.cmd.p_seqenc & 0x0F;
	if (ps == ccb -> vr) {
	    /*
	     The P(S) value is OK, increment the receive sequence number, set
	     to return 0, and don't send a reply.
	     */
	    ccb -> vr = (ccb -> vr + 1) & (MODULO - 1);
	    ans = 0;
	    pkt = 0;
	    /*
	     If we were rejecting, zero the reject packet number.  Either
	     way, clear the RR and REJ reply bits.
	     */
	    if (ccb -> recv_state & REP_REJ)
		ccb -> rej_sent = 0;
	    ccb -> recv_state &= ~(REP_REJ | REP_RR);
	    /*
	     If this is a data packet, increment the count of data packets
	     in.
	     */
	    if  ((que -> u.cmd.p_gfilci & C_BIT) == 0)
		(ccb -> data_in)++;
	    if (ps == ccb -> vrl) {
		/*
		 Packet window is (was) empty, cancel idle RR timer, and start
		 the reply timer again.
		 */
		int_off ();
	    /*  stop_time (rep_time,ccb->x_lci,TIM_REP);  * canceled by pk_start */
		stop_time (rr_time, ccb -> x_lci, TIM_RR);
		int_on ();
		int_off ();
		strt_time (lcb.htime, rep_time, ccb -> x_lci, TIM_REP);
		int_on ();
		pkt = 1;
	    }
	    else {
		/*
		 If packet window is full, say time to send a reply.  If data
		 packet, and data window is full, say time to send a reply.
		 */
		if (((ps + 1) & (MODULO - 1)) == ccb -> vrh)
		    pkt = 1;
		if ((que -> u.cmd.p_gfilci & C_BIT) == 0)
		    if (((ccb -> data_in) + ccb -> early_rep) >= ccb -> dat_window)
			pkt = 1;
		/*
		 If time (probably is) to send a reply, set up to send an RR.
		 */
		if (pkt)
		    ccb -> next_reply = REP_RR;
	    }
	}
	else {
	    /*
	     P(S) is not correct, don't do anything with the packet.
	     */
	    ans = -1;
	    if (((ps + 1) & (MODULO - 1)) == ccb -> vr) {
		/*
		 Duplicate P(S), send an RR next
		 */
		ccb -> next_reply = REP_RR;
	    }
	    else {
		/*
		 Not a duplicate P(S), if the value is in the window, reject
		 the packet. If it's out of the window, send a reset.
		 */
		if ((ans = between (ccb -> vrl, ps, ccb -> vrh)) >= 0) {
		    ccb -> next_reply = REP_REJ;
		}
		else {
		    ps = (lcb.driver_mode == PDTE_MODE) ? RES_DTEORG : RES_LOCPE;
		    rset_send (ccb -> x_lci, ps, 1);
		}
	    }
	}
    }
    return (ans);
}

/* ------------------------------------------------------------------------
   Between will verify the receive sequence number (to see if any packets
   sent can be released) or will verify the send sequence number (to see if
   the packet received is the number expected, or an old one retransmitted),
   except when Reset or Restart packets are received.                    return
   When in_psu = in_ps = P(S), then packet is ok (no outstanding packets), 0
     if in_psu < in_ps, then in_psu <= P(S) < in_ps is ok (acks packets), +n
     if in_psu > in_ps, then in_ps <= P(S) < in_psu is bad                -n
-------------------------------------------------------------------------- */
between (low, mid, high)
int     low,
	mid,
	high;                   /* input values */
{
    int     answer;             /* returned value */
/*
 Adjust for wrap around, MODULO 16 (or other)
 */
    if (high < low) {
	if (mid < low)
	    mid += MODULO;
	high += MODULO; 
    }
/*
 Check for inside of window
 */
    if (low <= mid && mid <= high)
	answer = high - mid;
    else
	answer = -1;
    return (answer);
}
