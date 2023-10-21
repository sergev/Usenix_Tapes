/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		Packet Layer/PAD Interface Routines.
			filename = p_pkcomm.c

   strt_chrmode Sets up packet layer in character mode, will call link_init
		to enable hardware input/output interrupts.
   term_chrmode Turns off packet layer's character mode processing,
		calls link_term to disable hardware input/output interrupts.

   strt_pktmode Sets up packet layer in packet mode, will call link_init
		to enable hardware input/output interrupts.
   term_pktmode Sets up packet layer in packet mode, will call link_init
		to enable hardware input/output interrupts.

   pack_open    called to setup input/output on a channel, will call link_open
		to turn on packet processing.  First parameter is either -1 or
		is a already-known Logical chan #.  Also needs the PAD channel
		number used (setup channel status pointers), and channel type.
   pack_close   called to shut down input/output on a channel, will call
		link_close to turn off packet processing.
   pack_read    accepts a channel number, and returns the first queued packet
		from that logical channel's input que (returns NULL if no pkt).
   pack_write   accepts a channel number, a que entry, and a packet size.  Puts
		the packet pointed to by que entry in the channels output que.

   assign_ccb   accepts a channel type (from PVC, VC, DATAGRAM), and returns
		either a position in the ccb pointer array of the lcb (as LCI),
		or returns NO_RESOURCE (no channels available of that type).
   check_ccb    accepts a channel number and type (from PVC, VC, DATAGRAM), and
		returns the channel number (OK to put call in that LCI), or it
		returns NO_RESOURCE (no channels available of that type).
   send_pktterm Sends Restart ending packet mode (PAD layer must wait, then...
   -------------------------------------------------------------------------
     Date     By   Reason
   05/24/84  Curt  Initial Generation of file.
   06/13/84  Curt  Combined 'p_pckcom' and 'p_pkinit' into 1 file.
   07/12/84  Curt  Changed 'pack_open' to work for incoming calls.
   07/18/84  Curt  Moved init_pktlyr to p_lcbdcl.c
   07/18/84  Mike  Moved chan_enable, disable to packgen.c
   07/26/84  Mike  If restart, pack_open must prevent data xfer
   08/01/84  Mike  Correct open test for restart, zero more of ccb
   08/09/84  Mike  Fix pack_open setting CIR_OPEN bit in circ_state
   08/14/84  Mike  send_ok now requires qued/qall_out parameter
   08/23/84  Mike  try_send now has in_off inside, not in call
   08/23/84  Mike  turn interrupts off/on around end_time in term_pktmode
   08/27/84  Mike  Eliminate duplicate timeout entries using strt/stop_time
   10/11/84  Mike  End RNR state as soon as application reads a packet
   10/23/84  Mike  Init Restart packet: DCE no send, DTE delay before send
   11/01/84  Mike  Correct stop_time number of arguments
   11/16/84  Mike  Correct pack_read setting chanst_update's receive to FALSE
   12/19/84  Mike  pack_write test of SEN_TIME and SEN_RTRN moved to send_ok
   10/01/86  Ed M  Reformat and recomment.
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "pkt.h"
#include "lcbccb.h"

#define  NO_RESOURCE (-1)       /* returned by assign_ccb when no chans. */
#define  CHAN_TYPE   0x0C       /* mask with bits 2 and 3 on, to get type */
#define  LCI_UNKNOWN (-1)       /* input to pack_open when not incomecall */

extern  rr_time (), rnr_time ();/* timer-driver packet layer routine. */



/* -------------------------------------------------------------------------
   This routine is called by the PAD to start up character mode.  The PAD
   will have initialized the 8250.  This routine calls the link I/O enabler.
   ------------------------------------------------------------------------- */
void strt_chrmode ()
{
    extern void link_init();

    lcb.driver_mode = CHAR_MODE;
    if (pack_open (LCI_UNKNOWN, CHAR_CHAN, CHAR_CHAN) == CHAR_CHAN)
	link_init ();
    return;
}



/* -------------------------------------------------------------------------
   This routine is called by the PAD to terminate character mode, it must
   disable the link layer's input/output logic, then disconnect the channel.
   ------------------------------------------------------------------------- */
void term_chrmode ()
{
    extern void link_term();

    link_term ();
    pack_close (CHAR_CHAN);
    lcb.driver_mode = DEAD_MODE;
    return;
}



/* -------------------------------------------------------------------------
   This routine is called by the PAD to start up packet mode.  To start packet,
   try to open the character channel, used as system manager channel by packet
   layer.  If cannot open, abort because big trouble if no system manager. PAD
   will have initialized the 8250, so this routine calls the link I/O enabler.
   ------------------------------------------------------------------------- */
void strt_pktmode (mode)
int     mode;
{
    extern void link_init();

    CCB_PTR ccb;
    lcb.driver_mode = mode;
    crc_gen (CRC_POLYNOM);
    if (pack_open (LCI_UNKNOWN, CHAR_CHAN, CHAR_CHAN) >= 0) {
	link_init ();
	/*
	 Set protocol control fields, and send restarts as determined
	 by DTE/DCE.
	 */
	if (ccb = lcb.ccbp[CHAR_CHAN]) {
	    if (lcb.driver_mode == PDTE_MODE) {
		if (ccb -> r_state < R2) {
		    rstr_send (RST_DTEORG, 0);
		    set_state (ccb, R2);
		}
	    }
	    else {
	    /* rstr_send (RST_NETOPR,0);        *  DCE waits for restart */
		set_state (ccb, R0);
	    }
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine is called by the PAD to terminate packet mode.  This routine
   must disable the link layer's input/output processing.
   ------------------------------------------------------------------------- */
void term_pktmode ()
{
    extern void ccb_init(),
		link_term();
    CCB_PTR ccb;
    if (ccb = lcb.ccbp[CHAR_CHAN]) {
	if (lcb.driver_mode == PDTE_MODE) {
	    rstr_send (RST_DTEORG, 241);
	    set_state (ccb, R2);
	}
	else {
	    rstr_send (RST_NETCNG, 241);
	    set_state (ccb, R3);
	}
	/*
	Close the link's chan, then close the packet chan, set driver mode.
	End all timeouts on channel 0, clear out all packets and queues, and
	clear out the ccb.
	*/
	int_off ();
	stop_time (0, 0, 0xff); 
	int_on ();
	end_ques (0, RSTRC_PKT + 1);
	ccb_init (ccb);         
	link_term ();
	pack_close (CHAR_CHAN);
	lcb.driver_mode = DEAD_MODE;
    }
    return;
}



/* -------------------------------------------------------------------------
   Pack_open is called by application server to OPEN a channel for I/O, or
   by the packet layer to handle an incoming call from the remote system.
   'new_lci' is -1 if the Logical Channel number is to be determined by the
   open (using options), or is 1-15 if an incoming call specified the LCI.
   'pad_chan' is the channel number used by the PAD to access, the value is
   only needed to 'call back' to the PAD to inform of the LCI assigned.
   When options == zero, need to init the character channel for char mode, or
   the system channel for packet mode.  When options != zero, is a PVC or VC
   channel for packet mode.  Init timer control when open system chan, pktmode.
   ------------------------------------------------------------------------- */
int     pack_open (new_lci, pad_chan, options)
int     new_lci,
	pad_chan,
	options;
{
    extern CCB_PTR get_ccbavl();
    extern void    link_open(),
		   chanst_set(),
		   chanst_upd();

    int     lci;
    CCB_PTR ccb;
 /*
 If user doesn't know LCI, assign one.  If knows, check if OK.  Both
 routines return the logical chan number of the CCB array that the
 channel is assigned. If found a slot that could put application into,
 try to get a ccb structure pointer to use.  If no ccb structures left,
 return error to the caller.    
 */
    if (new_lci < 0)
	lci = assign_ccb (options);
    else
	lci = check_ccb (new_lci, options);
    if (lci != NO_RESOURCE) {
	lcb.ccbp[lci] = get_ccbavl ();
	if (lcb.ccbp[lci] == NULL)
	    lci = (NO_RESOURCE);
    }
    if (lci != NO_RESOURCE) {
	/*
	First: tell the PAD the LCI assigned, so can do channel updating using
	the 'chanst_upd' routine as soon as want to. Initialize the ccb
	assigned to the channel, and setup protocol control fields.
	*/
	chanst_set (lci, pad_chan);
	if (ccb = lcb.ccbp[lci]) {
	    ccb -> x_lci = lci; 
	    chan_enable (lci);
	    link_open (lci, options);
	    ccb -> circ_state = CIR_OPEN;
	    if ((lcb.driver_mode & PACK_FLAG) != 0) {
		ccb -> lc_vc = (options & VC_CHANNEL);
		ccb -> lc_2way = (options & TWO_CALLS);
		ccb -> r_state = lcb.ccbp[0] -> r_state;
		ccb -> p_state = ccb -> d_state = ccb -> cur_state = 0;
		if ((lcb.ccbp[0] -> circ_state & CIR_RSTR) != 0)
		    set_state (ccb, (ccb -> lc_vc == 0) ? D1 : P1);
		else
		    chanst_upd (lci, UPD_XMIT, FALSE);
	    }
	}
	chanst_upd (lci, UPD_RECV, FALSE);
    }
    return (lci);
}



/* -------------------------------------------------------------------------
   Pack_close is called by application server when a channel is already
   cleared (CLEAR REQEST or CLEAR CONFIRM sent).  Disable I/O for the chan.
   ------------------------------------------------------------------------- */
void pack_close (lci)
int     lci;                    /* log chan # */
{
    extern void rel_ccbavl(),
		link_close();

    CCB_PTR ccb;

    if (ccb = lcb.ccbp[lci]) {
	link_close (lci);
	chan_disable (lci, CALLR_PKT);
	rel_ccbavl (ccb);
	lcb.ccbp[lci] = NULL;
    }
    return;
}



/* -------------------------------------------------------------------------
   Pack_read is called by PAD, when data is available to be read.
   ------------------------------------------------------------------------- */
QUE_PTR pack_read (lci)
int     lci;
{
    extern QUE_PTR deque();
    extern void    chanst_upd();

    CCB_PTR ccb;
    QUE_PTR qentry;
    if (ccb = lcb.ccbp[lci]) {
	int_off ();
	if (ccb -> read_que.ent_cnt)
	    qentry = deque (&ccb -> read_que);
	else
	    qentry = NULL;
	int_on ();
	if (qentry != NULL) {
	    ccb -> qued_in -= 1;
	    ccb -> circ_state |= CIR_ACTV;
	    if ((qentry -> u.cmd.p_gfilci & C_BIT) == 0)
		qentry -> u.cmd.p_id = NORMAL_DATA;
	    /*
	    Cannot change ccb's next_reply from here, since we were not called
	    by pack_in (but by applicat. task).  Thus, we must call rep_send
	    with RR.  But, we must have both possible conditions cleared up:
	    namely, RNR_XPC says buf/que low, RNR_WIN says window over filled
	    */
	}
	int_off ();
	if (ccb -> recv_state & REP_RNR) {
	    if (rnr_still (ccb, ccb -> recv_state) == 0) {
		stop_time (rnr_time, lci, TIM_RNR);
	    /* int_on ();  deleted to prevent state change int_off (); */
		strt_time (lcb.htime << 2, rr_time, lci, TIM_RR);
		int_on ();
		rep_send (lci, RR_PKT, ccb -> vr);
	    }
	}
	int_on ();
	if ((lcb.driver_mode & PACK_FLAG) != 0) {
	    int_off ();
	    if (ccb -> qued_in == 0)
		chanst_upd (lci, UPD_RECV, FALSE);
	    int_on ();
	}
    }
    return (qentry);
}



/* -------------------------------------------------------------------------
   Pack_write is called by the application to write packets out.  It is also
   called by the packet layer to write certain control packets.
   ------------------------------------------------------------------------- */
void pack_write (lci, qentry, size)
QUE_PTR qentry;
int     lci,
	size;
{
    extern QUE_PTR deque();
    extern void    enque(),
		   link_write(),
		   chanst_upd();

    CCB_PTR ccb;
    int     count;

    qentry -> u.cmd.b_state = B_QUED;
    if (ccb = lcb.ccbp[lci]) {
	if ((lcb.driver_mode & PACK_FLAG) != 0) {
	    ccb -> qall_out += 1;
	    ccb -> circ_state |= CIR_ACTV;
	    if (qentry -> u.cmd.p_gfilci == XPC_WRITE)
		qentry -> u.cmd.p_gfilci = C_BIT;
	    else {
		qentry -> u.cmd.b_state += B_APPLIC;
		if (qentry -> u.cmd.p_id == NORMAL_DATA) {
		    ccb -> qued_out += 1;
		    count = ccb -> qued_out;
		}
		else {
		    qentry -> u.cmd.p_gfilci |= C_BIT;
		    count = ccb -> qall_out;
		}
		int_off ();
		if ((send_ok (ccb, qentry, count) == FALSE) || (lcb.xpc_nrdy))
		    count = FALSE;
		else
		    count = TRUE;
		chanst_upd (lci, UPD_XMIT, count);
		int_on ();
	    }
	    qentry -> u.cmd.p_gfilci += lci;
	    qentry -> u.cmd.bfirst -> data[B_SIZE] = size - 1;
	    qentry -> u.cmd.bfirst -> data[B_GFLCI] = qentry -> u.cmd.p_gfilci;
	    qentry -> u.cmd.bfirst -> data[B_PKTID] = (qentry -> u.cmd.p_id == NORMAL_DATA) ?
		qentry -> u.cmd.bfirst -> data[B_BYTE0] : qentry -> u.cmd.p_id;
	    if ((size > 1) && ((qentry -> u.cmd.b_state & B_RETRAN) == 0))
		crc2_gen (ccb, qentry);

	    int_off ();
	    enque (&ccb -> writ_que, qentry);
	    int_on ();
	    try_send (ccb);
	}
	else {
	    ccb -> qued_out += 1;
	    int_off ();
	    enque (&ccb -> writ_que, qentry);
	    int_on ();
	    int_off ();
	    qentry = deque (&ccb -> writ_que);
	    int_on ();
	    link_write (LWRIT_OUT, qentry);
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   These two routines are called by "assign_ccb" routine.  Caller sets up
   first and last params, then calls search routine. One searches forward
   (first to last), the other backward (first to last).
   ------------------------------------------------------------------------- */
int     frst_forw (first, last)
int     first,
	last;
{
    int     ans;
    ans = NO_RESOURCE;
    if (first > 0)
	for (; first <= last; first++)
	    if (lcb.ccbp[first] == NULL)
		ans = first;
	return (ans);
}
/* -------------------------------------------------------------------------
These two routines are called by "assign_ccb" routine.  Caller sets up first
and last params, then calls search routine. One searches forward (first to
last), the other backward (first to last).
   ------------------------------------------------------------------------- */
int     frst_back (first, last)
int     first,
	last;
{
    int     ans;
    ans = NO_RESOURCE;
    if (first > 0)
	for (; first >= last; first--)
	    if (lcb.ccbp[first] == NULL)
		ans = first;
    return (ans);
}



/* -------------------------------------------------------------------------
   This routine attempts to find a position in the ccb array for a channel
   with the capabilities indicated by 'chan_option'.  It is assumed that the
   caller has already determined that the channel to be assigned is not open
   (it does not relate to another ccbp position). Thus, opens on channels being
   reopened (with RECOVR_FLG set to 1) should not invoke this routine.
   The routine returns the index into the CCBP array that the channel can use.
   ------------------------------------------------------------------------- */
int     assign_ccb (chan_option)
int     chan_option;
{
    int     found;

 /*
 Options = 0 is a special case, is used for the character channel.
 */
	/*
	all remaining channels will be > 0 if OK.
	*/
    found = (NO_RESOURCE);
    if (chan_option == 0)
	found = 0;
    else {
	/*
	Not trying to open channel 0, so use search algorithm to decide which
	ccb (logical channel position) to assign the application channel to.
	DTE PVC goes backward, DCE PVC goes forward. 
	*/
	switch (chan_option & CHAN_TYPE) {
	    case PVC_CHANNEL:
		    switch (lcb.driver_mode) {
			case PDTE_MODE:
				found = frst_back (HI_PVC, LO_PVC);
				break;
			case PDCE_MODE:
				found = frst_forw (LO_PVC, HI_PVC);
				break;
		    }
	}
    }
    return (found);
}



/* -------------------------------------------------------------------------
   This routine determines whether the incoming call that came in with the
   LCI indicated by new_lci, will be acceptable given the ranges of LCI's
   that the system is configured to support Incoming calls on.
   ------------------------------------------------------------------------- */
int     check_ccb (new_lci, options)
int     new_lci,
	options;
{
    int     result;

    switch (options & CHAN_TYPE) {
	case PVC_CHANNEL:
		if ((new_lci >= LO_PVC) && (new_lci <= HI_PVC))
		    result = new_lci;
		else
		    result = NO_RESOURCE;
		break;
    }
    return (result);
}



/* -------------------------------------------------------------------------
   This routine is called by the PAD to send a Restart terminating packet mode.
   Then the PAD layer must call "send_active" until it returns a zero, which
   means output queues are empty.  Finally, call term_pktmode.
   ------------------------------------------------------------------------- */
void send_pktterm ()
{
    CCB_PTR ccb;
    if (ccb = lcb.ccbp[CHAR_CHAN]) {
	if (lcb.driver_mode == PDTE_MODE) {
	    rstr_send (RST_DTEORG, 241);
	    set_state (ccb, R2);
	}
	else {
	    rstr_send (RST_NETCNG, 241);
	    set_state (ccb, R3);
	}
    }
    return;
}
