/****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"
int     call_time (), clr_time ();
/* ------------------------------------------------------------------------
   08/27/84  Mike  Eliminate duplicate timeout entries using strt/stop_time
   11/28/84  Mike  Correct number of arguements in clr_conf
   11/28/84  Mike  fix timeout logic to try again
   09/26/86  Ed M  Reformat and re-comment, no code changed
------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------
   pkt_state is called in state d1 as a switched virtual circuit (PVC),
------------------------------------------------------------------------- */
pkt_state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    switch (ccb -> p_state) {
	case P1:
	    p1state (ccb, que);
	    break;
	case P2:
	    p2state (ccb, que);
	    break;
	case P3:
	    p3state (ccb, que);
	    break;
	case P4:
	    p4state (ccb, que);
	    break;
	case P5:
	    p5state (ccb, que);
	    break;
	case P6:
	    p6state (ccb, que);
	    break;
	case P7:
	    p7state (ccb, que);
    }
    return;
}

/* ------------------------------------------------------------------------
   ------------------------------------------------------------------------
   P1state is the normal ready (idle) state; Call or Clear will take us out.
------------------------------------------------------------------------- */
p1state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    return;
}

/* ------------------------------------------------------------------------
   P2state is entered when DTE sends a Call, and exited when DCE confirms.
   Note: DCE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
p2state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    return;
}

/* ------------------------------------------------------------------------
   P3state is entered when DCE sends a Call, and exited when DTE confirms.
   Note: DTE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
p3state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    return;
}

/* ------------------------------------------------------------------------
   P4state is the normal data transfer state; only Clear will take us out.
------------------------------------------------------------------------- */
p4state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    switch (ccb -> in_pktindex) {
	case CALLR_TYP:
	    break;
	case CALLC_TYP:
	    break;
	case CLRR_TYP:
	    break;
	case CLRC_TYP:
	    break;
	default:
	    data_state (ccb, que);
    }
    return;
}

/* ------------------------------------------------------------------------
   P5state is entered when DTE and DCE send a Call request/indication
   Note: Call collision requires a Clear
------------------------------------------------------------------------- */
p5state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    return;
}

/* ------------------------------------------------------------------------
   P6state is entered when DTE sends a Clear, and exited when DCE confirms.
   Note: DCE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
p6state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    return;
}

/* ------------------------------------------------------------------------
   P7state is entered when DCE sends a Clear, and exited when DTE confirms.
   Note: DTE must allow time for RR or REJ of Confirm packet.
------------------------------------------------------------------------- */
p7state (ccb, que)
CCB_PTR ccb;
QUE_PTR que;
{
    return;
}

/* ------------------------------------------------------------------------
   clr_send is called by clrr_err when an input error is detected.
   will send to the local DXE a Clear Packet, with the given cause and code;
------------------------------------------------------------------------ */
void clr_send (lci, cause, code)
byte    lci,			/* logical channel to send it on */
	cause,			/* clearing cause */
	code;			/* diagnostic code */
{
    QUE_PTR que;
    QUE_PTR get_que();
    BUF_PTR buf;
    if (lcb.ccbp[lci]) {
	clr_done (lcb.ccbp[lci], NULL, CLRR_PKT);
	int_off ();
	que = get_que (YES_BUFF);
	int_on ();
	if (que != 0) {
	    que -> u.cmd.p_gfilci = XPC_WRITE;
	    que -> u.cmd.p_id = CLRR_PKT;
	    buf = que -> u.cmd.bfirst;
	    buf -> num_bytes = B_BYTE0 + 3;
	    buf -> data[B_BYTE0 + 1] = cause;
	    buf -> data[B_BYTE0 + 2] = code;
	    pack_write (lci, que, 3);
	}
    }
    return;
}

/* ----------------------------------------------------------------------
   call_time is called by timeout when the DXE does not respond within T21
   seconds to a call request packet within.  Retry R21 times, then quit.
--------------------------------------------------------------------------- */
call_time (lci)
int     lci;
{
    extern void         rel_que(),
			link_write();
    extern QUE_PTR      fid_cmd();
    CCB_PTR             ccb;
    QUE_PTR             que;
    /*
    If we have a valid logical channel, clear the timer bit to start over.
    If we can get the command on the retransmit queue, check if the retry
    count has been reached.  If so, get rid of the call request, and send a
    clear.  Otherwise, set up to retransmit the request.
    */
    if (ccb = lcb.ccbp[lci]) {
	ccb ->  time_run &= ~TIM_CALL;
	int_off ();
	if ((que = fid_cmd (&ccb -> hold_que, lci, CALLR_PKT)) != NULL) {
	    int_on ();
	    if (--ccb -> retry20[1] == 0) {
		/*
		Retry count has been reached.  Set b.state to make pk_start
		reset the retry count, release request, and send the clear.
		*/
		que -> u.cmd.b_state &= ~B_STRTD;
		int_off ();
		rel_que (que, YES_BUFF);
		int_on ();
		call_err (ccb, CLR_LOCPE, 49);
	    }
	    else {
		/*
		Tell the machine it's a retransmission, set up for retran. and
		restart the timer.
		*/
		que -> u.cmd.b_state = (que -> u.cmd.b_state & ~B_STRTD) | B_RETRAN;
		link_write (LWRIT_OUT, que);
		int_off ();
		strt_time (T21 * EVNT_PSEC, call_time, lci, TIM_CALL);
	    }
	}
	int_on ();
    }
    return;
}

/* ----------------------------------------------------------------------
   clr_time is called by timeout when the DXE does not respond within T23
   seconds to a clear request packet.  Retry R23 times, then quit.
--------------------------------------------------------------------------- */
clr_time (lci)
int     lci;
{
    extern void    link_write();
    extern QUE_PTR fid_cmd();
    CCB_PTR	   ccb;
    QUE_PTR	   que;
    static char empty[3] = {0, 0, 0};

    /*
    Check for valid logical channel.  If so, clear out timer running bit,
    and see if the clear request is in the hold queue.
    */
    if (ccb = lcb.ccbp[lci]) {
	ccb ->  time_run &= ~TIM_CLR;
	int_off ();
	if ((que = fid_cmd (&ccb -> hold_que, lci, CLRR_PKT)) != NULL) {
	    int_on ();
	    if (--ccb -> retry20[3] == 0) {
		/*
		Retry count has expired, set b_state to make pk_start reset the
		count, log the error, sending a clear request, and start up the
		timer.
		*/
		que -> u.cmd.b_state &= ~B_STRTD;
		empty[1] = lci;
		diag_err (ccb, empty, 50);
		int_off ();
		strt_time (T23 * EVNT_PSEC, clr_time, lci, TIM_CLR);
	    }
	    else {
		/*
		Flag as a retransmission, and restart the timer.
		*/
		que -> u.cmd.b_state = (que -> u.cmd.b_state & ~B_STRTD) | B_RETRAN;
		link_write (LWRIT_OUT, que);
		int_off ();
		strt_time (T23 * EVNT_PSEC, clr_time, lci, TIM_CLR);
	    }
	}
	int_on ();
    }
    return;
}

/* ------------------------------------------------------------------------
   call_conf is called by r2state or r3state when either a restart request or
   a restart confirm is received.
------------------------------------------------------------------------ */
call_conf (ccb)
CCB_PTR ccb;
{
    int_off ();
    stop_time (call_time, ccb -> x_lci, TIM_CALL);
    ccb -> retry20[0] = 0;
    int_on ();
    rel_reqst (ccb, CALLR_PKT);
    set_state (ccb, P4);
    return;
}

/* ------------------------------------------------------------------------
   clr_conf is called by r2state or r3state when either a restart request or
   a restart confirm is received.
------------------------------------------------------------------------ */
clr_conf (ccb, que)
CCB_PTR ccb;
QUE_PTR que;                    /* real que = 0,        if restart sent */
{
    int_off ();
    stop_time (clr_time, ccb -> x_lci, TIM_CLR);
    ccb -> retry20[0] = 0;
    int_on ();
    rel_reqst (ccb, CLRR_PKT);
    set_state (ccb, R1);
    return;
}
/* #define UPD_CLEAR 0x?? */
/* ------------------------------------------------------------------------
   clr_done is called by rstr_done when a restart request is received (recv=1),
   or its output is completed (recv=0), to clear a virtual call channel.
------------------------------------------------------------------------ */
clr_done (ccb, que, pack)
CCB_PTR ccb;                    /* real ccb */
QUE_PTR que;                    /* real que = 0,        if restart sent */
int     pack;                   /* packet is RSTRR_PKT or CLRR_PKT */
{

    rset_done (ccb, que, pack);
    ccb -> circ_state |= CIR_CLR;
    if (ccb -> circ_state & CIR_ACTV) {
	/*
	If circuit is active, and packet is a clear request, do nothing
	until clear handling code is added.
	*/
	if (pack == CLRR_PKT)
	    nothing;
/*      chanst_upd (ccb->x_lci, UPD_CLEAR, TRUE);  */
    }
    set_state (ccb, P1);
    return;
}

/* ------------------------------------------------------------------------
  call_err is called by call software when an input error is detected.
   The packet must be discarded by the caller.  As a DCE this function will
   send to the local DTE a Restart packet, with "local procedure error" cause.
------------------------------------------------------------------------ */
call_err (ccb, cause, code)
CCB_PTR ccb;
int     cause,                  /* Clear cause  */
	code;                   /* diagnostic code */
{
    int     i,
	    state;
    static char zero[] = {0, 0, 0};

    i = cause;
    /*
    If driver is in DTE mode, change cause to local procedure error, and
    state to P6.  Otherwise, set state to P7.
    */
    if (lcb.driver_mode == PDTE_MODE) {
	i = CLR_LOCPE;
	state = P6;
    }
    else {
	state = P7;
    }
    /*
    Send a clear packet, stash the channel number and report an error.
    */
    clr_send (ccb -> x_lci, i, code);
    set_state (ccb, state);
    zero[1] = ccb -> x_lci;
    log_err ((MS_INFO << 4) + ccb -> x_lci, MO_PACK, MP_CLRSENT, zero);
    return;
}
