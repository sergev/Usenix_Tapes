/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		User Event Starting and Processing Routines
			   file name = p_apevnt.c

   This module includes routines that provide 'event-oriented' functions to
   the applications, including: start and stopping timers, routines run from
   timers, data updates made on a timer, and updates on modem signal changes.

			      Module contents

   set_intrup  accepts chan ^.  Processes a 'set intrup' call for driver.
   rst_intrup  accepts chan ^.  Processes a 'reset intrup' call for driver.
   set_update  accepts chan ^.  Processes a 'set update' call for driver.
   rst_update  accepts chan ^.  Processes a 'reset update' call for driver.
   upd_ptimer  accepts chan ^.  Run by timer logic when timer done.
   -------------------------------------------------------------------------
	 Date   Change   By      Reason
      07/05/84    01    curt     Initial Generation of Code.
      09/12/84    02    curt     Added modem signal change functions.
      10/06/84    03    curt     Added checkpoint functions.
      11/19/84    04    curt     Fixed pointer bug in checkpoint function.
      09/26/86    05    Ed M     Reformat and recomment
      10/22/86    06    Ed M     Fix bug in modem status update reported by
				 Wes Kranitz
   ------------------------------------------------------------------------- */
#include "driver.h"
#include "pad.h"



/* -------------------------------------------------------------------------
   This routine will update an address in the user's data area with FALSE.
   ------------------------------------------------------------------------- */
void upd_ptimer (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void mvwrd_to();

    if (ptr_2chcb -> seg_timupd != NULL)
	mvwrd_to (ptr_2chcb -> seg_timupd, ptr_2chcb -> off_timupd, FALSE);
    ptr_2chcb -> seg_timupd = ptr_2chcb -> off_timupd = NULL;
    return;
}



/* -------------------------------------------------------------------------
   This routine enables an 'update' oriented event-code driver function.
   The event-code is specified in 'params.param1', each event has data params.
   ------------------------------------------------------------------------- */
void set_update (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void mvwrd_fr(),
		mvwrd_to();
    extern int  gtwrd_fr(),
		set_time(),
		ret_code;               /* from p_driver.c */
    int         time_length; 

    if (dcb.device.port_numb == 0)
	ret_code = ILL_WAITRQ;
    else {
	switch (gtwrd_fr (params.param_seg, params.param1)) {
	    /*
	     This starts a timer for the application channel being serviced.
	     If timer is running, return an error code: only one allowed/chan.
	     If in disconnect state, no timers allowed: all other states OK.
	     If no timer running, try to start one.  If couldn't start one, no
	     queue entries.  If could start one, run for the application. If
	     wise-acre wants negative or more than 30 minutes, return error.
	     */
	    case TIMER_DONE: 
		if (ptr_2chcb -> seg_timupd != NULL)
		    ret_code = TIMER_RUNNING;
		else
		    if (ptr_2chcb -> call_state == DISCONNECT)
			ret_code = ILL_CALSTAT;
		    else {
			int_off ();
			time_length = gtwrd_fr (params.param_seg, params.param3);
			if ((time_length <= 0)
				|| (time_length > 30 * (60 * (18 / EVNT_PSEC)))
				|| (set_time (time_length, &upd_ptimer, ptr_2chcb) == FALSE)) {
			    ptr_2chcb -> seg_timupd = ptr_2chcb -> off_timupd = NULL;
			    ret_code = NO_RESOURCES;
			}
			else {
			    ptr_2chcb -> seg_timupd = params.param_seg;
			    mvwrd_fr (params.param_seg, params.param2, &ptr_2chcb -> off_timupd);
			}
			int_on ();
		    }
		break;
	    /* 
	     Copy in the address of the word that the application wants the
	     modem status register contents written to when a signal changes.
	     Just so user knows initial value, output current register contents.
	     */
	    case MODEM_CHG: 
		dcb.seg_mdmupd = params.param_seg;
		mvwrd_fr (params.param_seg, params.param2, &dcb.off_mdmupd);
		mvwrd_to (dcb.seg_mdmupd, dcb.off_mdmupd, rep_mdst ());
		break;
	    /* 
	     This routine copies in the address of the word that the application
	     wants set to FALSE when an ORANGE-BALL (q-bit) comes in.  The ball
	     arrival means that all data sent to the remote has been read. It
	     must be done on connected channel, with no checkpoint already set.
	     */
	    case CHECKPOINT: 
		if (ptr_2chcb -> call_state != CONNECTED)
		    ret_code = ILL_CALSTAT;
		else
		    if (ptr_2chcb -> seg_ckpupd != NULL)
			ret_code = CHKPNT_RUNNING;
		    else {
			ptr_2chcb -> seg_ckpupd = params.param_seg;
			mvwrd_fr (params.param_seg, params.param2, &ptr_2chcb -> off_ckpupd);
			snd_qbitpkt (ptr_2chcb, QYELLOW_BALL);
		    }
		break;

	    default: 
		ret_code = ILL_EVENT;
		break;

	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine disables an 'update' oriented event-code driver function.
   For timers it stops anything running. For all of them, it zeros addresses.
   ------------------------------------------------------------------------- */
void rst_update (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern int  gtwrd_fr(),
		end_time(),
		ret_code;               /* from p_driver.c */

    if (dcb.device.port_numb == 0)
	ret_code = ILL_WAITRQ;
    else {
	switch (gtwrd_fr (params.param_seg, params.param1)) {
	    /* 
	     Turn off the timer if one running, otherwise just zero addresses.
	     */
	    case TIMER_DONE: 
		if (ptr_2chcb -> seg_timupd != NULL) {
		    int_off ();
		    end_time (&upd_ptimer, ptr_2chcb);
		    int_on ();
		}
		ptr_2chcb -> seg_timupd = ptr_2chcb -> off_timupd = NULL;
		break;
	    /* 
	     This NULLs out the address of the application's modem change word.
	     */
	    case MODEM_CHG: 
		dcb.seg_mdmupd = dcb.off_mdmupd = NULL;
		break;
	    /* 
	     This NULLs out the address of the channels checkpoint update word.
	     */
	    case CHECKPOINT: 
		ptr_2chcb -> seg_ckpupd = ptr_2chcb -> off_ckpupd = NULL;
		break;
	    default: 
		ret_code = ILL_EVENT;
		break;
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine disables an 'interrupt' oriented event-code driver function.
   ------------------------------------------------------------------------- */
void set_intrup (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    ret_code = ILL_FUNCTION;
    return;
}



/* -------------------------------------------------------------------------
   This routine disables an 'interrupt' oriented event-code driver function.
   ------------------------------------------------------------------------- */
void rst_intrup (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    ret_code = ILL_FUNCTION;
    return;
}
