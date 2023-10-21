/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.              *
*****************************************************************************/
#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
extern  rr_time (), rnr_time ();/* define external funtions */
int     rset_time (), intr_time ();/* define internal funtions */
/* ------------------------------------------------------------------------
   08/27/84  Mike  Eliminate duplicate timeout entries using strt/stop_time
   08/30/84  Mike  Fix setting of RNR, and add rnr_time (out)
   10/01/84  Mike  Make r1state set recv_state and rnr_sent when sending RNR
   10/11/84  Mike  add ccb_zero to reset; no longer done in chan_dis/enable
   10/17/84  Mike  correct d1state's default case test of full window
   10/19/84  Mike  make reset confirm set state, even when sending confirm
   10/23/84  Mike  make reset confirm set state, r1 amd d1
   11/28/84  Mike  remove unused variable ps in d1state
   11/28/84  Mike  fix timeout logic to try again
   09/26/86  Ed M  Reformat and recomment the code
------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------
   data_state is called in state r1 as a permanent virtual circuit (PVC),
   or in state p4, within state r1, as a switched virtual circuit  (SVC).
------------------------------------------------------------------------- */
data_state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    switch (ccb -> d_state) {
	case D1: 
	    d1state (ccb, que);
	    break;
	case D2: 
	    d2state (ccb, que);
	    break;
	case D3: 
	    d3state (ccb, que);
    }
    return;
}

/* ------------------------------------------------------------------------
   D1state is the normal data transfer state; only reset will take us out.
------------------------------------------------------------------------- */
d1state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    int     num;

    if (ccb -> in_error) {
	/*
	If we found an error before, process it now.  Report the error, sending
	a reset for the channel, with a cause of local procedure error, and set
	the state accordingly.
	*/
	data_err (ccb, RES_LOCPE, ccb -> in_error);
	set_state (ccb, D3);
    }
    else {
	switch  (ccb -> in_pktindex) {
	    case RSETR_TYP: 
		/*
		 On a reset request, pass the request along to the
		 application, set the state according to the driver mode,
		 send a restart confirmation, and toss this packet.
		 */
		rset_done (ccb, que, RSETR_PKT);
		set_state (ccb, (lcb.driver_mode == PDTE_MODE) ? D2 : D3);
		conf_send (ccb -> x_lci, RSETC_PKT);
		ccb -> in_error = IGNORE;
		break;
	    case RSETC_TYP:
		/*
		 On a reset confirm, log the error, and go into the
		 appropriate data state.  Toss the packet.
		 */
		data_err (ccb, RES_LOCPE, 27);
		set_state (ccb, (lcb.driver_mode == PDTE_MODE) ? D2 : D3);
		ccb -> in_error = IGNORE;
		break;
	    case INT_TYP: 
		/*
		 Check to see if we already have an interrupt request
		 outstanding.  If we do, this is an error, so record it as
		 such and send a reset with error code 44, do the
		 appropriate thing with the state, and toss the packet.

		 If we don't have an outstanding interrupt request, say
		 that we do now, send an interrupt confirm, and accept the
		 packet.
		 */
		switch (ccb -> int_recv) {
		    case FALSE: 
			ccb -> int_recv = TRUE;
			conf_send (ccb -> x_lci, INTC_PKT);
			pack_rdy (ccb, que);
			break;
		    case TRUE: 
			data_err (ccb, RES_LOCPE, 44);
			set_state (ccb, D2);
			ccb -> in_error = IGNORE;
		}
		break;
	    case INTC_TYP: 
		/*
		 On an interrupt confirm packet:

		 If we didn't send an interrupt request, it's an error.
		 Send  a reset code 43, do the appropriate thing to the data
		 state and throw away the packet.

		 If we did send one, re-enable our ability to send
		 interrupt requests, turn off the timer, and accept the
		 packet.
		 */
		switch (ccb -> int_sent) {
		    case FALSE: 
			data_err (ccb, RES_LOCPE, 43);
			set_state (ccb, D2);
			ccb -> in_error = IGNORE;
			break;
		    case TRUE: 
			ccb -> send_state &= ~SEN_INTR;
			int_off ();
			stop_time (intr_time, ccb -> x_lci, TIM_INTR);
			int_on ();
			ccb -> int_sent = FALSE;
			conf_send (ccb, INTC_PKT);
			pack_rdy (ccb, que);
		}
		break;
	    default:
		/*
		check for
		1) current RNR,
		2) appl read_que over filled,
		3) available buffer or queue count is low.
		If so, send RNR to stop sender
		*/
		num = ccb -> rnr_sent & ~0x0F;
		if (ccb -> read_que.ent_cnt + 1 > ccb -> dat_window)
		    num |= RNR_WIN;
		if (lcb.xpc_nrdy != 0)
		    num |= RNR_XPC;
		if (num != 0) {
		    ccb -> next_reply = REP_RNR;
		    ccb -> rnr_sent = num + ccb -> vr;
		}
		pack_rdy (ccb, que);
	}
    }
    return;
}

/* ------------------------------------------------------------------------
   D2state is entered when DTE sends a Reset, and exited when DCE confirms.
   Note: DCE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
d2state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    switch (ccb -> in_pktindex) {
	/*
	 Check packet type.

	 If a reset request and we're a DTE, confirm reset, get packin to send
	 an RR, tell the application that we've been reset, confirm the reset.
	 If reset request and we're a DCE do nothing.  In either case, toss the
	 packet.

	 If a reset confirm and we're a DTE, get packin to send an RR, confirm
	 the reset.  If we're a DCE, send a reset with error code 28 and LOCPE
	 and go to D2 state.  In either case, toss the packet.

	 On any other kind of packet, toss the packet, send a reset with error
	 code 28 and LOCPE.  If a DCE go to state D3.
	 */
	case RSETR_TYP: 
	    ccb -> in_error = IGNORE;
	    if  (lcb.driver_mode == PDTE_MODE) {
		ccb -> next_reply = REP_RR;
		rset_done (ccb, que, RSETR_PKT);
		rset_conf (ccb, que);
	    }
	    else {
		nothing;
	    }
	    break;
	case RSETC_TYP: 
	    ccb -> in_error = IGNORE;
	    if  (lcb.driver_mode == PDTE_MODE) {
		ccb -> next_reply = REP_RR;
		rset_conf (ccb, que);
	    }
	    else {
		data_err (ccb, RES_LOCPE, 28);
		set_state (ccb, D2);
	    }
	    break;
	default: 
	    ccb -> in_error = IGNORE;
	    data_err (ccb, RES_LOCPE, 28);
	    if  (lcb.driver_mode == PDTE_MODE) {
		nothing;
	    }
	    else {
		set_state (ccb, D3);
	    }
    }
    return;
}

/* ------------------------------------------------------------------------
   D3state is entered when DCE sends a Reset, and exited when DTE confirms.
   Note: DTE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
d3state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    /*
     If error found before, report it, and go to state D3.
     */
    if (ccb -> in_error) {
	data_err (ccb, RES_LOCPE, ccb -> in_error);
	set_state (ccb, D3);
    }
    else {
	switch (ccb -> in_pktindex) {
	    /*
	     Check packet type:

	     If a reset request, and we're a DTE, do nothing.  If we're a DCE,
	     set up packin to send an RR next time, pass along the fact that
	     we've been reset to the application, and confirm the reset.  In
	     either case, toss the packet.

	     If a reset confirm, and we're a DTE, send a reset with an error
	     code of 29 and LOCPE, stay in state D3.  If we're a DCE, tell
	     packin to send an RR packet, confirm the reset.  In either case,
	     toss the packet.

	     If an interrupt packet and we're a DCE, ignore it.  If we're a
	     DTE send a reset with code 29 and LOCPE and toss the packet.
	     Either way, stay in D3.

	     Any other type of packet is simply ignored for the time being.
	     */
	    case RSETR_TYP:
		ccb -> in_error = IGNORE;
		if (lcb.driver_mode == PDTE_MODE) {
		    nothing;
		}
		else {
		    ccb -> next_reply = REP_RR;
		    rset_done (ccb, que, RSETR_PKT);
		    rset_conf (ccb, que);
		}
		break;
	    case RSETC_TYP: 
		ccb -> in_error = IGNORE;
		if (lcb.driver_mode == PDTE_MODE) {
		    data_err (ccb, RES_LOCPE, 29);
		    set_state (ccb, D3);
		}
		else {
		    ccb -> next_reply = REP_RR;
		    rset_conf (ccb, que);
		}
		break;
	    case INT_TYP: 
		ccb -> in_error = IGNORE;
		if (lcb.driver_mode == PDTE_MODE) {
		    data_err (ccb, RES_LOCPE, 29);
		}
		else {
		    nothing;
		}
		break;
	    default: 
		ccb -> in_error = IGNORE;
		if (lcb.driver_mode == PDTE_MODE) {
		     /* ??????  */ ;
		}
		else {
		    nothing;
		}
	}
    }
    return;
}

/* ------------------------------------------------------------------------
   rset_send is called by clrr_err when an input error is detected.
   will send to the local DXE a Clear Packet, with the given cause and code;
------------------------------------------------------------------------ */
rset_send (lci, cause, code)
byte    lci,                    /* logical channel number */
	cause,                  /* reset cause */
	code;                   /* diagnostic code */
{
    extern QUE_PTR      get_que();
    QUE_PTR             que;
    BUF_PTR             buf;
    /*
    If we have a valid lci, say reset is done, get a queue.  If we've got
    one, put the circuit state to reset, say we're using channel 0, put
    in the reset request packet ID, set the bufferlet size, put the cause
    and diagnostic code in the bufferlet, and tell pack_write to queue it
    up.
    */
    if (lcb.ccbp[lci]) {
	rset_done (lcb.ccbp[lci], NULL, RSETR_PKT);
	int_off ();
	que = get_que (YES_BUFF);
	int_on ();
	if (que != 0) {
	    lcb.ccbp[lci] -> circ_state |= CIR_RSET;
	    que -> u.cmd.p_gfilci = XPC_WRITE;
	    que -> u.cmd.p_id = RSETR_PKT;
	    buf = que -> u.cmd.bfirst;
	    buf -> num_bytes = B_BYTE0 + 3;
	    buf -> data[B_BYTE0 + 1] = cause;
	    buf -> data[B_BYTE0 + 2] = code;
	    pack_write (lci, que, 3);
	/* let pack_write set retry */
	}
    }
    return;
}

/* ---------------------------------------------------------------------------
   intr_time is called by pack_srtart timeout.  Retry n times and then give up.
------------------------------------------------------------------------- */
intr_time (lci)
int     lci;
{
    extern void    rel_que(),
		   link_write();
    extern QUE_PTR fid_cmd();
    CCB_PTR     ccb;
    QUE_PTR     que;
    if (ccb = lcb.ccbp[lci]) {
	/*
	 If we have a valid lci, clear the timer bit, so we can start again.
	 Try to find the Interrupt packet in the queue.  If we do, check for
	 retry expired.  If it has, make pk_start reset the retry count,
	 release the interrupt request, and send a reset with code 145
	 indicating time has expired..

	 If retry is not done, set the queue to show a retransmission, and use
	 link_write to transmit it again, and reset the timer.
	 */
	ccb -> time_run &= ~TIM_INTR;
	int_off ();
	if ((que = fid_cmd (&ccb -> hold_que, lci, INT_PKT)) != NULL) {
	    int_on ();
	    if  (--ccb -> retry20[6] == 0) {
		que -> u.cmd.b_state &= ~B_STRTD;
		int_off ();
		rel_que (que, YES_BUFF);
		int_on ();
		data_err (ccb, RES_LOCPE, 145);
	    }
	    else {
		que -> u.cmd.b_state = (que -> u.cmd.b_state & ~B_STRTD) | B_RETRAN;
		link_write (LWRIT_OUT, que);
		int_off ();
		strt_time (T26 * EVNT_PSEC, intr_time, lci, TIM_INTR);
	    }
	}
	int_on ();
    }
    return;
}

/* ---------------------------------------------------------------------------
   rset_time is called by restart timeout.  Retry n times and then give up.
------------------------------------------------------------------------- */
rset_time (lci)
int     lci;
{
    extern void    rel_que(),
		   link_write();
    extern QUE_PTR fid_cmd();
    CCB_PTR     ccb;
    QUE_PTR     que;
    int         cause,
		state;
    static char empty[] = {
	0, 0, 0
    };
    if (ccb = lcb.ccbp[lci]) {
	/*
	 If lci is valid, clear the reset timer bit.  If we can find the reset
	 request in the queue, check to see if retry count is expired.  If it
	 is, and we have a virtual call, get rid of the reset request.  Then set
	 the driver to state P6 if a DTE or P7 if it's a DCE, and send a clear
	 request with a cause of 0 for a DTE and LOCPE for a DCE.

	 If it wasn't a virtual call, but instead a PVC, send a reset request
	 indicating a timeout (51) waiting for a reset on the channel.

	 If the count hasn't expired, flag the queue as a retransmission, tell
	 link_write to send it, and restart the timer.
	 */
	ccb -> time_run &= ~TIM_RSET;
	int_off ();
	if ((que = fid_cmd (&ccb -> hold_que, lci, RSETR_PKT)) != NULL) {
	    int_on ();
	    if (--ccb -> retry20[2] == 0) {
		if (ccb -> lc_vc) {
		    int_off ();
		    rel_que (que, YES_BUFF);
		    int_on ();
		    if (lcb.driver_mode == PDTE_MODE) {
			state = P6;
			cause = 0;
		    }
		    else {
			state = P7;
			cause = CLR_LOCPE;
		    }
		    clr_send (lci, cause, 50);
		    set_state (ccb, state);
		}
		else {
		    empty[1] = lci;
		    diag_err (ccb, empty, 51);
		    int_off ();
		    strt_time (T22 * EVNT_PSEC, rset_time, lci, TIM_RSET);
		}
	    }
	    else {
		que -> u.cmd.b_state = (que -> u.cmd.b_state & ~B_STRTD) | B_RETRAN;
		link_write (LWRIT_OUT, que);
		int_off ();
		strt_time (T22 * EVNT_PSEC, rset_time, lci, TIM_RSET);
	    }
	}
	int_on ();
    }
    return;
}

/* ------------------------------------------------------------------------
   rset_conf is called by r2state or r3state when either a restart request or
   a restart confirm is received, and when one is sent (que = NULL).
------------------------------------------------------------------------ */
rset_conf (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    if (que) {
	/*
	 A confirm was received, stop the timeout, clear the retry counter,
	 release the restart request.
	 */
	int_off ();
	stop_time (rset_time, ccb -> x_lci, TIM_RSET);
	ccb -> retry20[2] = 0;
	int_on ();
	rel_reqst (ccb, RSETR_PKT);
    }
    /*
     Set state to R1 and D1, set the circuit state flag to show the reset,
     and start the idle timer (wait for RR).
     */
    set_state (ccb, R1);
    ccb -> circ_state |= CIR_RSET;
    int_off ();
    strt_time (lcb.htime << 2, rr_time, ccb -> x_lci, TIM_RR);
    int_on ();
    return;
}

#define PKT_RESET 0x20
/* ------------------------------------------------------------------------
   rset_done is called by rstr_done when a restart request is received (recv=1),
   or its output is completed (recv=0), to clear a virtual call channel.
   The pack variable is RSTRR_PKT, CLRR_PKT, or RSETR_PKT.
------------------------------------------------------------------------ */
rset_done (ccb, que, pack)
CCB_PTR ccb;                    /* real ccb */
QUE_PTR que;                    /* real que = 0, if restart sent */
int     pack;                   /* packet level to clear */
{
    void chanst_upd();
    if (ccb -> circ_state & CIR_ACTV) {
	ccb -> circ_state &= ~CIR_ACTV;
	if (pack == RSETR_PKT)
	    chanst_upd (ccb -> x_lci, UPD_RESET, TRUE);
    }
    /*
     Turn off xmit and recv, clear queues if lower priority than reset,
     clear out all send/receive and channel status variables and the data
     state.

     If reset was received, start receive window at 1 again.
     */
    chanst_upd (ccb -> x_lci, UPD_XMIT, FALSE);
    chanst_upd (ccb -> x_lci, UPD_RECV, FALSE);
    chan_disable (ccb -> x_lci, pack);
    ccb_zero (ccb);
    chan_enable (ccb -> x_lci);
    ccb -> d_state = 0;
    if (que != NULL)
	ccb -> vr = ccb -> vrl = 1;
    return;
}

/* ------------------------------------------------------------------------
   data_err is called by restart software when an input error is detected.
   The packet must be discarded by the caller.  As a DCE, this function will
   send to the local DTE a Restart packet, with "local procedure error" cause.
   - If virtual call is connected, the distant DTE is informed by a clear indi-
     cation packet, with "remote procedure error" cause (same diagnostic).
   - If a perm virtual ciruit, the distant DTE is informed by a reset indi-
     cation packet, with "remote procedure error" cause (same diagnostic).
   If current state is R2, change to state R1.
------------------------------------------------------------------------ */
data_err (ccb, cause, code)
CCB_PTR ccb;
int     cause,
	code;
{
    int     i;
    static char zero[] = {
	0, 0, 0
    };
    i = cause;
    if (lcb.driver_mode == PDTE_MODE) {
	/*
	 If a DTE change cause to local procedure error and set state to D2
	 */
	i = RES_LOCPE;
	set_state (ccb, D2);
    }
    else {
	/*
	 If a DCE just set state to D3.
	 */
	set_state (ccb, D3);
    }
    rset_send (ccb -> x_lci, i, code);
    zero[1] = ccb -> x_lci;
    log_err ((MS_INFO << 4) + ccb -> x_lci, MO_PACK, MP_RSETSENT, zero);
 /* sev_chan, origin, mess_num */
    set_state (ccb, R1);
    return;
}
