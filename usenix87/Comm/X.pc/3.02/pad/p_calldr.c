/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		   Virtual and Logical "CALL" logic
			 file name = p_calldr.c

   The routines in this module implement for the driver the logic needed to
   process PVC calls (Permanant Virtul Circuits).  Only the driver uses the
   routines, as a direct result of an application request.  All routines
   accept as a parameter a pointer to the channel being processed.

   chstat_rep  Reports the status of the channel, and if there are any
	       packets available that will alter the status, processes them.

   pkch_close  Called by chstat_rep when a call has been cleared, and the state
	       changed to 'disconnect'.  Stops timers, clears data structures.

   call_ctrl   Controls access to the remaining routines.

   cl_reqsnd   Formats and sends out a call request packet.

   cl_clrsnd   For non-incoming chans: Formats and sends a call clear packet.
	       For incoming chans: makes channel state 'call cleared'.

   cl_answait  Sets the channel state to 'pending an incoming call'.

   cl_accsnd   Formats and sends out a call accept packet.

   cl_dataread If in the proper state, and data available, returns the data
	       found in an accept or incoming call packet.
   -------------------------------------------------------------------------
	 Date    Change  By      Reason
      07/12/84     01   curt     Moved routines out of driver module.
      09/10/84     02   curt     Removed references to Virtual Call channels.
      11/08/84     03   curt     Added pkch_close routine.
      07/31/85     04   curt     Completion of MCI echo code.
      09/26/86     05   Ed M     Reformat and recomment
      10/20/86     06   Ed M     Fixed chstat_rep() to avoid lockup on query
				 of channel 0's status.
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "pad.h"
#include "pkt.h"

extern  io_active ();


/* -------------------------------------------------------------------------
   This routine reports the status of the channel indicated by the caller.
   If not disconnected, check queues for packets.  If the status is call
   cleared, update clear code, then close the channel. Always report state.
   ------------------------------------------------------------------------- */
void chstat_rep (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern  void prc_qbitpkt(),
		 mv_wrdto();    
    int     tcall_state;

    if (ptr_2chcb -> call_state != DISCONNECT)
	prc_qbitpkt (ptr_2chcb);
    tcall_state = ptr_2chcb -> call_state;
    if (ptr_2chcb -> call_state == CALL_CLEARED) {
	mvwrd_to (params.param_seg, params.param2, ptr_2chcb -> clear_code);
	/*
	 If check installed by Ed Mooring on 10-20-86 to get around problem
	 with channel 0 status query locking up the driver.
	 */
	if (&dcb.ch[0] != ptr_2chcb)
		pkch_close (ptr_2chcb);
	else
		ptr_2chcb -> call_state = DISCONNECT;
    }
    mvwrd_to (params.param_seg, params.param1, tcall_state);
    return;
}





/* -------------------------------------------------------------------------
   This routine performs the cleanup necessary to close a packet mode channel:
   stop all timers, reset data structures to initialized state (zeros),
   important to re-initialize the 'terminal characteristics' of the channel,
   and VERY IMPORTANT to release all queue/bufferlets held in in/out lists.
   ------------------------------------------------------------------------- */
void pkch_close (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    int     i;
    extern  void upd_ptimer(),
		 rel_que(),
		 clr_cnftmr(),
		 ball_timr(),
		 frwd_timr(),
		 zero_mem();
    extern  int  pack_close(),
		 end_time();

    ptr_2chcb -> call_state = DISCONNECT;
    int_off ();
    end_time (&upd_ptimer, ptr_2chcb);
    end_time (&io_active, ptr_2chcb);
    end_time (&ball_timr, ptr_2chcb);
    end_time (&frwd_timr, ptr_2chcb);
    end_time (&clr_cnftmr, ptr_2chcb);
    rel_qhead (&ptr_2chcb -> in_que);
    rel_qhead (&ptr_2chcb -> out_que);
    if (ptr_2chcb -> aprd_que != NULL)
	rel_que (ptr_2chcb -> aprd_que, YES_BUFF);
    if (ptr_2chcb -> apwr_que != NULL)
	rel_que (ptr_2chcb -> apwr_que, YES_BUFF);
    int_on ();
    pack_close (ptr_2chcb -> pkt_chan);
    for (i = 1; i < 16; i++)
	if (&dcb.ch[i] == ptr_2chcb)
	    break;
    dcb.logch_map[ptr_2chcb -> pkt_chan] = NULL;
    zero_mem ((char *) ptr_2chcb, sizeof (CHCB_STR));
    init_trmchar (i);
    rsrc_adjust (CLOS_CHAN);
    return;
}



/* -------------------------------------------------------------------------
   This routine verifies that the driver state is PACKET, and that the channel
   number is in the range 1-15.  All other verification is done by the routine
   that is called.  ce the
   ------------------------------------------------------------------------- */
void call_ctrl (chan_num)
int     chan_num;
{
    extern int  func_code,
		ret_code;               /* from p_driver.c */
    CHCB_PTR ptr_2chcb;

    if (dcb.device.state != PACKET)
	ret_code = ILL_CHARRQ;
    else
	if ((chan_num < 1) || (chan_num > 15))
	    ret_code = ILL_CHANNEL;
	else {
	    ptr_2chcb = &dcb.ch[chan_num];
	    switch (func_code) {
		case CALLRQ_SND: 
		    cl_reqsnd (ptr_2chcb, chan_num);
		    break;
		case CALLCL_SND: 
		    cl_clrsnd (ptr_2chcb);
		    break;
		case SET_CALLANS: 
		    cl_answait (ptr_2chcb);
		    break;
		case CALLAC_SND: 
		    cl_accsnd (ptr_2chcb);
		    break;
		case READ_CALDAT: 
		    cl_dataread (ptr_2chcb);
		    break;
	    }
	}
    return;
}



/* -------------------------------------------------------------------------
   This routine is used to send either Virtual Circuit Call Requests, or the
   Permanent Virtual Circuit Logical Call Requests (as q-bit packets).  If the
   the format indicated by the caller is 0 (string) it assumed that the chan
   will be used as a PVC and the q-bit call request is sent.  Other formats
   are related to VC channels, and the Virtcall sender is called.  If the
   routine called does not alter the ret_code, the channels state is set to
   'pending call accept' which means the call request was valid and sent.
   ------------------------------------------------------------------------- */
#define LCI_UNKNOWN (-1)

void cl_reqsnd (ptr_2chcb, pad_chan)
CHCB_PTR ptr_2chcb;
int     pad_chan;
{
    extern  void snd_qbitpkt();
    extern  int  pack_open(),
		 gtwrd_fr(),
		 ret_code;

    int     options;

    if (ptr_2chcb -> call_state != DISCONNECT)
	ret_code = ILL_CALSTAT;
    else {
	/* 
	 Must be packet state and OK channel.  Check state, else process
	 request.  Channel is disconnected, ok to attempt call request.
	 Determine PVC/VC by which format the caller specified call data was in.
	 */
	switch (gtwrd_fr (params.param_seg, params.param3)) {
	    case 0: 
		ptr_2chcb -> chan_type = options = PVC_CHANNEL;
		break;
	    default: 
		ret_code = ILL_CALFORM;
		break;
	}
	if (ret_code == FUNC_COMPLETE) {
	    ptr_2chcb -> pkt_chan = pack_open (LCI_UNKNOWN, pad_chan, options);
	    if (ptr_2chcb -> pkt_chan > 0) {
		rsrc_adjust (OPEN_CHAN);
		snd_qbitpkt (ptr_2chcb, QCALL_REQ);
		ptr_2chcb -> call_state = CALLACC_PND;
	    }
	    else {
		ptr_2chcb -> chan_type = 0;
		ret_code = NO_CHANSAVAIL;
	    }
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   Provided the state of a channel is not 'disconnected', or 'call cleared',
   this routine will let the application clear (close) a channel.  It uses
   the channel type setup by the call request to tell whether to send a logical
   q-bit packet with the clear flag, or a Virtual Circuit Clear Request.
   If packet sent OK, channel state changes to 'clear confirm pending'.
   When channel is 'INCOMCAL_PND', use call clear to abort, when no call came.
   ------------------------------------------------------------------------- */
void cl_clrsnd (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void snd_qbitpkt();
    extern int  ret_code;

    if ((ptr_2chcb -> call_state == DISCONNECT)
	    || (ptr_2chcb -> call_state == CALL_CLEARED))
	ret_code = ILL_CALSTAT;
    else {
	if (ptr_2chcb -> call_state == INCOMCAL_PND) {
	    ptr_2chcb -> call_state = CALL_CLEARED;
	    ptr_2chcb -> clear_code = CLR_NORMAL;
	}
	else {
	    snd_qbitpkt (ptr_2chcb, QCALL_CLR);
	    if (ret_code == FUNC_COMPLETE)
		ptr_2chcb -> call_state = CLEARCNF_PND;
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine just sets the channels call state to 'Incoming Call Pending',
   but only when the channel is in a disconnected state.
   ------------------------------------------------------------------------- */
void cl_answait (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern int ret_code;

    if (ptr_2chcb -> call_state != DISCONNECT)
	ret_code = ILL_CALSTAT;
    else {
	ptr_2chcb -> call_state = INCOMCAL_PND;
	rsrc_adjust (OPEN_CHAN);
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine checks the channel status, and if all is OK, sends out an
   accept packet for the channel.  If send accept OK, sets channel CONNECTED.
   ------------------------------------------------------------------------- */
void cl_accsnd (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void snd_qbitpkt();
    extern int  ret_code;

    if (ptr_2chcb -> call_state == ACCEPT_PND) {
	/* 
	 Channel has received and accepted a call request, send out accept pkt.
	 */
	snd_qbitpkt (ptr_2chcb, QCALL_ACC);
	if (ret_code == FUNC_COMPLETE)
	    ptr_2chcb -> call_state = CONNECTED;
    }
    else
	ret_code = ILL_CALSTAT; /* wrong state for call accept. */
    return;
}



/* -------------------------------------------------------------------------
   This routine assumes that the 'aprd_que' entry is pointing to an incoming
   packet that is either: a call request packet sent on an incoming call
   channel, or, a call accept packet that the remote sent in response to a
   call request packet sent.  It processes the data found (if any), and sets
   the packet channels state to connected.
   ------------------------------------------------------------------------- */
void cl_dataread (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern  int mv_pkt2buf(),
		mv_wrdto(),
		gtwrd_fr(),
		ret_code;
    int     amt_moved = 0;


    if ((ptr_2chcb -> call_state == CALL_ACCEPT)
	    || (ptr_2chcb -> call_state == INCOME_CALL)) {
	/* 
	 First position of data is in the first byte of frame trailer portion.
	 If 'mv_pkt2buf' doesn't throw away aprd_que entry, means not enough
	 room in buffer for all of data.  If non-NULL que entry, leave for next
	 read.  If does throw away, got all data read, change state to CONNECTED
	 for CALL_ACCEPT state channels, to ACCEPT_PND for INCOME_CALL channels.
	 */
	if ((ptr_2chcb -> aprd_que -> u.cmd.p_gfilci & Q_BIT) == Q_BIT) {
	    amt_moved = mv_pkt2buf (ptr_2chcb, params.param_seg, params.param1,
		    gtwrd_fr (params.param_seg, params.param2));
	    if (ptr_2chcb -> aprd_que == NULL)
		if (ptr_2chcb -> call_state == CALL_ACCEPT)
		    ptr_2chcb -> call_state = CONNECTED;
		else
		    ptr_2chcb -> call_state = ACCEPT_PND;
	}
	mvwrd_to (params.param_seg, params.param3, amt_moved);
	mvwrd_to (params.param_seg, params.param4, 0);
    }
    else
	ret_code = ILL_CALSTAT;
    return;
}
