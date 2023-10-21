/* -------------------------------------------------------------------------
		     Utility I/O PAD routines
		       filename = p_utilio.c

   The routines in this module are useful to the standard Input/output process
   of the PAD and to the PAD echoing input/output process.

   snd_asis    accepts a chan ^.  Copies in caller's data to packets/bufferlets
	       output que, no special processing is done on the bytes.
   snd_byte    accepts a byte.  Calls link_write, makes byte go out priority.

   net_xoffprc Processes the current character channel situation with respect
	       to XOFF/XON chars that need to be sent to the network.
   pad_xoffprc accepts a character (either XOFF or XON).  Processes the current
	       situation with respect to XOFF/XON chars that are received.

   pad_in      accepts a que ^.  Called by Link layer (char mode), enque frame.
   pad_write   accepts a chan ^.  While that channel has any data in the
	       'out_que', it trys to turn over it to the packet layer.
   pad_read    accepts a chan ^.  In packet mode, while any data queued at the
	       packet layer, tries to retrieve to pad layer for processing.
   strt_wrpkt  accepts a chan ^.  Inits the control values for the channel:
	       gets a que entry and bufferlet for use as an output packet.
	       Assumes packet state, reset 'num_bytes' if using for char state.
   next_wrbuf  accepts a chan ^.  Appends a bufferlet to the end of the chain
	       (apwr_que + buffs) and initializes indices.  Will return FALSE
	       if there are not any bufferlets avail (apwr_buf still valid).
   writ_wrpkt  accepts a chan ^.  Enques the current 'apwr_que' packet onto the
	       'out_que' header.  While chan's xmit_enble flag = TRUE, out_que
	       packets are given to packet level.  Uses 'apwr_tot' for length.

   next_rdpkt  accepts a chan ^.  Sets up the channels 'aprd' data structures
	       with pointers to next packet in input que: in_que quehead, and
	       if nothing is there, calls pad_read.  Returns with aprd_que
	       pointing to a packet or set to NULL.
   next_rdbuf  accepts a chan ^.  Moves the pointers forward in the bufferlet
	       chain, by using 'aprd_buf' as current.  If current is last one,
	       releases entire chain it to the available bufferlet pool.

   out_flush   accepts a chan^. Throws away all data waiting for packet write.
   in_flush    accepts a chan^.  Throws away all data waiting for appl read.
   chrs_sent   accpts a count of bytes transmitted for channel 0, char mode.
   mv_buf2pkt  accepts a chan ^, a buffer segment and offset, and max to move.
	       Moves from buffer address to packets/bufferlets in the channels
	       output que.  Moves one packets worth at a time.
   mv_pkt2buf  accepts a chan ^, a buffer segment and offset, and max to move.
	       Moves the data from the current read packet (only) to buffer.
   -------------------------------------------------------------------------
	 Date   Change   By   Reason
      05/25/84    01    curt  Initial Generation of Code.
      06/12/84    02    curt  Cleaned up for testing phase.
      06/17/84    03    curt  Major rewrite to use pointer (^), not channel #.
      10/12/84    04    curt  Major fixup of network XOFF/XON processing.
      05/18/85    05    curt  Changes to move data, fixup comments.
      07/31/85    06    curt  Completion of MCI echo code
      09/26/86    07    Ed M  Reformat and recomment
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "pad.h"
extern  frwd_timr ();
#define offclc(offset,index) ((unsigned) (offset) + (unsigned) (index))

/* -------------------------------------------------------------------------
   This routine copies all of the data from the caller's buffer directly into
   a packet with bufferlets.  No processing done concerning the data content.
   It terminates when all the bytes of the callers buffer are moved.
   It return the number of bytes movedfrom the buffer to the packets.
   ------------------------------------------------------------------------- */
int     snd_asis (ptr_2chcb, buf_seg, buf_off, byte_cnt)
	CHCB_PTR ptr_2chcb;
char   *buf_seg;
char   *buf_off;
int     byte_cnt;
{
    int     cur_byt = 0;

    while (cur_byt < byte_cnt) {
	if (ptr_2chcb -> apwr_tot >= DEF_pktlim)
	    writ_wrpkt (ptr_2chcb);
	if (ptr_2chcb -> apwr_que == NULL)
	    if (strt_wrpkt (ptr_2chcb) == FALSE)
		break;          /* no resources, abort   */
	cur_byt += mv_buf2pkt (ptr_2chcb, buf_seg,
		offclc (buf_off, cur_byt), (byte_cnt - cur_byt));
    }
    if (ptr_2chcb -> tc.forwrd_tim == 0)
	if (ptr_2chcb -> apwr_tot > 0)
	    writ_wrpkt (ptr_2chcb);
    return (cur_byt);
}



/* -------------------------------------------------------------------------
   This routine allows the caller to send a 1-byte value, in a bufferlet.
   Important to increment byte count, will be decremented on transmission.
   ------------------------------------------------------------------------- */
void send_byte (out_byte)
int     out_byte;
{
    extern QUE_PTR get_que();
    extern void    link_write();

    QUE_PTR tque;

    int_off ();
    tque = get_que (YES_BUFF);
    int_on ();
    if (tque != NULL) {
	dcb.ch[0].out_bytecnt += 1;
	tque -> u.cmd.bfirst -> data[0] = out_byte & 0xFF;
	tque -> u.cmd.bfirst -> num_bytes = 1;
	link_write (LWRIT_REP, tque);
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine checks out the current XOFF/XON status for the character chan.
   May write out XOFF/XON bytes, processing them in order to control the flow.
   'why_run' indicates whether normal run to CHECK need for XOFF/XON (on byte
   in or I/O status req), or FLUSH to clean up (termination of xoff control).
   If sent XOFF, and many more characters still came in, send another XOFF.
   ------------------------------------------------------------------------- */
void net_xoffprc (why_run)
int     why_run;
{
    int     cnt_quedup;

    if (dcb.port.xon_xoff == TRUE) {
	/* 
	 If haven't sent XOFF, and have either too many characters or queues
	 held, send an XOFF.  If have sent XOFF, and we are FLUSHing or have
	 dropped below lower threshold, send an XON.  If have sent XOFF, and
	 still have characters coming in, send another one.
	 */
	cnt_quedup = dcb.ch[0].in_que.ent_cnt + dcb.ch[0].out_que.ent_cnt;
	if (dcb.port.net_xoffed == FALSE) {
	    if ((cnt_quedup >= dcb.qmax_xoff)
		    || (dcb.ch[0].in_bytecnt >= dcb.bmax_xoff)) {
		dcb.port.net_xoffed = TRUE;
		dcb.port.xoff_point = dcb.ch[0].in_bytecnt;
		send_byte (XOFF);
	    }
	}
	else {
	    if (((cnt_quedup < dcb.qmin_xoff)
			&& (dcb.ch[0].in_bytecnt < dcb.bmin_xoff)) || (why_run == FLUSH)) {
		dcb.port.net_xoffed = FALSE;
		dcb.port.xoff_point = 0;
		send_byte (XON);
	    }
	    else
		if (dcb.ch[0].in_bytecnt > (dcb.port.xoff_point + 100)) {
		    dcb.port.xoff_point = dcb.ch[0].in_bytecnt;
		    send_byte (XOFF);
		}
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   This is called by the link layer when any character is input, to give the
   pad a chance to process (respond to) the character.  If the character is an
   XOFF and the character channel has XOFF/XON processing enabled, any output
   in process is stopped.  If the character is an XON and the character chan
   has XOFF/XON process enabled, output is restarted by calling link_write.
   The function returns TRUE if the char was 'eaten' by the xoff/xon controls.
   ------------------------------------------------------------------------- */
int     pad_xoffprc (bits7)
int     bits7;
{
    int     ate_thechar;

    ate_thechar = FALSE;
    if (dcb.port.xon_xoff != FALSE) {
	bits7 = bits7 & 0x7F;
	if (dcb.port.pad_xoffed == TRUE) {
	    if (bits7 == XON) {
		dcb.port.pad_xoffed = FALSE;
		cont_linkout ();
		dcb.ch[0].xmit_enble = TRUE;
		pad_write (&dcb.ch[0]);
		ate_thechar = TRUE;
	    }
	}
    /* else     REMOVED! as 2nd XOFF in a row was treated as data */
	if (bits7 == XOFF) {
	    dcb.port.pad_xoffed = TRUE;
	    dcb.ch[0].xmit_enble = FALSE;
	    hold_linkout ();
	    ate_thechar = TRUE;
	}
    }
    return (ate_thechar);
}



/* -------------------------------------------------------------------------
   This routine attempts to output any queued data to the packet layer.
   It passes the 'TRUE' length of packet, from byte field that contains it.
   If packet mode, and sending a normal data packet, decr. out byte count.
   ------------------------------------------------------------------------- */
void pad_write (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern QUE_PTR deque();
    extern void    link_write(),
		   pack_write();

    QUE_PTR tque;
    int     length;

    while ((ptr_2chcb -> xmit_enble) && (ptr_2chcb -> out_que.ent_cnt != 0)) {
	int_off ();
	tque = deque (&ptr_2chcb -> out_que);
	int_on ();

	/* 
	 If neither Q-bit or C-bits are on, must be data packet, update bytecnt.
	 Character mode packets are decremented when actually xmitted by LINK.
	 */
	length = tque -> DATLEN_FLD + 1;
	tque -> DATLEN_FLD = tque -> u.cmd.p_seqenc = 0;

	if (dcb.device.state == PACKET) {
	    if ((tque -> u.cmd.p_gfilci & Q_BIT) == 0)
		ptr_2chcb -> out_bytecnt -= length;
	    pack_write (ptr_2chcb -> pkt_chan, tque, length);
	}
	else
	    link_write (LWRIT_OUT, tque);
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine udpates the 'characters queued for output' count of the char.
   channel 0.  Must depend upon link layer to inform if xmit is OK.
   ------------------------------------------------------------------------- */
void chrs_sent (count)
int     count;
{
    dcb.ch[0].out_bytecnt -= count;
}



/* -------------------------------------------------------------------------
   This routine is called by the link layer in character mode, when a frame
   has been input.  Update character count: # of bytes ready to be read.
   Queue up the frame in the input queue of channel 0.
   ------------------------------------------------------------------------- */
void pad_in (tque)
QUE_PTR tque;
{
    extern void enque();

    BUF_PTR tbuff;

    tbuff = tque -> u.cmd.bfirst;
    while (tbuff != NULL) {
	dcb.ch[0].in_bytecnt += tbuff -> num_bytes;
	tbuff = tbuff -> bnext;
    }
    int_off ();
    enque (&dcb.ch[0].in_que, tque);
    int_on ();
    dcb.ch[0].recv_ready = TRUE;
    return;
}



/* -------------------------------------------------------------------------
   This routine is only called during PACKET mode.  Character mode frames are
   put directly into the input queue by the 'pad_in' function.

   This routine attempts to input any queued packet data from the packet layer.
   It increments system count of number of bytes available for read on channel.
   The routine will only read from the packet layer if there are not too many
   packets currently qued for input or output at the pad layer.
   ------------------------------------------------------------------------- */
void pad_read (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern QUE_PTR pack_read();
    extern void    enque();

    QUE_PTR tque;

    if (dcb.device.state == PACKET) {
	while (ptr_2chcb -> recv_ready) {
	    if (((ptr_2chcb -> in_que.ent_cnt + ptr_2chcb -> out_que.ent_cnt)
			>= dcb.qmax_xoff) || (ptr_2chcb -> in_bytecnt >= dcb.bmax_xoff))
		break;
	    tque = pack_read (ptr_2chcb -> pkt_chan);
	    if (tque != NULL) {
		if ((tque -> u.cmd.p_gfilci & Q_BIT) == 0)
		    ptr_2chcb -> in_bytecnt += (tque -> u.cmd.bfirst -> data[B_SIZE] + 1);

		int_off ();
		enque (&ptr_2chcb -> in_que, tque);
		int_on ();
	    }
	    else
		break;
	}
    }
    return;
}



/* -------------------------------------------------------------------------
   Setup the write control que and buffer pointers with new values, but need
   only get those fields with the buffer.  General values set by writ_wrpkt.
   This routine assumes the packet is used to hold data during packet mode.
   If the routine cannot get a que (or buff), returns FALSE, apwr_buf invalid.
   ------------------------------------------------------------------------- */
int     strt_wrpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern QUE_PTR get_que();

    int     got_pkt;

    int_off ();
    ptr_2chcb -> apwr_que = get_que (YES_BUFF);
    if (ptr_2chcb -> apwr_que != NULL) {
	got_pkt = TRUE;
	ptr_2chcb -> apwr_que -> u.cmd.p_id = NORM_DATA;
	ptr_2chcb -> apwr_buf = ptr_2chcb -> apwr_que -> u.cmd.bfirst;
	ptr_2chcb -> apwr_buf -> num_bytes = (dcb.device.state == CHARACTER) ? 0 : B_BYTE0;
    }
    else
	got_pkt = FALSE;
    int_on ();
    return (got_pkt);
}



/* -------------------------------------------------------------------------
   Link the current bufferlet into a new bufferlet.  If can't get new one,
   set maximum read/write control value to zero to stop any further use.  If
   got one, chain to the previous end-of-chain.  Either case, reset counters.
   If could not get a bufferlet, will return false, but apwr_buf still valid.
   ------------------------------------------------------------------------- */
int     next_wrbuf (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern BUF_PTR get_buff();

    int     got_buff;

    got_buff = FALSE;
    int_off ();
    ptr_2chcb -> apwr_buf -> bnext = get_buff ();
    if (ptr_2chcb -> apwr_buf -> bnext != NULL) {
	got_buff = TRUE;
	ptr_2chcb -> apwr_buf = ptr_2chcb -> apwr_buf -> bnext;
	ptr_2chcb -> apwr_buf -> num_bytes = 0;
    }
    int_on ();
    return (got_buff);
}



/* -------------------------------------------------------------------------
   Turn the current write packet over to the out_que, zero all pointers.
   If output is enabled for this channel, give packet layer first queued pkt.
   ------------------------------------------------------------------------- */
void writ_wrpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void enque();

 /* 
  If character state, add full character count to out total.  If packet mode
  and not a Q-bit or C-bit packet, i.e., a normal data packet, add to total.
  */
    if (dcb.device.state == CHARACTER)
	ptr_2chcb -> out_bytecnt += ptr_2chcb -> apwr_tot;
    else
	if ((ptr_2chcb -> apwr_que -> u.cmd.p_gfilci & (C_BIT + Q_BIT)) == 0)
	    ptr_2chcb -> out_bytecnt += ptr_2chcb -> apwr_tot;

 /* 
  Set up the data length field as amount - 1 (byte field), enque the que
  entry onto the end of the out_que list.  Reset the indices of 'apwr' fields.
  */
    ptr_2chcb -> apwr_que -> DATLEN_FLD = ptr_2chcb -> apwr_tot - 1;
    int_off ();
    enque (&ptr_2chcb -> out_que, ptr_2chcb -> apwr_que);
    ptr_2chcb -> apwr_que = NULL;
    ptr_2chcb -> apwr_buf = NULL;
    ptr_2chcb -> apwr_tot = 0;
    int_on ();
 /* 
  Call the packet-layer interface pad write routine to output all packets can.
  */
    pad_write (ptr_2chcb);
    return;
}



/* -------------------------------------------------------------------------
   This routine is called by application through p_driver's data_read.
   This routine sets up the 'aprd_...' variables for the channel indicated
   with the pointers to the first packet available from the channels read
   queue structures.  It gets data from the in_que, which receives it from
   pad_in (in character mode) or from pad_read (in packet mode).
   Will set aprd_... values to NULL if there is no data available to read.
   Can only call pack_read if packet layer has packets avail (recv_ready).
   ------------------------------------------------------------------------- */
void next_rdpkt (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern QUE_PTR deque();

    pad_read (ptr_2chcb);

    if (ECHO_ON (ptr_2chcb))
	if (ptr_2chcb -> in_que.ent_cnt == 0)
	    ptr_2chcb -> aprd_que = ptr_2chcb -> curr_echque;

    if (ptr_2chcb -> aprd_que == NULL) {
	int_off ();
	ptr_2chcb -> aprd_que = deque (&ptr_2chcb -> in_que);
	int_on ();
    }

    if (ptr_2chcb -> aprd_que != NULL) {
	ptr_2chcb -> aprd_ind = (dcb.device.state == CHARACTER) ? 0 : B_BYTE0;
	ptr_2chcb -> aprd_buf = ptr_2chcb -> aprd_que -> u.cmd.bfirst;
    }
    else {
	ptr_2chcb -> aprd_buf = NULL;
    }
    return;
}



/* -------------------------------------------------------------------------
   This routine moves the 'aprd_buf' pointer for the channel indicated.  If
   there are not more bufferlets in the current chain, the 'aprd_que' entry is
   released to the available pool, and the pointer values are zeroed.  If the
   que/buf pointed to are the permanent echo resources for TYM/MCI echo, keep.
   ------------------------------------------------------------------------- */
void next_rdbuf (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern void rel_blist(),
		rel_que();

    ptr_2chcb -> aprd_buf = ptr_2chcb -> aprd_buf -> bnext;
    ptr_2chcb -> aprd_ind = 0;
    if (ptr_2chcb -> aprd_buf == NULL) {
	if ((MCI_ECHO (ptr_2chcb)) || (TYMLCL_ECHO (ptr_2chcb))) {
	    if (ptr_2chcb -> aprd_que == ptr_2chcb -> curr_echque) {
		ptr_2chcb -> curr_echque = NULL;
		ptr_2chcb -> curr_echbuf = NULL;
	    }
	    if (ptr_2chcb -> aprd_que == ptr_2chcb -> perm_echque) {
		if (ptr_2chcb -> perm_echque -> u.cmd.bfirst -> bnext != NULL) {
		    int_off ();
		    rel_blist (ptr_2chcb -> perm_echque -> u.cmd.bfirst -> bnext);
		    int_on ();
		    ptr_2chcb -> perm_echque -> u.cmd.bfirst -> bnext = NULL;
		}
	    }   
	    else {
		int_off ();
		rel_que (ptr_2chcb -> aprd_que, YES_BUFF);
		int_on ();
	    }
	}
	else {
	    /*
	    If find that reading from perm_echque and not local echo, must have
	    had echo turned off before echoed chars read.  Release que/buf and
	    also set perm_ech que/buf to NULL, no longer echoing. 
	    */
	    if (ptr_2chcb -> aprd_que == ptr_2chcb -> perm_echque)
		ptr_2chcb -> perm_echque = NULL;

	    int_off ();
	    rel_que (ptr_2chcb -> aprd_que, YES_BUFF);
	    int_on ();
	}
	ptr_2chcb -> aprd_que = NULL;
    }

    return;
}



/* -------------------------------------------------------------------------
   out_flush accepts a chan^. Throws away all data waiting for packet write,
   unless finds a q-bit packet.  Only data packets are thrown out, and only
   the data packet byte count can be deducted from the out_bytecnt total.
   ------------------------------------------------------------------------- */
void out_flush (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern QUE_PTR last_que();
    extern void    rel_que();

    int_off ();
    while (ptr_2chcb -> out_que.ent_cnt != 0) {
    /* if ((ptr_2chcb->out_que.last->u.cmd.p_gfilci & Q_BIT) == 0) { */
	ptr_2chcb -> out_bytecnt -= ptr_2chcb -> out_que.last -> DATLEN_FLD + 1;
	rel_que (last_que (&ptr_2chcb -> out_que), YES_BUFF);
    /* } else break;  */
    }

    if (ptr_2chcb -> apwr_que != NULL) {
	ptr_2chcb -> out_bytecnt -= ptr_2chcb -> apwr_que -> DATLEN_FLD + 1;
	rel_que (ptr_2chcb -> apwr_que, YES_BUFF);
    }

    int_on ();
    ptr_2chcb -> apwr_que = NULL;
    ptr_2chcb -> apwr_buf = NULL;
    ptr_2chcb -> apwr_ind = ptr_2chcb -> apwr_tot = 0;
    return;
}



/* -------------------------------------------------------------------------
   in_flush accepts a chan^.  Throws away all data waiting for appl read for
   character state, throws away all data packets waiting for appl read: PACKET.
   ------------------------------------------------------------------------- */
void in_flush (ptr_2chcb)
CHCB_PTR ptr_2chcb;
{
    extern QUE_PTR deque();
    extern void    rel_que();

    if (dcb.device.state == CHARACTER) {
	int_off ();
	rel_qhead (&ptr_2chcb -> in_que);
	if (ptr_2chcb -> aprd_que != NULL)
	    rel_que (ptr_2chcb -> aprd_que, YES_BUFF);
	ptr_2chcb -> in_bytecnt = 0;
	int_on ();

	ptr_2chcb -> aprd_que = NULL;
	ptr_2chcb -> aprd_buf = NULL;
	ptr_2chcb -> aprd_ind = ptr_2chcb -> aprd_tot = 0;
    }
    else {
	if (ptr_2chcb -> aprd_que != NULL) {
	/* &&  ((ptr_2chcb->aprd_que->u.cmd.p_gfilci & (C_BIT + Q_BIT)) == 0)) */
	    ptr_2chcb -> in_bytecnt -=
		(ptr_2chcb -> aprd_que -> u.cmd.bfirst -> data[B_SIZE] + 1);
	    int_off ();
	    rel_que (ptr_2chcb -> aprd_que, YES_BUFF);
	    int_on ();
	    ptr_2chcb -> aprd_que = NULL;
	    ptr_2chcb -> aprd_buf = NULL;
	    ptr_2chcb -> aprd_ind = ptr_2chcb -> aprd_tot = 0;
	}

	if (ptr_2chcb -> aprd_que == NULL)
	    while (ptr_2chcb -> in_que.ent_cnt != 0) {
/*    if ((ptr_2chcb->in_que.first->u.cmd.p_gfilci & (C_BIT + Q_BIT)) == 0) { */
		ptr_2chcb -> in_bytecnt -=
		    (ptr_2chcb -> in_que.first -> u.cmd.bfirst -> data[B_SIZE] + 1);
		int_off ();
		rel_que (deque (&ptr_2chcb -> in_que), YES_BUFF);
		int_on ();
	    /*      } else break;  */
	    }
    }
    return;
}



/* -------------------------------------------------------------------------
   Move data bytes, if any available, from bufferlets into the buffer pointed
   to by buffseg & buffoff.  If no data or no room for data, will move zero.
   Moves only data from current que/bufferlets pointed to 'aprd_que'.
   Returns the number of bytes moved from the packets to the buffer.
   ------------------------------------------------------------------------- */
int     mv_pkt2buf (ptr_2chcb, buffseg, buffoff, max_iobuff)
CHCB_PTR ptr_2chcb;
char   *buffseg;
char   *buffoff;
int     max_iobuff;
{
    extern void mvbyt_to();

    int     temp1,
	    temp2;
    int     mov_amt;
    int     pos_iobuff = 0;

    while (TRUE) {
	/* 
	 Find out how many bytes to move by taking mimimum of bytes remaining in
	 the application buffer and bytes remaining in the current bufferlet.
	 Call ASM routine to move bytes across segments, aprd_tot tells the
	 routine which position within appl buffer to begin filling at.
	 */
	temp1 = (max_iobuff - pos_iobuff);
	temp2 = (ptr_2chcb -> aprd_buf -> num_bytes - ptr_2chcb -> aprd_ind);

	mov_amt = min (temp1, temp2);
	if (mov_amt > 0)
	    mvbyt_to (buffseg, offclc (buffoff, pos_iobuff),
		    &ptr_2chcb -> aprd_buf -> data[ptr_2chcb -> aprd_ind], mov_amt);
	pos_iobuff += mov_amt;
	ptr_2chcb -> aprd_tot += mov_amt;
	ptr_2chcb -> aprd_ind += mov_amt;

	if (ptr_2chcb -> aprd_ind >= ptr_2chcb -> aprd_buf -> num_bytes) {
	    /* 
	     When index of current empty buffer is greater than or equal to
	     number bytes in buffer, done with the bufferlet.  If no more
	     bufferlets, exit.
	     */
	    next_rdbuf (ptr_2chcb);
	    if (ptr_2chcb -> aprd_buf == NULL)
		break;          /* Moved all we can, return amount. */
	}

    /* 
     The move operation either used 'mov_amt' as the remaining bytes in the
     bufferlet (handled above), or as the remaining bytes in the application
     buffer (handled here).  If filled the application buffer, return count.
     If did not exit above due to empty or no bufferlet data, and do not
     exit here due to full buffer, continue loop at top.
     */
	if (pos_iobuff >= max_iobuff)
	    break;
    }
    return (pos_iobuff);
}



/* -------------------------------------------------------------------------
   Move data from pointed to buffer into bufferlets, up to: maximum packet data
   byte count (DEF_pktlim); total_tomov reached; or, no bufferlets left
   to put data into.  Return number of bytes moved from buffer to packet.
   ------------------------------------------------------------------------- */
int     mv_buf2pkt (ptr_2chcb, buff_seg, buff_off, total_tomov)
	CHCB_PTR ptr_2chcb;
int    *buff_seg;
int    *buff_off;
int     total_tomov;
{
    extern void mvbyt_fr();

    int     temp1,
	    temp2,
	    temp3;
    int     mov_amt;
    int     current_tot;

    current_tot = 0;
    while (TRUE) {
	/* 
	 Find out how many bytes to move by taking mimimum of bytes remaining in
	 the application buffer and bytes available in the current bufferlet.
	 Call ASM routine to move bytes across segments, apwr_tot tells the
	 routine which position within appl buffer to start copy operation from.
	 */
	temp1 = (DEF_pktlim - ptr_2chcb -> apwr_tot);
	temp2 = (BFLT_DATA_SIZE - ptr_2chcb -> apwr_buf -> num_bytes);
	temp3 = (total_tomov - current_tot);
	mov_amt = min (min (temp1, temp2), temp3);
	if (mov_amt > 0)
	    mvbyt_fr (buff_seg, offclc (buff_off, current_tot),
		    &ptr_2chcb -> apwr_buf -> data[ptr_2chcb -> apwr_buf -> num_bytes], mov_amt);
	ptr_2chcb -> apwr_tot += mov_amt;
	ptr_2chcb -> apwr_buf -> num_bytes += mov_amt;/* update bufferlet byte cnt. */
	current_tot += mov_amt;
    /* 
     The move operation either used 'mov_amt' as the remaining bytes in the
     bufferlet (handled below), or the remaining possible byte count, as set
     by: packet size exceeded or no data left in appl. buffer (handled here).
     */
	if (ptr_2chcb -> apwr_tot >= DEF_pktlim)
	    break;
	if (current_tot >= total_tomov)
	    break;
    /* 
     Didn't terminate due to lack of data or room, so check bufferlet full.
     When index of current output buffer is greater than or equal to the
     number of bytes allowed in a bufferlet, current bufferlet full.  Try
     to get another bufferlet, return if no available bufferlets.
     */
	if (ptr_2chcb -> apwr_buf -> num_bytes >= BFLT_DATA_SIZE)
	    if (next_wrbuf (ptr_2chcb) == FALSE)
		break;
    }
    return (current_tot);
}
