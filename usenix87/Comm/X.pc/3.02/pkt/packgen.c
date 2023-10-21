/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		    Packet Layer General Routines.
			filename = packgen.c
   chan_enable  accepts a channel number, and sets up data structure values to
		allow that channel to do input/output processing.
   chan_disable accepts a channel number, and disables that channel's ability
		to do input/output processing.  Releases all data held.
   -------------------------------------------------------------------------
   08/23/84  Mike  Remove mod_inc function
   08/23/84  Mike  enable interrupts between end_time calls in end_ques
   08/23/84  Mike  enable interrupts between end_cmd calls in end_one
   08/27/84  Mike  Eliminate duplicate timeout entries using strt/stop_time
   09/07/84  Mike  Fix rep_check RNR packet, and check active channel
   10/11/84  Mike  Do not ccb_zero in chan en/dis-able, can't zero P(R), P(S)
   10/11/84  Mike  Start different timeout when entering RNR sent recv_state
   10/11/84  Mike  Fix log_err to get bytes in right location
   09/26/86  Ed M  Reformat and re-comment
   ------------------------------------------------------------------------ */
#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
extern  rstr_time (), call_time (), clr_time (), intr_time (), rset_time ();
extern  send_time (), rnr_time (), rep_time (), rr_time ();/* timer routines */

/* ------------------------------------------------------------------------
   Set_state is called by packin when any state must be changed.  Thus, we
   can set ccb->cur_state and reset any lower-priority state.  The states
   are R1-R3 = 0-2, P1-P7 = 3-9, D1-D3 = 10-12.
------------------------------------------------------------------------ */

/*
  Set the current state of the channel to value.  Then check to see what
  the new state is.

  If it's a data state, set the channel's data state accordingly.

  If it's call/clear state (P1-P5), set the call/clear state to value, and
  if it's P4 state, set the current state to D1 (data transfer).

  If it's a restart state (R1-R3), set the restart state accordingly.
  If the channel is a virtual call, set the call clear and current states to
  P1, and set the data transfer state to zero.
  If it's not a virtual call then it must be a PVC, in which case the
  current state is set to D1, and the call clear state is set to 0.

  If after all this, we're in state D1, then say that the channel is OK for
  writing.
  */
void set_state (ccb, value)
CCB_PTR ccb;
byte value;
{
    extern void chanst_upd();

    ccb -> cur_state = value;
    if (value >= D1)
	ccb ->  d_state = value;
    else {
	if (value >= P1) {
	    ccb -> p_state = value;
	    if  (value == P4)
		ccb -> cur_state = D1;
	}
	else {
	    ccb -> r_state = value;
	    if  (value == R1) {
		if (ccb -> lc_vc) {
		    ccb -> p_state = P1;
		    ccb -> cur_state = P1;
		    ccb -> d_state = 0;
		}
		else {
		    ccb -> p_state = 0;
		    ccb -> cur_state = D1;
		}
	    }
	}
    }
    if (ccb -> cur_state == D1) {
	ccb ->  d_state = D1;
	chanst_upd (ccb -> x_lci, UPD_XMIT, TRUE);
    }
    return;
}

/* ------------------------------------------------------------------------
   Pack_check will convert the packet ID from the buffer into a useful index:
     0-5  diagnos, data, inter, int confirm, reset, reset confirm; for d1 state
     6-9  call, call accept, clear, clear confirm;                 for p4 state
    10-11 restart, restart confirm;                                for r1 state
    12-14 RR, RNR, REJ
     15   invalid packet type
------------------------------------------------------------------------- */
 /* 
  returns: 1=data    4=rset rq 7=call cn 10=reg req 13=rstr cn 16=RNR
	   2=inter   5=rset cn  8=clr req 11=reg con 14=diagnos 17=REJ
	   3=int con 6=call rq  9=clr con 12=rstr rq 15=RR      18=error
  */
int pack_check (que)
QUE_PTR que;
{
    int     n,
	    pkt_type;
 /* 
	RR in RN IC RJ  ca  ? CA di cl  ?  CC  ?        re  ?   RC data
  from:  0  1  2  3  4  5  6  7  8  9 10  11 12 13 14  15 no
  */
    static int  map[16] = {
	15, 2,  16, 3, 17, 6, 18, 7, 14, -10, 18, -11, 18, -12, 18, -13
    };
    if (que -> u.cmd.p_gfilci & C_BIT) {
	/* 
	 If packet is a control type, verify that it has a correct packet
	 ID (odd number).  If so, check to see if it is an exception.  If the
	 packet is a clear packet, decrement the value from the packet type
	 map by 2 to get the return value.  If it's a restart decrement by 8.
	 The error return (18) only happens on an even-numbered packet type.
	 */
	pkt_type = que -> u.cmd.p_id;
	if (pkt_type & 1) {
	    n = map[(pkt_type >> 1) & 0x0F];
	    if  (n < 0) {
		n = (-n);
		if ((pkt_type & 0xE0) == 0) {
		    if (n < 12)
			n -= 2;
		    else
			n -= 8;
		}
	    }
	}
	else
	    n = 18;
    }
    else
	n = 1;
    return (n);
}

/* ------------------------------------------------------------------------
   Log_err is called by any X.PC software when a message must be logged.
------------------------------------------------------------------------ */
log_err (sev_chan, source, mess_num, expl)
byte sev_chan, source, mess_num, expl[];
{
 /* 
  however, we cannot call DOS in any interrupt, so do nothing
  */
    return;
}

/* ------------------------------------------------------------------------
   err_fix is called by receive timeouts to check for bad application status,
   that is now good.  If so, update the channel status.
------------------------------------------------------------------------ */
err_fix ()
{
    CCB_PTR     ccb;		/* temp ccb */
    int         in,
		lci;
    extern void chanst_upd();
    extern int  chanst_rep();
 /* 
  For each active chan, check for bad XMIT or Receive status.
  */
    for (lci = 1; lci <= 15; lci++) {
	if (ccb = lcb.ccbp[lci]) {
	    in  = chanst_rep (lci);
	    if  ((in >> 8) == 0) {
		/* 
		 If receive is not running, and the channel is in data transfer
		 (D1)  state, and the channel has not received an RNR packet, the
		 driver is ready to handle more data (buffers and queues are
		 available), and the data window is not full; then turn
		 transmit back on for that channel.
		 */
		if (ccb -> d_state ==  D1
			&& ccb -> rnr_recv == 0
			&& lcb.xpc_nrdy == 0
			&& ccb -> qued_out < ccb -> dat_window)
		    chanst_upd (lci, UPD_XMIT, TRUE);
	    }
	    if  ((in & 0xFF) == 0) {
		/* 
		 If transmit is not running, and data is queued up, turn on
		 receive.
		 */
		if (ccb -> qued_in > 0)
		    chanst_upd (lci, UPD_RECV, TRUE);
	    }
	}
    }
    return;
}

/* -------------------------------------------------------------------------
   chan_enable is called by pack_open when a channel is being enabled for I/O.
   It is also called by several restart conditions, make no assumptions about
   the previous status of any fields.
   ------------------------------------------------------------------------- */
void chan_enable (lci)
int     lci;
{
    extern void chanst_upd();
 /* 
  If in packet mode, do nothing, otherwise update the xmit enable
  */
    if ((lcb.driver_mode & PACK_FLAG) != 0) {
	nothing;
    /* ccb_zero (lcb.ccbp[lci]);     DELETED, CAN'T ZERO SEQ NUMBERS */
    }
    else {
	chanst_upd (lci, UPD_XMIT, TRUE);
    }
    return;
}

/* -------------------------------------------------------------------------
   chan_disable is called by pack_close when a channel is being disconnected.
   It is also called by several restart conditions, make no assumptions about
   the previous status of any fields.
   This function presupposes that the channel was OPEN.  Otherwise, the fields
   would contain junk, and could cause problems in other functions.
   ------------------------------------------------------------------------- */
void chan_disable (lci, pktid)
int     lci,
	pktid;
{
    CCB_PTR      ccb;
    extern void chanst_upd();

 /* 
  turn off recv/xmit flags in PAD.
  */
    chanst_upd (lci, UPD_RECV, FALSE);
    chanst_upd (lci, UPD_XMIT, FALSE);
    int_off ();
    stop_time (rr_time, lci, TIM_RR);
    int_on ();
    int_off ();
    stop_time (rep_time, lci, TIM_REP);
    int_on ();
    int_off ();
    stop_time (send_time, lci, TIM_SEND);
    int_on ();
    int_off ();
    stop_time (intr_time, lci, TIM_INTR);
    int_on ();
    if (pktid != RSETR_PKT) {
	int_off ();
	stop_time (rset_time, lci, TIM_RSET);
	int_on  ();
    }
    if (pktid == RSTRR_PKT) {
	int_off ();
	stop_time (clr_time, lci, TIM_CLR);
	int_on  ();
	int_off ();
	stop_time (call_time, lci, TIM_CALL);
	int_on  ();
    /* 
     stop_time (rstr_time,lci,TIM_RSTR);         * term_pktmode ends restart
     */
    }
    end_ques (lci, pktid);
    if (ccb = lcb.ccbp[lci])
	ccb ->  data_in = ccb -> qued_in = ccb -> qall_out = ccb -> qued_out = 0;
 /* 
  ccb_zero (lcb.ccbp[lci]);      CAN'T ZERO SEQ NUMBERS nor CHANGE STATE
  */
    return;
}

/* -------------------------------------------------------------------------
   end_ques is called by term_pktmode and chan_disable to clear packets from
   all queues, in lcb and ccb, based on the priority of the packet.
   ------------------------------------------------------------------------- */
end_ques (lci, pktid)
int     lci,
	pktid;
{
    CCB_PTR ccb;

 /* 
  Remove all entries from the lcb queues.
  */
    end_one (&lcb.in_que, lci, pktid);
    end_one (&lcb.out_que, lci, pktid);
    end_one (&lcb.rep_que, lci, pktid);
    if (ccb = lcb.ccbp[lci]) {
	/* 
	 If a valid logical channel, dump its queues too
	 */
	end_one (&ccb -> read_que, lci, pktid);
	end_one (&ccb -> writ_que, lci, pktid);
	end_one (&ccb -> hold_que, lci, pktid);
    }
    return;
}

/* -------------------------------------------------------------------------
   end_one is called by end_ques to clear packets from all queues, in lcb and
   ccb, based on the priority of the packet.
   NOTE: When end_cmd returns NULL, there may still be
	 higher priority packets left in the que.
   ------------------------------------------------------------------------- */
end_one (qhead, lci, pktid)
QHEADPTR qhead;
int     lci,
	pktid;
{
    int     found;
    extern int  end_cmd();
    found = TRUE;
    int_off ();
    while (found && (qhead -> ent_cnt != 0)) {
	found = end_cmd (qhead, lci, pktid);
	int_on  ();
	int_off ();
    }
    int_on ();
    return;
}
