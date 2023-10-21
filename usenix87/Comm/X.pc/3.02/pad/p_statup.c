/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
			X.PC Status Updating Routines
			      file name = x_statup.c

   The routines in this module are used to update changes to link and modem
   status and settings.  Some are called directly by ASM routines that are
   attached to Async. Interrupts, others are callable by any 'C' code.

   chanst_set     Called by packet open routine as soon as a LCI is assigned.
		  Initializes pointer value used by 'upd' and 'rep' routines.

   chanst_upd     Called by packet routines that affect channel status.  The
		  routine accepts a channel number, an integer indicating which
		  status field has changed (see UPD defines below), and a
		  BOOLEAN value for the field indicated.

   chanst_rep     Called by packet routines interested in channel status.  The
		  routine accepts a channel number, and returns an integer
		  whose left byte is the recv status, and whose right half is
		  the transmit status for that logical channel.

   chanst_chk     Called by packet routines that have gotten a packet in on a
		  channel not opened.  Checks the PAD layer for channels that
		  are waiting for an incoming call, returns PAD chan if found.

   rpt_pkterr     Called by the Driver to cause the statistics stored by the
		  packet layer to be report to the application.

   -------------------------------------------------------------------------
	 Date    Change  By      Reason
      05/27/84    01    curt     Initial generation of code.
      06/30/84    02    curt     Converted logch_map to CHCB_PTR array.
      07/12/84    03    curt     Added 'chanst_chk' logic for incoming calls.
      07/24/84    04    curt     Added 'UPD_RESET/RSTRT' to chanst_upd call.
      05/18/85    05    curt     Put in stub for packet error report routine.
      09/26/86    06    Ed M     Reformat and Recomment
   ------------------------------------------------------------------------- */
#include "driver.h"
#include "pad.h"


/* -------------------------------------------------------------------------
   This routine maps the logical chan of the packet layer into the PAD CHCB.
   ------------------------------------------------------------------------- */
void chanst_set (log_chan, PAD_chan)
int     log_chan;
int     PAD_chan;
{
    dcb.logch_map[log_chan] = &dcb.ch[PAD_chan];
    dcb.ch[PAD_chan].pkt_chan = log_chan;
    return;
}



/* -------------------------------------------------------------------------
   This routine accepts a logical channel number, an indication of which status
   field within the channel's Pad control block to update, and the new BOOLEAN
   value to put into the field.
   ------------------------------------------------------------------------- */
void chanst_upd (log_chan, which_stat, new_value)
int     log_chan;
int     which_stat;
int     new_value;
{
    int     cause;
    if (dcb.logch_map[log_chan] != NULL) {
	switch (which_stat) {
	    case UPD_RECV: 
		dcb.logch_map[log_chan] -> recv_ready = new_value;
		break;
	    case UPD_XMIT: 
		dcb.logch_map[log_chan] -> xmit_enble = new_value;
		break;
	    default: 
		cause = which_stat + (CLR_RESET - UPD_RESET);
	    /* 
	     CLR_RESET    = UPD_RESET    + 7
	     CLR_RESTART  = UPD_RESTART  + 7
	     CLR_CRESTART = UPD_CRESTART + 7
	     CLR_PKTLOST  = UPD_PKTLOST  + 7
	     CLR_MDMLOST  = UPD_MDMLOST  + 7    
	     */
	    /* if (dcb.logch_map[log_chan]->call_state != DISCONNECT) { */
		dcb.logch_map[log_chan] -> call_state = CALL_CLEARED;
		dcb.logch_map[log_chan] -> clear_code = cause;
	    /* }  remove test for not disconnected */
	}
    }
}



/* -------------------------------------------------------------------------
   This routine accepts a logical channel number.  It returns a word that has
   the receive and transmit status values for that channel: the left byte has
   the receive status, the right byte has the transmit status.
   ------------------------------------------------------------------------- */
int     chanst_rep (log_chan)
int     log_chan;
{
    int     ret_value;

    ret_value = 0;
    if (dcb.logch_map[log_chan] != NULL)
	ret_value = (dcb.logch_map[log_chan] -> recv_ready << 8) |
	    dcb.logch_map[log_chan] -> xmit_enble;
    return (ret_value);
}



/* -------------------------------------------------------------------------
   This routine will be called by the packet layer to ask for the PAD channel
   number of a channel waiting for an incoming call.  If there are no channels
   waiting for a call, the routine returns -1.
   ------------------------------------------------------------------------- */
int     chanst_chk (chan_type)
{
    int     i;
    for (i = 1; i <= 15; i++)
	if (dcb.ch[i].call_state == INCOMCAL_PND)
	    break;

    return ((i > 15) ? (-1) : i);
}


/* -------------------------------------------------------------------------
   Errors recorded by the packet layer to the application.  The application
   gives the address of an area for a copy of this structure.
   ------------------------------------------------------------------------- */

typedef struct err_type {
    int     error_cnt;
}                       ERROR_STR, *ERR_PTR;
ERROR_STR errors;

void rep_pkterr (user_seg, user_off)
int    *user_seg;
int    *user_off;
{
    extern void mvbyt_to();

    mvbyt_to (user_seg, user_off, &errors, sizeof (ERROR_STR));
}
