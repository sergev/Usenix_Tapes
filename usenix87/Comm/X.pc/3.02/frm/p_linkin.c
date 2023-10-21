/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
			Link Input Routines
			filename = p_linkin.c

   The routines in this module implement the input of frames from the
   communication line, and the output of frames to the communication line.
   The definition of a 'frame' depends upon whether the mode of the Driver
   is 'character' or 'packet' mode.  Character mode frames are simply
   sequences of characters, and they are passed as is to the packet layer.
   Packet mode frames must be in a particular format, as defined by the
   X.PC protocol.

   strt_inframe  Called when frame has been input, and/or init next for frame.
   byte_in       Called by Hardware Interrupt, process a byte that was input.
   pbyte_in      Called by byte_in during packet state.
   end_inframe   Process a complete (incoming) frame, enque into input queue.
   prc_inframe   Take frame from intput que, turn over to packet or pad layer.
   abort_infram  Will abort the current frame: start over with first bufferlet,
		 release any after the first back to the available pool.
   ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
	 Date    Change  By      Reason
      06/13/84     00   curt     Initial generation of PAD version of code.
      06/26/84     01   curt     Major rewrite to clean up xpc_ndry situation.
      07/15/84     02   curt     Minor rewrite to clean up in general.
      08/14/84     03   curt     Major rewrite to speed up input processing.
      08/31/84     04   curt     Still more cleanup of I/O processing.
      09/01/84     05   Mike     Fix input of superv, contol pkt (RR, REJ RNR)
      10/12/84     06   curt     Rewrote charmode I/O, direct connect to PAD.
      10/31/84     07   curt     Major rewrite: due to changes in p_comint.asm.
				 Added the 'prc_inframe' call, process frames.
      11/30/84     08   curt     cleanup packet mode overrun detection.
   ------------------------------------------------------------------------- */
#include "driver.h"
#include "lcbccb.h"
#include "pkt.h"
#include "pad.h"

#define  LCBINQUE lcb.inq_entry->u.cmd
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

extern  chr_framend ();         /* character mode input timeout */

/* -------------------------------------------------------------------------
   abort_inframe is called by plnk_chg (break status) when pkt_mode & in_synch,
   and by pbyte_in when we cannot get a bufferlet (to start or continue chain).
   This routine will throw away the current contents of the input que entry,
   and will release any chained bufferlets for long frames.  All pointers and
   indices will be set to point to the start of the bufferlet, and the packet
   mode flag 'in_synch' is set to be FALSE (forcing new search for header).
   Caller must have *** INTERRUPTS OFF *** to prevent input durring this time.
   ------------------------------------------------------------------------- */
void abort_inframe () {
    if (lcb.inb_ptr) {
	if (lcb.toss_cnt)
	    lcb.toss_cnt--;
	else {
	    if ((lcb.in_synch) && (lcb.inb_ptr != &lcb.in_buff)) {
		lcb.toss_cnt = (lcb.in_size) ? lcb.in_size - lcb.in_count : 0;
		rel_blist (LCBINQUE.bfirst);
	    }
	    lcb.inb_ptr = &lcb.in_buff;
	    LCBINQUE.bfirst = lcb.inb_ptr -> bnext = NULL;
	    lcb.in_count = lcb.inb_ptr -> num_bytes = 0;
	    lcb.in_synch = FALSE;
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   byte_in is called by Async post-interrupt prodess when a character is found
   in the ring buffer.  This routine checks first to make sure that there is a
   bufferlet to store into, and attempts to get one if there isn't one.  If it
   can't get one, it exits.  If there is a bufferlet or one is gotten, input
   proceeds:  call proper routine for mode.
   INTERRUPTS ARE ENABLED.
   ------------------------------------------------------------------------- */
/*
 First, check for transmit interrupt timeout.  If it did, re-enable the
 8250.  Then see if we are discarding part of a packet, if so, decrement
 the toss count and return.

 If we actually have some useful data and we want to do something with it,
 get a queue or buffer as needed and stuff the data in it.
 */

int     byte_in (data)
int     data;                   /* received character */
{
    extern BUF_PTR get_buff();
    extern int     pad_xoffprc();
    BUF_PTR bptr;
    int     process;

    if (lcb.time_on & TIM_EXPR) {
	lcb.time_on &= ~TIM_EXPR;
	int_off ();
	re_enable ();
	int_on ();
    }
    if (lcb.toss_cnt > 0)
	lcb.toss_cnt--;
    else {
	if (lcb.inq_entry == NULL)
	    strt_inframe ();
	else
	    if (lcb.inb_ptr == NULL) {
		int_off ();
		lcb.inb_ptr = lcb.inq_entry -> u.cmd.bfirst = get_buff ();
		int_on ();
	    }
	if (lcb.inb_ptr) {
	    /*
	     Char mode uses XOFF/XON to control send; will return TRUE,
	     if ate it
	     */
	    if (IN_PKTMODE || !pad_xoffprc (data)) {
		lcb.inb_ptr -> data[lcb.inb_ptr -> num_bytes++] = data;
		lcb.in_count++;
		if (lcb.inb_ptr -> num_bytes >= BFLT_DATA_SIZE) {
		    int_off ();
		    /*
		    If full bufferlet, chain to next.  If no more, ignore byte.
		    If using the lcb's buffer, or there are no more bufferlets
		    to chain to, ignore this character, and bump the count of
		    characters lost.
		    */
		    if ((lcb.inb_ptr == &lcb.in_buff) ||
			((bptr = get_buff ()) == NULL)) {
			lcb.inb_ptr -> num_bytes--;
			lcb.num_chrlost++;
		    }
		    else {
			lcb.inb_ptr -> bnext = bptr;
			lcb.inb_ptr = bptr;
			lcb.inb_ptr -> num_bytes = 0;
		    }
		    int_on ();
		}
		if (IN_PKTMODE) {
		    process = pbyte_in (data);
		    if (process & 2)
			pack_sync (LCI_FLD (lcb.inq_entry), lcb.inq_entry);
		}
		else
		    process = cbyte_in (lcb.in_count);
		if (process & 1) {
		    end_inframe ();
		    prc_inframe ();
		}
	    }
	}
	else
	    bptr = bptr;        /* NOP for debugging */
    }
    return;
}



/* -------------------------------------------------------------------------
   pbyte_in is called by 'byte_in', for bytes input during packet mode.  8-bit
   value is input, is handled unless no que/buffs avail.  If the byte fills
   the current bufferlet, a new bufferlet is chained to the full one.  If the
   byte is the last for a frame, the frame is turned over to the packet layer
   and a new frame is started.  QUE pointer is guaranteed by byte_in.
   INTERRUPTS ARE ENABLED.
   ------------------------------------------------------------------------- */
int     pbyte_in (data)
int     data;                   /* for CRC computation per byte */
{
    extern int  pkt_recvd;      /* set: pbyte_in (good CRC1);
				   reset: test in rr_time */
    extern BUF_PTR get_buff();

    BUF_PTR bptr;
    int     i,                  /* loop index */
	    found,              /* used in STX search after bad CRC found.  */
	    full_frame = 0,     /* end-of-frame. async_int calls prc_inframe */
	    crc_one = 0;        /* processing CRC1 byte. call pack_sync */

    /*
     If looking for STX, and this not it, just exit after EOI to 8259
     */
    if ((lcb.in_count == 1) && (data != STX))
	lcb.in_count = lcb.inb_ptr -> num_bytes = 0;
    else {
	full_frame = FALSE;
	/*
	In Synch: Store the byte recvd.  Incr ctrl count in buflet and lcb.
	*/
	if (lcb.in_synch)
	    full_frame = (lcb.in_count >= lcb.in_size) ? TRUE : FALSE;
	else {
	    /*
	    OK to put data into the packet.  If 7 bytes stored (index is 8) in
	    the packet header, try to verify data correct by checking CRC1.
	    */
	    if (lcb.in_count == 7) {
		if (crc1_chk (lcb.inb_ptr)) {
		    crc_one = TRUE;
		    pkt_recvd = TRUE;
		    LCBINQUE.p_id = lcb.inb_ptr -> data[B_PKTID];
		    LCBINQUE.p_seqenc = lcb.inb_ptr -> data[B_PRPS];
		    LCBINQUE.p_gfilci = lcb.inb_ptr -> data[B_GFLCI];
		    lcb.in_size = lcb.inb_ptr -> data[B_SIZE];
		    /*
		    Got valid packet!!! Now in_synch, and know size of packet
		    to expect.  If zero, is short packet; else continue
		    */
		    if (lcb.in_size == 0)
			full_frame = TRUE;
		    else {
			lcb.in_size += PKT_OVERHEAD;
			lcb.in_synch = TRUE;
		    }
		    /*
		     If packet  is not a supervisory packet, that is not an
		     RR, REJ, or RNR, copy its data from lcb's buffer to a
		     new one to put the rest of its data into.  If no new
		     buffer can be found, abort this frame and start over.
		     */
		    if (((i = pack_check (lcb.inq_entry)) < RR_TYP)
			    || (i > REJ_TYP)) {
			LCBINQUE.b_state = B_RECVD;
			int_off ();
			if (bptr = get_buff ()) {
			    bptr -> num_bytes = lcb.in_buff.num_bytes;
			    for (i = 0; i < 7; i++)
				bptr -> data[i] = lcb.in_buff.data[i];
			    LCBINQUE.bfirst = lcb.inb_ptr = bptr;
			}
			else {
			    full_frame = 0;
			    crc_one = 0;
			    abort_inframe ();
			}
			int_on ();
		    }
		}
		else {
		    /*
		    Got 7 bytes in, with an STX at front, but CRC in packet was
		    bad. Look through bufferlet for another STX, if find one,
		    shift bufferlet contents so STX is in first byte.
		    */
		    found = 0;
		    for (i = 1; i < 7; i++) {
			if ((found == 0) && (lcb.inb_ptr -> data[i] == STX))
			    found = i;
			if (found > 0)
			    lcb.inb_ptr -> data[i - found] = lcb.inb_ptr -> data[i];
		    }
		    /*
		    If didn't find STX, start with first byte next input.
		    If found the STX, decrement indices into bufferlet.
		    */
		    if (found == 0)
			lcb.in_count = lcb.inb_ptr -> num_bytes = 0;
		    else {
			lcb.in_count -= found;
			lcb.inb_ptr -> num_bytes -= found;
		    }
		}
	    }   
	}
    }   
    /*
    call pack_sync (return 2) and end/prc_inframe (return 1)
    */
    return ((crc_one << 1) + full_frame);
}



/* -------------------------------------------------------------------------
   end_inframe is called by both byte_in when end_of_frame occurs, or by the
   chr_framend routine to simulate an end_of_frame in char mode (after timeout).
   The routine only enques the current input frame into the input que, it does
   not hand the frame over to the packet or pad layer (see prc_inframe).
   INTERRUPTS ARE ENABLED
   ------------------------------------------------------------------------- */
void end_inframe ()
{
#define OVERUN_FLG  0x02        /* Line Status bits ON if overrun error */
#define FRMERR_FLG  0x08        /* Line Status bits ON if framing error */
#define  BREAK_FLG  0x10        /* Line Status bit ON if break received. */
#define INPERR_FLG OVERUN_FLG+FRMERR_FLG

    extern void enque();
    QUE_PTR que;

    if (!IN_PKTMODE) {
	if (lcb.lin_error & INPERR_FLG)
	    lcb.inq_entry -> CHRERR_FLD |= DATERR;
	else
	    if (lcb.lin_error & BREAK_FLG)
		lcb.inq_entry -> CHRERR_FLD |= BRKRCV;
	lcb.lin_error = 0;
    }
    que = lcb.inq_entry;
 /* 
  set up next frame for input, then add the frame to the input que.
  */
    strt_inframe ();
    int_off ();
    enque (&lcb.in_que, que);
    int_on ();
    return;
}



/* -------------------------------------------------------------------------
   process-input-frame is called after byte_in returns to async_int and an EOI
   output to 8259, or by the character mode timeout routine (chr_framend).
   Handling of packet at this point depends upon character versus packet mode.
   Character mode, pad_in (go around packet layer). Pack_in call for pktmode.
   If a packet is currently being processed by either, don't call again with
   a new one (happens when a packet's processing takes longer than the input
   of the following packet).  If not currently processing a packet, and while
   there are link frames available, turn them over to pad or packet layer.
   ------------------------------------------------------------------------- */
void prc_inframe ()
{
    extern QUE_PTR deque();
    QUE_PTR qhold;

    int_off ();
    if ((lcb.in_que.ent_cnt != 0) && (lcb.pkt_inproc == FALSE)) {
	lcb.pkt_inproc = TRUE;
	do {
	    qhold = deque (&lcb.in_que);
	    int_on ();
	    if (IN_PKTMODE)
		pack_in (LCI_FLD (qhold), qhold);
	    else
		pad_in (qhold);
	    int_off ();
	}
	while (lcb.in_que.ent_cnt != 0);
	lcb.pkt_inproc = FALSE;
    }
    int_on ();
    return;
}



/* -------------------------------------------------------------------------
   strt_inframe is called by Link_init and by Byte_in (if end-of-frame), to
   set up input variables for the next frame and enable input.  This allows
   the next byte received to be stored in the bufferlet by Byte_in.
   If the buff_addr is NULL, need to get a bufferlet, else use one passed in.
   INTERRUPTS ARE ENABLED.
   ------------------------------------------------------------------------- */
void strt_inframe ()
{
    extern BUF_PTR get_buff();
    extern QUE_PTR get_que();
    extern void    rel_que(),
		   recv_enb();
    extern int     recv_on();
    QUE_PTR que;
    BUF_PTR buf;

    int_off ();
    que = get_que (NO_BUFF);
    if (que == NULL)
	buf = NULL;
    else {
	/*
	 If in packet mode, use lcb's buffer.  In character mode, try to get a
	 buffer.
	 */
	if (IN_PKTMODE)
	    buf = &lcb.in_buff;
	else {
	    int_on ();
	    int_off ();
	    if (!(buf = get_buff ())) {
		rel_que (que, NO_BUFF);
		que = NULL;
	    }
	}
	int_on ();
	int_off ();
	if (recv_on () == FALSE)
	    recv_enb ();
    }
    int_on ();
 /* 
  NO LCB'S BUFFER ADDR IN QUE ENTRY, or rel_que will fail
  */
    int_off ();
    if ((lcb.inq_entry = que) && (buf != &lcb.in_buff))
	que -> u.cmd.bfirst = buf;
    if (lcb.inb_ptr = buf) {
	buf -> bnext = NULL;
	buf -> num_bytes = 0;
    }
    lcb.toss_cnt = lcb.in_count = 0;
    lcb.in_synch = FALSE;
    int_on ();
    return;
}
