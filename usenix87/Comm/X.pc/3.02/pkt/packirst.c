/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
extern  rr_time ();
int     rstr_time ();
/* ------------------------------------------------------------------------
   08/27/84  Mike  Eliminate duplicate timeout entries using strt/stop_time
   09/06/84  Mike  Fix receiver of restart, to set R1 state in rstr_conf
   10/23/84  Mike  Add initial Restart state, R0, (waiting for restart)
    2/11/85  Mike  Make switch on r_state use channel 0
    9/26/86  Ed M  Reformat and re-comment
   ------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------
   Rst_state is the restart state machine
------------------------------------------------------------------------- */
rst_state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    switch (lcb.ccbp[0] -> r_state) {
	case R0: 
	    /*
	     If we get a restart packet, set state to R1 for this channel, and
	     call the proper state handler.

	     If packet is not a restart request, send out one indicating error
	     code 227, and throw away the packet.
	     */
	    if (ccb -> in_pktindex == RSTRR_TYP) {
		ccb -> r_state = R1;
		r1state (ccb, que);
	    }
	    else {
		diag_err (lcb.ccbp[0], &que -> u.cmd.p_gfilci, 227);
		ccb -> in_error = IGNORE;
	    }
	    break;
	case R1: 
	    r1state (ccb, que);
	    break;
	case R2: 
	    r2state (ccb, que);
	    break;
	case R3: 
	    r3state (ccb, que);
    }
    return;
}

/* ------------------------------------------------------------------------
   R1state is the normal ready state; only Restart will take us out.
------------------------------------------------------------------------- */
r1state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    switch (ccb -> in_pktindex) {
	case RSTRR_TYP:
	    /*
	     For a restart request, if we have an error already, send a restart
	     indicating LOCPE.  Otherwise, check the diagnostic code for 241.
	     If it is set the circuit terminated bit.  After that, tell the
	     application about the restart, set the restart state machine to
	     the appropriate state, send a restart confirm, and toss the
	     packet.
	     */
	    if (ccb -> in_error)
		rstr_err (ccb, que, RST_LOCPE, ccb -> in_error);
	    else {
		if (que -> u.cmd.bfirst -> data[B_BYTE1 + 1] == 241)
		    ccb -> circ_state |= CIR_TERM;
		rstr_done (ccb, que);
		set_state (ccb, (lcb.driver_mode == PDTE_MODE) ? R3 : R2);
		conf_send (ccb -> x_lci, RSTRC_PKT);
	    /* rstr_conf (ccb,NULL);            this will be called when acknowledge */
		ccb -> in_error = IGNORE;
	    }
	    break;
	case RSTRC_TYP:
	    /*
	     On a restart confirm, if there was no previous error, set the
	     error flag to 17, set the restart state to the appropriate value.
	     In any case, report an error.
	     */
	    if (ccb -> in_error == 0) {
		ccb -> in_error = 17;
		set_state (ccb, (lcb.driver_mode == PDTE_MODE) ? R2 : R3);
	    }
	    rstr_err (ccb, que, RST_LOCPE, ccb -> in_error);
	    break;
	case REGR_TYP:  /* Fall through */
	case REGC_TYP: 
	    /*
	     On a registration request or confirmation, if there was no old
	     error value set the error value to 33.  Either way send a restart
	     error.
	     */
	    if (ccb -> in_error == 0)
		ccb -> in_error = 33;
	    rstr_err (ccb, que, RST_LOCPE, ccb -> in_error);
	    break;
	case DIAG_TYP: 
	    /*
	     On a diagnostic packet, if there was an old error, report it.  If
	     not, log the fact that a diagnostic was sent.  In any case, toss
	     the packet.
	     */
	    if (ccb -> in_error)
		rstr_err (ccb, que, RST_LOCPE, ccb -> in_error);
	    else {
		log_err ((MS_INFO << 4), MO_PACK, MP_DIAGSENT, &que -> u.cmd.p_gfilci);
		ccb -> in_error = IGNORE;
	    }
	    break;
	default: 
	    /*
	     For all other packet types, just call the appropriate state
	     machine.  (pkt_state for virtual calls, or data_state for PVC's.
	     */
	    if (ccb -> lc_vc)
		pkt_state (ccb, que);
	    else
		data_state (ccb, que);
    }
    return;
}

/* ------------------------------------------------------------------------
   R2state is entered when DTE sends a Restart, and exited when DCE confirms.
   Note: DCE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
r2state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    if (ccb -> in_error)
	rstr_err (ccb, que, RST_LOCPE, ccb -> in_error);
    else {
	switch (ccb -> in_pktindex) {
	    case RSTRR_TYP: 
		/*
		 On a Restart Request:

		 If we are a DTE, get packin to send an RR packet, flag that we
		 are doing a restart.  Confirm the restart, and toss the
		 packet.

		 If we are a DCE, just throw away the packet, ignoring the
		 extra restart.
		 */
		if (lcb.driver_mode == PDTE_MODE) {
		    ccb -> next_reply = REP_RR;
		    rstr_done (ccb, que);
		    rstr_conf (ccb, que);
		    ccb -> in_error = IGNORE;
		}
		else {
		    ccb -> in_error = IGNORE;
		}
		break;
	    case RSTRC_TYP:
		/*
		 On a Restart Confirm:

		 If we are a DTE, get packin to send an RR, confirm the
		 restart, and throw awy the packet.

		 If we are a DCE, report an error 18, set the state to R3, and
		 throw away the packet.
		 */
		if (lcb.driver_mode == PDTE_MODE) {
		    ccb -> next_reply = REP_RR;
		    rstr_conf (ccb, que);
		    ccb -> in_error = IGNORE;
		}
		else {
		    rstr_err (ccb, que, RST_LOCPE, 18);
		    set_state (ccb, R3);
		    ccb -> in_error = IGNORE;
		}
		break;
	    case REGR_TYP:      /* Fall through */
	    case REGC_TYP: 
		/*
		 On a Registration Request or confirm, issue an error 33.
		 */
		rstr_err (ccb, que, RST_LOCPE, 33);/* old error */
		break;
	    case DIAG_TYP: 
		/*
		 Report a diagnostic received and ignore the packet.
		 */
		log_err ((MS_INFO << 4), MO_PACK, MP_DIAGSENT, &que -> u.cmd.p_gfilci);
		ccb -> in_error = IGNORE;
		break;
	    default: 
		/*
		 For any other type of packet (data, datagram, interrupt, call,
		 flow ctrl, reset):

		 If we're a DTE, just ignore the packet.

		 If we're a DCE, report error 18, go to R3 state, and ignore
		 the packet.
		 */
		if (lcb.driver_mode == PDTE_MODE) {
		    ccb -> in_error = IGNORE;
		}
		else {
		    rstr_err (ccb, que, RST_LOCPE, 18);
		    set_state (ccb, R3);
		    ccb -> in_error = IGNORE;
		}
	}
    }
    return;
}

/* ------------------------------------------------------------------------
   R3state is entered when DCE sends a Restart, and exited when DTE confirms.
   Note: DTE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
r3state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    if (ccb -> in_error)
	rstr_err (ccb, que, RST_LOCPE, ccb -> in_error);
    else {
	switch (ccb -> in_pktindex) {
	    case RSTRR_TYP:
		/*
		 On a Restart Request:

		 If we're a DTE, ignore the packet.

		 If we're a DCE, tell packin to send an RR, tell the
		 application, clear the retry timer and end timeout (rstr_conf)
		 and ignore the packet.
		 */
		if (lcb.driver_mode == PDTE_MODE) {
		    ccb -> in_error = IGNORE;
		}
		else {
		    ccb -> next_reply = REP_RR;
		    rstr_done (ccb, que);
		    rstr_conf (ccb, que);
		    ccb -> in_error = IGNORE;
		}
		break;
	    case RSTRC_TYP: 
		/*
		 On a Restart Confirm:

		 If we're a DTE, report error 19, go to state R2, and toss the
		 packet.

		 If we're a DCE, send an RR, confirm the sucker, and ignore the
		 packet.
		 */
		if (lcb.driver_mode == PDTE_MODE) {
		    rstr_err (ccb, que, RST_LOCPE, 19);
		    set_state (ccb, R2);
		    ccb -> in_error = IGNORE;
		}
		else {
		    ccb -> next_reply = REP_RR;
		    rstr_conf (ccb, que);
		    ccb -> in_error = IGNORE;
		}
		break;
	    case REGR_TYP:      /* Fall through */
	    case REGC_TYP:
		/*
		 On a registration request or confirm, report error 33.
		 */
		rstr_err (ccb, que, RST_LOCPE, 33);/* old error */
		break;
	    case DIAG_TYP: 
		/*
		 On a diagnostic, report we received one and eat the packet.
		 */
		log_err ((MS_INFO << 4), MO_PACK, MP_DIAGSENT, &que -> u.cmd.p_gfilci);
		ccb -> in_error = IGNORE;
		break;
	    default: 
		/*
		 For any other type of packet, eat it.

		 If we're a DTE, report error 19.
		 */
		ccb -> in_error = IGNORE;
		if (lcb.driver_mode == PDTE_MODE) {
		    rstr_err (ccb, que, RST_LOCPE, 19);
		}
		else {
		    nothing;
		}
	}
    }
    return;
}

/* ------------------------------------------------------------------------
   rstr_err is called by restart software when an input error is detected.
   The packet must be discarded by the caller.  As a DCE this function will
   send to the local DTE a Restart packet, with "local procedure error" cause.
   - If virtual call is connected, the distant DTE is informed by a clear indi-
     cation packet, with "remote procedure error" cause (same diagnostic).
   - If a perm virtual ciruit, the distant DTE is informed by a reset indi-
     cation packet, with "remote procedure error" cause (same diagnostic).
   If current state is R2, change to state R1.
------------------------------------------------------------------------ */
rstr_err (ccb, que, cause, code)
CCB_PTR ccb;
QUE_PTR que;
int     cause,                  /*  Restart Cause */
	code;                   /*  Diagnostic Code */
{
    int     i;
    /*
     Flag packet so it gets tossed.
     */
    ccb -> in_error = code;
    i = cause;
    /*
     If we're a DTE, change the cause to a DTE Originated Restart.
     */
    if (lcb.driver_mode == PDTE_MODE)
	i = RST_DTEORG;
    /*
     1984 CCITT allows DTE cause + 0x80, which DCE must ignore
     */
    else {
	/*
	 network DCE must tell remote DTE to clear call
	 */
	if (ccb -> lc_vc) {
	    nothing;            
	}
	else {
	    nothing;
	}
    }
    /*
     Send a restart packet with the modified cause field and diagnostic
     code as passed.  Log the error, and if we were in R2, go to R1.
     */
    rstr_send (i, code);
    log_err (MS_INFO << 4, MO_PACK, MP_RSTRSENT, &que -> u.cmd.p_gfilci);
    if (ccb -> r_state == R2)
	set_state (ccb, R1);
    return;
}

/* ------------------------------------------------------------------------
   rstr_send is called by rstr_err when an input error is detected.
   will send to the local DXE a Restart packet, with the given cause and code;
------------------------------------------------------------------------ */
rstr_send (cause, code)
byte cause, code;
{
    extern QUE_PTR      get_que();
    QUE_PTR             que;
    BUF_PTR             buf;

    int_off ();
    que = get_que (YES_BUFF);
    int_on ();
    if (que != 0) {
	/*
	 If the code indicates termination of packet mode (241), then set the
	 appropriate bit in the circuit state.

	 Flag as restart done for all channels, and queue up a Restart Request
	 packet on channel 0 with the cause and code passed, and call
	 pack_write to set it up for transmission.
	 */
	if (code == 241)
	    lcb.ccbp[0] -> circ_state |= CIR_TERM;
	rstr_done (lcb.ccbp[0], NULL);
	que -> u.cmd.p_gfilci = XPC_WRITE;
	que -> u.cmd.p_id = RSTRR_PKT;
	buf = que -> u.cmd.bfirst;
	buf -> num_bytes = B_BYTE0 + 3;
	buf -> data[B_BYTE0 + 1] = cause;
	buf -> data[B_BYTE0 + 2] = code;
	pack_write (0, que, 3); 
    }
    return;
}

/* ------------------------------------------------------------------------
   rstr_time is called by timeout when the DXE does not respond to our restart
   within the time limit T20.  If retry20[0] decrements below zero, give up.
------------------------------------------------------------------------ */
rstr_time (lci)
int     lci;
{
    extern QUE_PTR      fid_cmd();
    extern void         link_write();
    CCB_PTR             ccb;
    QUE_PTR             que;
    static char         empty[] = {
	0, 0, 0
    };
    if (ccb = lcb.ccbp[lci]) {
	/*
	 If we have a valid lci, clear the timer running bit for Restart, and
	 try to find the packet in the hold queue.
	 */
	ccb -> time_run &= ~TIM_RSTR;
	int_off ();
	que = fid_cmd (&ccb -> hold_que, lci, RSTRR_PKT);
	int_on ();
	if (que != NULL) {
	    if (--ccb -> retry20[0] == 0) {
		/*
		 R20 has expired, so complain.
		 */
		diag_err (0, empty, 52);
	    }
	    else {
		/*
		 R20 hasn't expired yet.  Flag the packet as a retransmission,
		 and tell link_write to send it.
		 */
		que -> u.cmd.b_state = (que -> u.cmd.b_state & ~B_STRTD) | B_RETRAN;
		link_write (LWRIT_OUT, que);
	    }
	}
	int_off ();
	strt_time (T20 * EVNT_PSEC, rstr_time, lci, TIM_RSTR);
	int_on ();
    }
    return;
}

/* ------------------------------------------------------------------------
   rstr_conf is called by r2state or r3state when either a restart request or
   a restart confirm is received, and when one is sent (que = NULL) and RR.
------------------------------------------------------------------------ */
rstr_conf (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    int     lci;
    if (que) {
	/*
	 We got a confirm, end all timeouts, including restart, clear the retry
	 counter, and release the Restart Request packet that started it all.
	 */
	int_off ();
	stop_time (0, 0, 0xFF);
	ccb -> retry20[0] = 0;
	int_on ();
	rel_reqst (ccb, RSTRR_PKT);
    }
    /*
     Say that we've restarted.  Set the RR timeout to 8 seconds, and set
     all active channels to R1 state (this also sets them to P1 and D1).
     */
    ccb -> circ_state |= CIR_RSTR;
    int_off ();
    strt_time (EVNT_PSEC * 8, rr_time, 0, TIM_RR);
    int_on ();
    for (lci = 15; lci >= 0; lci--) {
	if (ccb = lcb.ccbp[lci])
	    set_state (ccb, R1);
    }
    return;
}

/* ------------------------------------------------------------------------
   rstr_done is called by rXstate when a restart request is received (recv=1),
   or its output is completed (recv=0), to give the packet to each channel.
------------------------------------------------------------------------ */
rstr_done (ccb, que)
CCB_PTR ccb;                    /* real ccb */
QUE_PTR que;                    /* real que = 0, if restart sent */
{
    extern void chanst_upd();
    int         lci,
		cause;
    for (lci = 15; lci >= 0; lci--) {
	if (ccb = lcb.ccbp[lci]) {
	    if (lci == 0) {
		rset_done (ccb, NULL, RSTRR_PKT);
		if (ccb -> circ_state & CIR_OPEN) {
		    cause = ((ccb -> circ_state & CIR_TERM) ? UPD_CRSTRT : UPD_RSTRT);
		    chanst_upd (lci, cause, TRUE);
		}
	    }
	    else {
		if (ccb -> lc_vc)
		    clr_done (ccb, NULL, RSTRR_PKT);
		else
		    rset_done (ccb, NULL, RSTRR_PKT);
		if (ccb -> circ_state & CIR_ACTV)
		    chanst_upd (lci, UPD_RESET, TRUE);
	    }
	    /*
	     Save channel open state, turn off xmit and active bits.
	     */
	    ccb -> circ_state &= CIR_OPEN;
	}
    }
    return;
}

/* ------------------------------------------------------------------------
   diag_err is called by packin software when an input error is detected.  The
   received packet must be discarded by the caller.  As a DCE this function
   will send to the local DTE a Restart packet, with the given cause; but a
   DTE will give an error indication to the application (for typing).
------------------------------------------------------------------------ */
diag_err (ccb, pkt, code)
CCB_PTR ccb;
char    pkt[];
byte code; {
    if (lcb.driver_mode != PDTE_MODE) {
	diag_send (code, pkt);
    }
    log_err (MS_INFO + ccb -> x_lci, MO_PACK, MP_DIAGDCE, pkt);
    return;
}

/* ------------------------------------------------------------------------
   diag_send is called by diag_err when an input error is detected.  A DCE
   will send to the local DTE a Diagnostic packet, with the given cause;
------------------------------------------------------------------------ */
diag_send (code, expl)
char    code,
	expl[];
{
    extern QUE_PTR      get_que();
    QUE_PTR             que;
    BUF_PTR             buf;

    int_off ();
    que = get_que (YES_BUFF);
    int_on ();
    if (que != 0) {
	/*
	 If we got a queue, set it up for a channel 0 message, sequence number
	 of 0, a diagnostic packet, with the code passed, and three bytes of
	 explanation.
	 Then tell pack_write to send it.
	 */
	que -> u.cmd.p_gfilci = XPC_WRITE;
	que -> u.cmd.p_seqenc = 0;
	que -> u.cmd.p_id = DIAG_PKT;
	buf = que -> u.cmd.bfirst;
	buf -> num_bytes = B_BYTE0 + 5;
	buf -> data[B_BYTE0 + 1] = code;
	buf -> data[B_BYTE0 + 2] = expl[0];
	buf -> data[B_BYTE0 + 3] = expl[1];
	buf -> data[B_BYTE0 + 4] = expl[2];
	pack_write (0, que, 5);
    }
    return;
}
