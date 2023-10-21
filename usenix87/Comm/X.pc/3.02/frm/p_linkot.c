/* -------------------------------------------------------------------------
			Link Output Routines
			filename = p_linkot.c

   The routines in this module implement the input of frames from the
   communication line, and the output of frames to the communication line.
   The definition of a 'frame' depends upon whether the mode of the Driver
   is 'character' or 'packet' mode.  Character mode frames are simply
   sequences of characters, and they are passed as is to the packet layer.
   Packet mode frames must be in a particular format, as defined by the
   X.PC protocol.

   link_open     Opens the Link layer for processing.
   link_close    Closes the Link layer, no further processing possible.
   link_init     Enables Input/Output Interrupt processing for the Link layer.
   link_term     Disables Input/Output Interrupt processing for the Link layer.
   send_active   waits until output queues empty and output buffer ptr = 0.
   strt_ouframe  Begin processing to output a frame.
   link_write    Que up frame for output, call strt_ouframe if output not going.
   byte_out      Output a byte by calling Async. Interrupt Routine to xmit.
   stop_linkout  Sets the lcb flag 'xmit_blocked' so next char will not go.
   hlt_curoutput Will fix up bufferlets: allow restart of output where stopped.
   ------------------------------------------------------------------------- */
#include "driver.h"
#include "lcbccb.h"
#include "pad.h"
#include "pkt.h"

#define  LCBINQUE lcb.inq_entry->u.cmd
#define  LCBOUTQUE lcb.outq_entry->u.cmd
#define  IN_PKTMODE  (lcb.driver_mode & PACK_FLAG)
#define  IN_CHARMODE (lcb.driver_mode == CHAR_MODE)
#define  PKT_OVERHEAD 9         /* # of bytes overhead: header=7, CRC2=2. */
#define  LCI_FLD(qptr)  (qptr->u.cmd.p_gfilci & P_LCI)
extern  chr_framend ();         /* need forward-declare for link_term.   */

/* -------------------------------------------------------------------------
   These are for use with the character-mode frame timeout processing.
   ------------------------------------------------------------------------- */
#define  CHRIN_YES   1          /* have seen a byte, no collect needed.  */
#define  CHRIN_COLL  2          /* have timed out, need to collect.      */
#define  CHRIN_WAIT  3          /* waiting to see the first byte of frame */

static int  timeval[] = {
    128, 64, 32, 16, 8, 4, 2, 1
};
/* define internal timeout functions */
int     xint_time ();           /* no buffer for ctrl pkt, lost xmit intrupt */



/* -------------------------------------------------------------------------
   Link_open is called by the packet layer to open a channel at the link layer.
   ------------------------------------------------------------------------- */
void link_open (lci, open_opt)
int     lci,
	open_opt;
{
    lcb.ch_open |= (1 << lci);
    return;
}



/* -------------------------------------------------------------------------
   Link_close is called by the packet layer to close the link layer channel.
   ------------------------------------------------------------------------- */
void link_close (lci)
int     lci;
{
    lcb.ch_open &= ~(1 << lci);
    return;
}



/* -------------------------------------------------------------------------
   Link_init is called by packet layer to initialize link layer input/output.
   Make sure xmit disabled, call strt_inframe to set up pointers, enable input.
   ------------------------------------------------------------------------- */
void link_init ()
{
    int     delay,
	    i;
    int     rate;
    /*
     baud =          75, 150, 300, 600,1200,2400,4800,9600
     rate =           0,   1,   2,   3,   4,   5,   6,   7 
     char_timout  =   4,   4,   3,   3,   2,   2,   1,   1
     timeval[] = {  128,  64,  32,  16,   8,   4,   2,   1}
     htime        = 384  192   96   48   24   12    6    3 
     if EVNT=6
     seconds =       64   32   16    8    4    2    1  0.5
     We don't start the recv timeout here; it is done at 1st byte
     */

    rate = rep_lkch () >> 8;    /* get link baud rate number */
    /*
	Ed M -- change char_timout to a flat 1 to avoid jerky character mode
	at lower baud rates.
    */
    lcb.char_timout =  1;
    /*
	Ed M -- set lcb.htime to flat 4 seconds
    */
    lcb.htime = 4 * EVNT_PSEC;
     /*
	((timeval[rate] * DAT_WNDWSZ * EVNT_PSEC) >> 3);
      # seconds for a given baud rate computation as follows:
      9600:     1    *    4    *    6    /    8  =  3 events = 1/2 sec
      1200:     8    *    4    *    6    /    8  = 24 events =  4  sec
      */
    if (lcb.htime == 0)
	lcb.htime = 1;          /* insurance */
    /*
     Delay restart send to allow time for other end to start packet mode.
     On an IBM XT, delay should work out to about 60 ms at 1200 baud.
     Probably should do something more intelligent here since timing loops
     depend on the speed of the hardware.  On a quick AT clone, this loop
     might only take 15 ms.
     */
    rate = timeval[rate];
    for (i = 0; i < rate; i++)
	for (delay = 0; delay < 256; delay++);
    int_off ();
    lcb.outq_entry = NULL;
    lcb.outb_ptr = NULL;
    lcb.outb_index = lcb.num_chrlost = lcb.num_outlost = lcb.time_on = 0;
    lcb.mod_status = lcb.num_break = lcb.num_overrun = lcb.num_framerr = 0;
    int_on ();
    strt_inframe ();
    return;
}



/* -------------------------------------------------------------------------
   Link_term is called by packet layer to disable all link layer communication.
   The routine disables interrupts and releases all currently held que entries.
   ------------------------------------------------------------------------- */
void link_term ()
{
    extern void rel_que(),
		recv_dsb(),
		xmit_dsb();

    int_off ();
    recv_dsb ();
    xmit_dsb ();
    rel_qhead (&lcb.in_que);
    rel_qhead (&lcb.rep_que);
    rel_qhead (&lcb.out_que);

    if (lcb.inq_entry != NULL)
	rel_que (lcb.inq_entry, LCBINQUE.bfirst);
    /*
     if (lcb.outq_entry != NULL)
     * in pkt mode, this is already rel *
     |     rel_que (lcb.outq_entry, LCBOUTQUE.bfirst);
     */
    lcb.outq_entry = lcb.inq_entry = NULL;
    lcb.outb_ptr = lcb.inb_ptr = NULL;
    lcb.outb_index = lcb.in_count = lcb.in_size = 0;
    stop_time (chr_framend, 0, TIM_CHRI);
    stop_time (xint_time, 0, TIM_XMIT);
    int_on ();
    return;
}



/* -------------------------------------------------------------------------
   strt_ouframe is called by link_write and Byte_out (if end-of-frame, inter
   after xmit buffer empty).  Processing at the Packet level has put all
   packets into a queue that is common to all channels for output.
   This routine attempts to get a que entry setup for output by byte_out.
   INTERRUPTS ARE ENABLED.
   ------------------------------------------------------------------------- */
void strt_ouframe ()
{
    extern QUE_PTR deque();
    extern void    rel_que(),
		   xmit_enb(),
		   chanst_upd();

    BUF_PTR tbuff;
    QUE_PTR tque;
 /*
  The last comm interrupt has occurred, can run with interrupt enabled.
  If outq_entry points to pkt, release depends upon char/pkt state:
      Char lets PAD decide how many were sent;
      Packet calls pack_complete routine
 */
    int_off ();
    if ((tque = lcb.outq_entry) != NULL) {
	lcb.outq_entry = NULL;
	int_on ();
	if (IN_PKTMODE)
	    pack_compl (tque);
	else
	    char_compl (tque);
    }
    int_on ();
    /*
     Try to get a queue entry from one of the output queues.  Search in order
     of priority for output (high to low).  Once deque returns a non-NULL value,
     drop out of if-then-else to process the entry.  Otherwise, once searched
     all queue headers, turn off transmit inters, & exit (packet sets RR timer)
     */
    int_off ();
    if (lcb.driver_mode & PACK_FLAG) {
	while (lcb.rep_que.ent_cnt && purge_rr ()) {
	    rel_que (deque (&lcb.rep_que), NO_BUFF);
	    int_on ();
	    int_off ();
	}
    }   
    tque = deque (&lcb.rep_que);
    int_on ();
    if (tque == NULL) {
	int_off ();
	tque = deque (&lcb.out_que);
	int_on ();
    }
    if (tque == NULL) {
	/*
	 Did not find a queue entry to process, done with transmission.
	*/
	lcb.xmit_busy = FALSE;
	if (!IN_PKTMODE)
	    chanst_upd (0, UPD_XMIT, (lcb.xpc_nrdy == 0));
    }
    else {
	/*
	At this stage lcb.outq_entry contains que entry to send, with buffer.
	Process the que entry:  If no bufferlet, control packet needs buffer.
	*/
	tbuff = tque -> u.cmd.bfirst;
	if (IN_PKTMODE) {
	    if (tbuff == NULL) {
		/*
		Superv, ctrl packets (RR, RNR, REJ) use LCB's short buffer.
		NO LCB'S BUFFER ADDR IN QUE ENTRY, or rel_que will fail.
		*/
		tbuff = &lcb.out_buff;
		tbuff -> data[B_GFLCI] = tque -> u.cmd.p_gfilci;
		tbuff -> data[B_PRPS] = tque -> u.cmd.p_seqenc;
		tbuff -> data[B_PKTID] = tque -> u.cmd.p_id;
		tbuff -> data[B_SIZE] = 0;
		tbuff -> num_bytes = 7;
	    }
	    /*
	    Pack_start stores the P(R)/P(S) numbers for the  packet. Last thing
	    to do is to calculate and store checksum by calling 'gen' routine.
	    */
	    tbuff -> data[B_STX] = STX;
	    pack_start (LCI_FLD (tque), tque);
	    crc1_gen (tbuff);
	}
	/*
	Set up the pointer and index into the first buffer to output.  If transmit
	interrupts not already on, enable them to start frame out.
	*/
	int_off ();
	strt_time ((lcb.htime + 1) >> 1, xint_time, 0, TIM_XMIT);
	lcb.outq_entry = tque;
	lcb.outb_ptr = tbuff;
	lcb.outb_index = 0;
	xmit_enb ();
	int_on ();
    }
    return;
}



/* -------------------------------------------------------------------------
   Link_write is called by any routine with a frame to send.  It queues the
   frame up into the proper queue, then starts transmission if not running.
   May also be called with 'which_que' = LWRIT_NUL, which should cause the
   enque logic to be skipped, but the xmit startup logic to execute.
   ------------------------------------------------------------------------- */
void link_write (which_que, frame)
int     which_que;
QUE_PTR frame;
{
    extern void enque();
    extern int  xmit_on();

    int_off ();
    if (which_que == LWRIT_REP)
	enque (&lcb.rep_que, frame);
    else
	if (which_que == LWRIT_OUT)
	    enque (&lcb.out_que, frame);
     /*
     If not currently transmitting, startup frame output logic. If currently
     transmitting, the end-of-current-frame logic will pick up new frame. 
     */
    int_on ();
    int_off ();
    if ((xmit_on () == FALSE) && (lcb.xmit_busy == FALSE)) {
	lcb.xmit_busy = TRUE;
	int_on ();
	strt_ouframe ();
    }
    int_on ();
    return;
}



/* -------------------------------------------------------------------------
   Byte_out is called by Comm-output interrupt.  xmit_byt gets next byte
   from current bufferlet (pointer is always valid), and sends it.  After send,
   increment index value.  If finished current bufferlet, get next.  If have
   finished the entire bufferlet list, call strt_ouframe: will setup next frame
   or will disable transmission (when no next frame available).
   INTERRUPTS ARE DISABLED.
   ------------------------------------------------------------------------- */
int     byte_out ()
{
    extern void xmit_byt(),
		xmit_dsb();

    int     complete = 0;
    if (is_enb ())
	abort ();
    if (lcb.xmit_blocked == TRUE)
	hlt_curoutput ();       /* char I/O recved XOFF */
    else {
	if (lcb.outb_ptr != NULL) {
	    /*
	     Send the current byte and bump the index.
	     Check to see if have sent this bufferlet, if so, get next one
	     and reset the index.
	     */
	    xmit_byt (lcb.outb_ptr -> data[lcb.outb_index++]);
	    if (lcb.outb_index >= lcb.outb_ptr -> num_bytes) {
		lcb.outb_index = 0;
		lcb.outb_ptr = lcb.outb_ptr -> bnext;
	    }
	}
	else {
	    /*
	     Sent all the bytes we have, disable transmit interrupts, end
	     the transmit timeout, and tell async_int() that we're all
	     done.
	     */
	    xmit_dsb ();
	    stop_time (xint_time, 0, TIM_XMIT);
	    complete = 4;
	}
    }
    return (complete);
}



/* -------------------------------------------------------------------------
   xint_time is called by time_out to try again to send a byte after transmit
   failed to produce an xmit interrupt.  If lcb.hlt_curoutput is set, just
   disable and clear the chip's errors (don't enable or start timeout)

   Increment count of lost interrupts, check to see if first entry to
   function.  If so, and input active, tell the receive part that transmit
   has failed.  If input not active re-enable the 8250.  Restart the
   transmit timeout whether or not input is active.

   On second entry, just re-enable the chip.
   ------------------------------------------------------------------------- */
xint_time (lci)
int     lci; {
    if (lcb.ccbp[0])
	lcb.ccbp[0] -> time_run &= ~TIM_XMIT;
    lcb.num_outlost++;
    int_off ();
    if (!(lcb.time_on & TIM_EXPR)) {
	if (lcb.in_count)
	    lcb.time_on |= TIM_EXPR;
	else
	    re_enable ();
	strt_time ((lcb.htime + 1) >> 1, xint_time, 0, TIM_XMIT);
    }
    else
	re_enable ();
    int_on ();
    return;
}



/* -------------------------------------------------------------------------
   send_active is called by pad layer to find out if link output has completed,
   for transition from character mode to packet mode or vice-versa.
   Returned is done when there is NO ACTIVE I/O, when all are zero:
   lcb.out_que, ccb[0].hold_que, and the output buffer pointer (active I/O).
   ------------------------------------------------------------------------- */
void send_active ()
{
    int     count;
    int     delay,
	    i;                  
    int     rate;       
    rate = rep_lkch () >> 8;    /* get link baud rate number */
    rate = timeval[rate] * 256;
    /*
     loop overhead is 60ms (link_init).  Attempt to delay 2 seconds, again,
     we should probably do something more intelligent here than a software
     loop.
     */
    for (delay = 0; delay < rate; delay++) {
	int_off ();
	count = lcb.out_que.ent_cnt | (int) lcb.outq_entry;
	if (lcb.ccbp[0])
	    count |= lcb.ccbp[0] -> hold_que.ent_cnt;
	int_on ();
	if (count == 0)
	    delay = rate;
    }
 /*
 If loop reaches limit and queue entries are out, then term_chrmode or
 term_pktmode will release them
 */
    return;
}
