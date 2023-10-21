/* -------------------------------------------------------------------------
			Link Input Routines
			filename = p_chario.c

   The routines in this module implement the input of frames from the
   communication line, and the output of frames to the communication line.
   The definition of a 'frame' depends upon whether the mode of the Driver
   is 'character' or 'packet' mode.  Character mode frames are simply
   sequences of characters, and they are passed as is to the packet layer.
   Packet mode frames must be in a particular format, as defined by the
   X.PC protocol.

   cbyte_in      Called by byte_in during character state.
   chr_framend   Terminate frame input while in character mode.

   pmdm_chg       Called by the Async. Interrupt handler when a 'Modem-status
		  changed' interrupt occurs. The interupt routine passes in
		  as a parameter the current 'modem status register' contents.
		  The routine passes the value up to the PAD device controller.
   plnk_chg       Called by the Async. Interrupt handler when a 'line-status
		  changed' interrupt occurs.  The interrupt routine passes in
		  as a parameter the current 'link status register' contents.
   xmit_fail      Called by async_int's xmit_ready when the DRS or CD is lost,
		  so that the application can be told.
   ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
	 Date   By      Reason
	 2/15   Mike    cbyte_in returns TRUE if input end-of-frame
	 2/28   Mike    put int_off & int_on around calls to abort_inframe
	 2/28   Mike    new cont_linkout, changed hold_linkout and hlt_curout
	10/01   Ed M    Reformatted and recommented.
   ------------------------------------------------------------------------- */
#include <driver.h>
#include <pad.h>
#include <pkt.h>
#include <lcbccb.h>

int     chr_framend ();         /* character mode timeout */
extern  xint_time ();           /* packet and character mode xmit timeout */

#define  IN_PKTMODE  (lcb.driver_mode & PACK_FLAG)
#define  IN_CHARMODE (lcb.driver_mode == CHAR_MODE)
#define  PKT_OVERHEAD 9         /* # of bytes overhead: header=7, CRC2=2. */
#define  LCI_FLD(qptr)  (qptr->u.cmd.p_gfilci & P_LCI)

/* -------------------------------------------------------------------------
   These are for use with the character-mode frame timeout processing.
   ------------------------------------------------------------------------- */
#define  CHRIN_YES   1          /* have seen a byte, no collect needed.  */
#define  CHRIN_COLL  2          /* have timed out, need to collect.      */
#define  CHRIN_WAIT  3          /* waiting to see the first byte of frame */


/* -------------------------------------------------------------------------
   pmdm_chg is called when a modem change occurs, while modem change interrupts
   are enabled.  It calls PAD, so the application can be informed of change.
   INTERRUPTS ARE DISABLED.
   ------------------------------------------------------------------------- */
void pmdm_chg (areg)
int     areg;
{
    if (is_enb ())
	abort ();
    lcb.mod_status = areg;
    pad_mdmchg (areg);
    return;
}



/* -------------------------------------------------------------------------
   This routine receives the current 'line status register' contents, because
   an interrupt occured due to a change in the register contents.  It updates
   the que entry header field that the PAD examines to see if data errors or
   break signals have been received.  NO EOI, is handled by interrupt level.
   INTERRUPTS ARE DISABLED.
   ------------------------------------------------------------------------- */
void plnk_chg (status_reg)
int     status_reg;
{
#define OVERUN_FLG   0x02       /* Line Status bits ON if overrun error */
#define FRMERR_FLG   0x08       /* Line Status bits ON if framing error */
#define  BREAK_FLG   0x10       /* Line Status bit ON if break received. */
#define  BRITE_CHR   0xdb       /* bright white char for overrun error */
#define  WHITE_CHR   0xdd       /* l-half white char for framing error */
    if (is_enb ())
	abort ();
    lcb.lin_error = status_reg;
    if (lcb.inb_ptr) {
	if (status_reg & BREAK_FLG) {
	    lcb.num_break++;
	    if (!IN_PKTMODE)
		nothing;
	    else
		abort_inframe ();
	}
	if (status_reg & OVERUN_FLG) {
	    lcb.num_overrun++;
	    if (!IN_PKTMODE)
		ring_in (BRITE_CHR);
	    else
		abort_inframe ();
	}
	if (status_reg & FRMERR_FLG) {
	    lcb.num_framerr++;
	    if (!IN_PKTMODE)
		ring_in (WHITE_CHR);
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   xmit_fail is called by async_int's xmit_ready, if the modem signals
   DSR or CD are lost.  It reports modem failure to PAD layer.
   ------------------------------------------------------------------------- */
void xmit_fail ()
{
    extern void chanst_upd();

    chanst_upd (0, UPD_MDMLOST, TRUE);
    return;
}



/* -------------------------------------------------------------------------
   chr_framend is called by time-out in char mode, to artificially cause
   an "end-of-frame" to occur for non-packet input of bytes.  If find that
   'lcb.chr_framtim' is still 'time to collect', and have characters in the
   current bufferlet, better call end_inframe to restart new psuedo-frame, then
   prc_inframe to turn any collected frames over to the pad layer. Keep timer,
   declare 'fake_param' because timer routine assumes have parameter to push.
   if prc_inbyte is TRUE, do nothing.
   ------------------------------------------------------------------------- */
void chr_framend (fake_param)
int fake_param;
{
    if (lcb.ccbp[0])
	lcb.ccbp[0] -> time_run &= ~TIM_CHRI;
    int_off ();
    /*
     If data to process, and 2 timer interrupts have occurred, set to
     CHRIN_WAIT, assume got end of frame, so put data into lcb.in_que, and
     tell prc_inframe () to process it.

     If two interrupts haven't happened yet, set state to CHRIN_COLL, so
     will get processed next time, and reset the timer for safety's sake.
     */
    if (lcb.in_count) {
	if (lcb.chr_framtim == CHRIN_COLL) {
	    lcb.chr_framtim = CHRIN_WAIT;
	    end_inframe ();
	    prc_inframe ();
	}
	else {
	    lcb.chr_framtim = CHRIN_COLL;
	    strt_time (lcb.char_timout, chr_framend, 0, TIM_CHRI);
	}
    }
    int_on ();
    return;
}



/* -------------------------------------------------------------------------
   cbyte_in is called by 'byte_in', for bytes input during character mode,
   after byte_in has stored the character.
   Count = 0, if full bufferlet.  QUE pointer is guaranteed by byte_in.
   INTERRUPTS ARE ENABLED.
   ------------------------------------------------------------------------- */
void cbyte_in (count)
int     count;          /* bufferlet count after storing byte */
{
    extern void net_xoffprc();
    BUF_PTR     buf;
    int         i,
		full_frame = 0; /* frame complete for async_int */
 /*
 While in char mode, there are no 'frames'.  Implement psuedo-frames by 
 nearly-full bufferlet, or by timer set chr_framtim = COLL (1st timeout)
 as a signal to us to end-frame (timer does end-frame in 2nd timeout) 
 */
    int_off ();
    if ((lcb.chr_framtim == CHRIN_COLL) || (count == (BFLT_DATA_SIZE - 2))) {
	lcb.chr_framtim = CHRIN_WAIT;
	count = 0;
	full_frame = TRUE;
	stop_time (chr_framend, 0, TIM_CHRI);
    }
    int_on ();
    /*
     if first byte in a "frame," start the receive timer
     */
    if (count == 1) {
	int_off ();
	strt_time (lcb.char_timout, chr_framend, 0, TIM_CHRI);
	int_on ();
    }
    net_xoffprc (CHECK);
    return (full_frame);
}



/* -------------------------------------------------------------------------
   char_compl is called by strt_ouframe, when a buffer chain end is found.
   ------------------------------------------------------------------------- */
char_compl (que)
QUE_PTR que;
{
    extern void rel_que(),
		chrs_sent(),
		chanst_upd();
    BUF_PTR     tbuff;
    int         bcount = 0;

    tbuff = que -> u.cmd.bfirst;
    /*
     add up all the bufferlet counts
     */
    while (tbuff != NULL) {
	bcount += tbuff -> num_bytes;
	tbuff = tbuff -> bnext;
    }
    chrs_sent (bcount);         /* updates queued char count */
    int_off ();
    rel_que (que, YES_BUFF);
    int_on ();
    chanst_upd (0, UPD_XMIT, (lcb.xpc_nrdy == 0));
    return;
}



/*-------------------------   MIKE'S CHANGE:  ---------------------------------
   There is no advantage to all this logic: to search for the last bufferlet
   in the chain; release all that have been sent; remove all sent bytes from
   the bufferlet; add all byte counts to the chars-sent total; and re-enque
   to the priority queue.  WE SIMPLY TURN OFF THE TRANSMITTER!!!  Later, the
   PAD layer will call cont_linkout (), to continue output from mid-buffer.
   Async_int (out) calls byte_out, which calls this function, if xmit_blocked.
   INTERRUPTS ARE DISABLED.
   ------------------------------------------------------------------------- */
void hlt_curoutput ()
{
    extern void xmit_dsb();
    /*
     if there is any output pending, disable the transmitter
     */
    if (lcb.outq_entry && lcb.outb_ptr) {
	xmit_dsb ();
	stop_time (xint_time, 0, TIM_XMIT);
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine is called by the PAD to halt character channel output pronto!
   ------------------------------------------------------------------------- */
void hold_linkout ()
{
    lcb.xmit_blocked = TRUE;
    return;
}



/* -------------------------------------------------------------------------
   This routine is called by the PAD to restart transmission.  If there is any
   data to send, restart the transmit interrupts as well.
   ------------------------------------------------------------------------- */
void cont_linkout ()
{
   extern void xmit_enb();

   int_off (); 
   if (lcb.xmit_blocked) {
	lcb.xmit_blocked = 0;
	if (lcb.outq_entry) {
	    int_off ();
	    strt_time (lcb.htime, xint_time, 0, TIM_XMIT);
	    xmit_enb ();
	    int_on ();
	}
    }
    int_on ();
    return;
}
