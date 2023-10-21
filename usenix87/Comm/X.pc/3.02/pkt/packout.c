#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
extern  rstr_time (), call_time (), clr_time (), rep_time (),
	intr_time (), rset_time ();
int     send_time ();           /* internal timeout routine */

/* Corrections:
   12/11/85  M.A.  pack_wreply fix REJ problem, if pkt only in out_que
   10/15/86  BobC  Adjust retry count value (R25 & R27 at least,) from N2.

   Initially, the transmitter is in the disabled state and xmit_on is FALSE.
   Pack_open will call link_open to enable input interrupt, if channel = 0.
   Pack_close will call link_close to disable interrupts, if channel = 0.

   Pack_write is called by Application Server to send.  It calls link_write,
   which is also called by packet level to resend (after REJ input), and by
   packet level to reply (after data input).  Pack_start put the packet's queue
   entry into the hold queue, where it stays while it is also in the LCB's
   outq_entry and outb_ptr (the duplicate is thrown away, but NOT RELEASED) */

/* ---------------------------------------------------------------------------
   purge_rr is called by strt_ouframe, when it is sending a packet from the
   reply queue (rep_que).  If the reply may be purged, return TRUE (non-zero).
   The packet that might be purged is still in the rep_que.
--------------------------------------------------------------------------- */
purge_rr ()
{
    int     tempi,              /* Used to store channel number */
	    ans;                /* Flag to say whether it's ok to purge */
    /*
     Assume that it's not OK to purge.  (The code will set it to 1 if it
     turns out to be OK), and get the channel number.
     */
    ans = 0;
    tempi = lcb.outq_entry -> u.cmd.p_gfilci & P_LCI;
    if (tempi == 0) {
	/*
	 For channel 0, if there is more than one packet in the reply queue,
	 and there is something in the out queue, say it's OK to purge.
	 */
	if ((lcb.rep_que.ent_cnt > 1) || (lcb.out_que.ent_cnt != 0))
	    ans = 1;
    }
    else
	if (lcb.ccbp[tempi]) {
	    /*
	     For all other channels, if the channel is active, and it's receive
	     state indicates that an RR has been sent, the only packet in the
	     reply queue is an RR for our channel, and there is something
	     pending in the out queue, then , and only then, is it OK to purge.
	     */
	    if (((lcb.ccbp[tempi] -> recv_state & REP_RR) == 0)
		    &&(lcb.rep_que.ent_cnt == 1)
		    &&(lcb.rep_que.first -> u.cmd.p_id == RR_PKT)
		    &&(lcb.out_que.ent_cnt != 0)
		    &&((lcb.out_que.first -> u.cmd.p_gfilci & P_LCI) == tempi))
		ans = 1;
	}
    return (ans);
}

/* ---------------------------------------------------------------------------
   Pack_start is called by Link layer's frame_write to:
   if char mode, return the queue entry & buffer to avail queue, using rel_que.
   if packet mode
   o sequenced data or control packet
     (move queue entry to hold_que, is now in PACK_COMPL when output completes)
     - set timeout and retry, and
     - store P(R) and P(S), and increment P(S) after using it
   o supervisory control packet (no buffer status or pointer)
     - use P(R) in the queue entry (leave it unchanged)
   Can P(R) get out of sequence?  RR got its sequenced number before!

   Receive timeout is 2 sec * window at 1200 baud, or 8 sec
   Send timeout is 2 sec * (window+1) at 1200 baud, or 10 sec
--------------------------------------------------------------------------- */
pack_start (lci, que)
QUE_PTR que;                    /* has pointer to data record in bufferlets */
int     lci;                    /* size of data record */
{

    extern void enque();
    CCB_PTR     ccb;            /* channel control block address */
    int         type,
		ndx,
		x;
    fncptr      timeout;
    static int
    /*
	 1  2   3  4   5  6   7  8   9   a   b  c   d   e   f   0   1   2
    */
		ndx_index[] = {
	-1, 6, -1, 2, -1, 1, -1, 3, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1
    }	       ,
		try_ndx[] = {
	 5, 6, -1, 2, -1, 1, -1, 3, -1, -1, -1, 0, -1, -1, -1, -1,  7, -1
    }	       ,
		tim[] = {
	T20, T21, T22, T23, T24, T25, T26, T27, T28
    };
    static char
		try[] = {
	R20, R21, R22, R23, R24, R25, R26, R27, R28
    };
 /*
 requests: 0=restart, 1=call, 2=reset, 3=clear, 4=recv_window? 5=data,
	   6=interrupt, 7=reject, 8=registration
  */
    if (ccb = lcb.ccbp[lci]) {
	if (lcb.driver_mode & PDTE_MODE) {
	    /*
	     If in packet mode, and we have a packet with P(S), that has not
	     yet been sent.
	     */
	    if (que -> u.cmd.b_state != 0) {
		if ((que -> u.cmd.b_state & B_RETRAN) == 0) {
		    /*
		      Fetch the packet code (1-18):
		      1=data    4=rset rq 7=call cn 10=reg req 13=rstr cn 16=RNR
		      2=inter   5=rset cn 8=clr req 11=reg con 14=diagnos 17=REJ
		      3=int con 6=call rq 9=clr con 12=rstr rq 15=RR      18=error

		      Use it as an index into ndx_index[] to get the timeout
		      function and bit to test.
		      */
		    type = pack_check (que);
		    if ((ndx = ndx_index[type - 1]) >= 0) {
			switch (ndx) {
			    case 0: /* restart */
				timeout = rstr_time;
				x = TIM_RSTR;
				break;
			    case 1: /* call */
				timeout = call_time;
				x = TIM_CALL;
				break;
			    case 2: /* reset */
				timeout = rset_time;
				x = TIM_RSET;
				break;
			    case 3: /* clear */
				timeout = clr_time;
				x = TIM_CLR;
				break;
			    case 6: /* interrupt */
				timeout = intr_time;
				x = TIM_INTR;
				ccb -> send_state |= SEN_INTR;
			}
			/*
			 If packet has not yet been sent, use the same
			 index to set up the retry count, and the retry
			 timer.
			 */
			if ((que -> u.cmd.b_state & B_STRTD) == 0)
			    ccb -> retry20[ndx] = try[ndx] + 1;
			int_off ();
			strt_time (tim[ndx] * EVNT_PSEC, timeout, lci, x);
			int_on ();
		    }
		    if (ccb -> vsl == ccb -> vs) {
			/*
			 If starting a new send window, stop the old send
			 timeout, and start a new one.  Must be longer than the
			 receive timeout.
			 */
			int_off ();
			stop_time (send_time, lci, TIM_SEND);
			int_on ();
			int_off ();
			strt_time (lcb.htime << 1, send_time, lci, TIM_SEND);
			int_on ();
		    }
		    /*
		     Select max number of retransmissions.  Use ccb-retry.
		     Stash current p(s) in x, and increment p(s), with wrap.
		     */
		    if ((ccb->retry = try [try_ndx [type - 1]] + 1) <= 1)
			ccb -> retry = N2 + 1;
		    x = ccb -> vs;
		    ccb -> vs = (ccb -> vs + 1) & (MODULO - 1);
		}
		else {
		    /*
		     On a retransmission, stash the old p(s) in x.
		     */
		    x = que -> u.cmd.p_seqenc & 0x0F;
		}
		/*
		 Put the new p(r) into the high 4 bits of x, store it into
		 the queue and the buffer, flag the packet as I/O started.
		 */
		x = (ccb -> vr << 4) + x;
		que -> u.cmd.p_seqenc = x;
		que -> u.cmd.bfirst -> data[B_PRPS] = x;
		que -> u.cmd.b_state |= B_STRTD;
		int_off ();
		/*
		 Stuff it into the hold queue, in case we have to
		 retransmit it.
		 */
		enque (&ccb -> hold_que, que);
		int_on ();
	    }
	    /* 
	     Rotate the receive packet window, and zero the count of data
	     packets outstanding.
	     */
	    int_off ();
	/* stop_time (rep_time,ccb->x_lci,TIM_REP);   * cancel recv timeout */
	    ccb -> vrl = ccb -> vr;
	    ccb -> vrh = (ccb -> vrl + ccb -> pkt_window - 1) & (MODULO - 1);
	    ccb -> data_in = 0; 
	    int_on ();
	}
    }
    return;
}

/* ----------------------------------------------------------------------
   pack_compl is called by frame_write when output is completed, to release
   packets that receive no reply.
--------------------------------------------------------------------------- */
pack_compl (que)
QUE_PTR que;
{
    extern void rel_que();
    CCB_PTR     ccb;
    int         lci,
		bad;

    lci = que -> u.cmd.p_gfilci & P_LCI;/* get logical channel ID */
    if (ccb = lcb.ccbp[lci]) {
	if (lcb.driver_mode & PDTE_MODE) {
	    if (que -> u.cmd.b_state == 0) {
		/*
		 If current packet is a reply packet, release it witout
		 releasing the lcb's buffer.
		 */
		int_off ();
		rel_que (que, NO_BUFF);
		int_on ();
	    }
	    else {
		/*
		 Current packet is a sequenced packet, flag state as output
		 completed.
		 */
		que -> u.cmd.b_state |= B_CMPLTD;
	    /*
	     * zero seq # after restart/reset *
	     if (que->u.cmd.p_id == RSTRR_PKT)     
		 rstr_done(ccb,NULL);
	     else if (que->u.cmd.p_id == RSETR_PKT)
		 rset_done(ccb,NULL);
	     else if (que->u.cmd.p_id ==  CLRR_PKT)
		 clr_done(ccb,NULL);
	     NOTE: que entry is not placed anywhere at this time; pack_start
	     put it into hold_que before packet was sent
	     */
	    }
	    try_send (ccb);
	}
/*  else {                               * character mode *
i     int_off ();
n     rel_que (que,YES_BUFF);                    * release the char buffer *
      int_on ();
P     if (lcb.xpc_nrdy == 0                      * if avail entries are OK *
A     &&  ccb->qued_out < ccb->dat_window)       * and window is not full, *
D       lci = TRUE;                              * start next packet *
      else
c       lci = FALSE;                             * stop next packet *
o     chanst_upd (ccb->x_lci,UPD_XMIT,lci);
d     ccb->qued_out -= 1;                        * decrement queued count *
e     ccb->qall_out -= 1;                        * decrement queued count *
    }    * else char mode *                                               */
    }
    return;
}

/* ------------------------------------------------------------------------
   Rel_pktyp is called by pack_in's rseq_check to release packet(s) when the
   receive sequence number, P(R), indicates there are packets waiting for RR.
   Rel_pktyp will release write packets after checking their type.  Thus,
   it can change the state after a restart (e.g.) confirm has been sent.
   Rel_pktyp must also restart the send timeout, if there are any outstanding
   packets, or cancel it if all send packets are acknowledged.
-------------------------------------------------------------------------- */
rel_pktyp (ccb, pr)
CCB_PTR ccb;
int     pr;
{
    extern QUE_PTR      deque();

    QUE_PTR             qsent;

    int                 keep,
			temp,
			holdps,
			more;
 /* 
  Release any buffers being held
  */
    more = TRUE;
    while ((more > 0) && (ccb -> hold_que.ent_cnt != 0)) {
	/*
	 Check hold queue p(s).  If it's between lowest p(s) sent and awaiting
	 acknowledgement, and the current p(r), set keep to FALSE, and process
	 the packet some more.
	 */
	holdps = ccb -> hold_que.first -> u.cmd.p_seqenc & 0x0F;
	more = between (ccb -> vsl, holdps, pr);
	if (more > 0) {
	    keep = FALSE;
	    /*
	     But request packet does not release, until confirmed (later) and
	     confirm packet changes state, when acknowledged (now)
	     */
	    if (ccb -> hold_que.first -> u.cmd.p_gfilci & C_BIT) {
		temp = ccb -> hold_que.first -> u.cmd.p_id;
		if (temp < 128) {
		    /*
		     If packet id is in positive set, check further on the id.
		     If it's a Reset, Call, or Clear Request, reset keep to
		     TRUE.  Process Reset confirms and clear confirms.
		     */
		    if (temp < CALLR_PKT) {
			if (temp == INTC_PKT)
			    ccb -> int_recv = 0;
			else
			    if (temp == RSETR_PKT)
				keep = TRUE;
			    else
				if (temp == RSETC_PKT)
				    rset_conf (ccb, NULL);
		    }
		    else {
			if (temp == CALLR_PKT)
			    keep = TRUE;
			else
			    if (temp == CLRR_PKT)
				keep = TRUE;
			    else
				if (temp == CLRC_PKT)
				    clr_conf (ccb, NULL);
		    }
		}
		else {
		    /*
		     Packet ID greater than 128.  If it's a Restart Request,
		     set keep to TRUE, if its a restart confirm process it.
		     */
		/* if (  temp == REGC_PKT)  registration may be added later */
		    if (temp == RSTRR_PKT)
			keep = TRUE;
		    else
			if (temp == RSTRC_PKT)
			    rstr_conf (ccb, NULL);
		}
	    }
	}
	else {
	    /*
	     If hold p(s) was not between keep the packet.
	     */
	    keep = TRUE;
	}
    /*
     NOTE: a request packet stops releasing of all packets 

     If packet was a keeper, stop processing the hold queue.
     */
	if (keep)
	    more = FALSE;
	else {
	    /*
	     Packet is not a keeper.  Take it off the hold queue and release
	     it's queue entry.  Update lowest send number to holdps, then bump
	     it with wrap.
	     */
	    int_off ();
	    qsent = deque (&ccb -> hold_que);
	    int_on ();
	    mak_avail (ccb, qsent);
	    ccb -> vsl = holdps;
	    ccb -> vsl = (ccb -> vsl + 1) & (MODULO - 1);
	}
    }
 /* 
  Rotate the send packet window, IF WE ARE SENDING
  */
    int_off ();
    if (ccb -> time_run & TIM_SEND) {
	/*
	 End the send timeout.  If there are any entries in the hold queue,
	 restart it for the full 8-second value.
	 */
	stop_time (send_time, ccb -> x_lci, TIM_SEND);
	int_on ();
	int_off ();
	if (ccb -> hold_que.ent_cnt != 0)
	    strt_time (lcb.htime << 1, send_time, ccb -> x_lci, TIM_SEND);
    }
 /* ccb->vsh = (ccb->vsl + (ccb->pkt_window-1)) & (MODULO - 1); */
    int_on ();
    return;
}

/* ------------------------------------------------------------------------
   rel_reqst is called by pack-in-restart, -call, or -data to release a packet
   after it has been the request has been confirmed.  Note that rel_pktyp must
   not release the request packets when they are acknowledged, since a confirm
   is required.  Required: ONLY ONE CONTROL PACKET MAY BE ACTIVE AT A TIME!
-------------------------------------------------------------------------- */
rel_reqst (ccb, pktid)
CCB_PTR ccb;
int     pktid;
{
    extern QUE_PTR fid_cmd();
    QUE_PTR        qsent;
 /*
  If we find a matching packet ID and channel number in the hold queue,
  release the packet.
  */
    int_off ();
    qsent = fid_cmd (&ccb -> hold_que, ccb -> x_lci, pktid);
    int_on ();
    if (qsent)
	mak_avail (ccb, qsent);
    return;
}

/* ------------------------------------------------------------------------
   mak_avail is called by rel_pktyp and rel_reqst to release a packet after
   it has been acknowledged; acknowledgement for X.PC request packets is in
   the form of a confirm packet.  Updates status and application send flag.
-------------------------------------------------------------------------- */
mak_avail (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    extern void rel_que(),
		chanst_upd();
    int         flag;

    /*
     Set the queues state to returned, and decrement the count of queues in
     use.
     */
    que -> u.cmd.b_state |= B_RETRND;
    ccb -> qall_out -= 1;
    if (que -> u.cmd.b_state & B_APPLIC) {
	/*
	 If the packet was originated by the application, decrement the count
	 of packets in queues.
	 */
	ccb -> qued_out -= 1;
	/*
	 If the channel is in data transfer state, has not received an RNR, the
	 available entries are OK, and the window is not full, update the
	 channel statues to say it's OK to send the next packet, otherwise say
	 it's not OK.
	 */
	if (ccb -> d_state == D1 && ccb -> rnr_recv == 0 && lcb.xpc_nrdy == 0
		&& ccb -> qued_out < ccb -> dat_window)
	    flag = TRUE;
	else
	    flag = FALSE;
	chanst_upd (ccb -> x_lci, UPD_XMIT, flag);
    }
    int_off ();
    rel_que (que, YES_BUFF);
    int_on ();
    return;
}

/* ----------------------------------------------------------------------
   Pack_wreply is called by Packet_in when a reply is received to written data.
   For any input, with sequence n, delete from the hold queue up to seq n-1.
   This has been done in packin, which processes all packets.
     if the input is RR, clear the other end not ready flag f_state.
     if the input is RNR, set the other end not ready flag, f_state.
     if the input is REJ, insert all hold queue packets in front of output que
       is nothing is in outque, the packets are already in output queue.
--------------------------------------------------------------------------- */
pack_wreply (ccb, que, type)
CCB_PTR ccb;                    /* channel control block address */
QUE_PTR que;                    /* queue entry of received supervisory packet */
int     type;                   /* packet type 0-15 from packin */
{
    extern void    enque(),
		   ins_list(),
		   chanst_upd();
    extern QUE_PTR fid_cmd();
    QUE_PTR        wptr;        /* hold queue pointer */
    int            seq;         /* p(r) from receive packet */

    seq = que -> u.cmd.p_seqenc >> 4;
    int_off ();
    switch (type) {
	case RR_TYP:
	    /*
	     On RR packet, if in Send Hold state, clear it, and clear rnr
	     received bit.

	     If the data window is not full, and the channel is in data
	     transfer state, update status to allow transmit.
	     */
	    if (ccb -> send_state & SEN_HOLD) {
		ccb -> send_state &= ~SEN_HOLD;
		ccb -> rnr_recv = NULL;
		if (ccb -> qued_out < ccb -> dat_window && ccb -> d_state == D1)
		    chanst_upd (ccb -> x_lci, UPD_XMIT, TRUE);
	    }
	    break;
	    /*
	    All packets in hold queue have been sent, the REJ received released
	    all packets with a lower sequence number, so all of the hold_que
	    packets may be resent, or inserted at the front of ccb.writ_que;
	    but first, if the link level has a packet for this chan, get it back.
	    Prevent interrupts so that nothing is moved.
	    */
	case REJ_TYP: 
	    /*
	     We got a reject.

	     If there is something on the out queue for our channel, fetch it
	     and stick it on the hold queue.

	     If there's anything in the hold queue, set the retransmission
	     flag, get a pointer to the first entry of the hold queue, and
	     stuff the entire hold queue into the out queue for the channel,
	     putting it in ahead of everything else.  Clear out the hold queue,
	     set the current p(s) to the p(r) from the received packet, and set
	     the reject received field to show the retransmitted p(s).
	     */
	    if (ccb -> writ_que.ent_cnt + ccb -> hold_que.ent_cnt != ccb -> qall_out) {
		if (wptr = fid_cmd (&lcb.out_que, ccb -> x_lci, 0)) {
		    enque (&ccb -> hold_que, wptr);
		}
	    }
	    if (ccb -> hold_que.ent_cnt) {
		ccb -> send_state = SEN_RTRN;
		wptr = ccb -> hold_que.first;
		int_on ();
		int_off ();
		ins_list (&ccb -> writ_que, wptr);
		ccb -> hold_que.first = NULL;
		ccb -> hold_que.last = NULL;
		ccb -> hold_que.ent_cnt = 0;
		ccb -> vs = seq;
		ccb -> rej_recv = SEN_RTRN + seq;
	    }
	    break;
	case RNR_TYP: 
	    /*
	     For an RNR packet, set the RNR received field to show we got one,
	     and what its p(r) was, set send hold state, and tell the channel
	     it's not OK to write any more.
	     */
	    ccb -> rnr_recv = RNR_RCV + seq;
	    ccb -> send_state |= SEN_HOLD;
	    chanst_upd (ccb -> x_lci, UPD_XMIT, FALSE);
    }
    int_on ();
 /*
    int_off ();                         * abort_inframe re-uses same que/buf *
    rel_que (que, NO_BUFF);             * release short input packet *
    int_on ();                          * released by pack_in
  */
    if (type != RNR_TYP) {
	/*
	 When this channel is sending, do nothing; pack_compl will call try_send.
	 When this channel is not sending, call try_send to put ccb->write_que
	 entry into lcb's out_que.                        
	 */
	int_off ();
	/*
	 If the link level does not have a packet for this chan, give it one
	 */
	if (ccb -> writ_que.ent_cnt + ccb -> hold_que.ent_cnt == ccb -> qall_out)
	    try_send (ccb);
	int_on ();
    }
    return;
}
