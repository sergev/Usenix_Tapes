/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/

/* --------------------------------------------------------------------------
		     X.PC Control Block Structures Definitions
			      filename = lcbccb.h
   This include file contains the definition of the structures of the control
   blocks used throughout X.PC code.  Note that to include this file, the
   user must precede the include with include of 'p_define.d', 'p_bufque.s'.

	Date	By    Reason
      06/04/84	curt  Initial conversion to PAD structures.
      07/18/84	mike  Change grouping of ccb for reset/clear .
      07/31/84	curt  Released version 1.0.
      08/31/84	both  Removed many fields of ccb, release version 1.02.
      10/04/84	Mike  Moved time_run fields of ccb, to prevent 0 by reset
      10/02/86  Ed M  renamed lcbccb.h, added refs to externs to get rid of
                      ".e" files.
   ------------------------------------------------------------------------- */



/* -------------------------------------------------------------------------
   A 'ccb' is a channel control block, and one copy will be allocated per
   channel supported.  The information in a ccb is used to keep track of
   what is happening to the channel, at any point in time.
   ------------------------------------------------------------------------- */
typedef struct ccb_type {
   int	    circ_state;    /* 00 state of circuit, see x_pktlyr.d for bits.  */
   int	    time_run;	   /* 02 timeout send, idle, reply...	see pktlyr.d */
   byte     r_state;	   /* 04 restart states 1-3			     */
   byte     p_state;	   /* 05 switched Virtual Call (VC) states 1-7	     */
   byte     d_state;	   /* 06 data transfer states 1-3		     */
   byte     cur_state;	   /* 07 current state (r=0-2, p=3-9, d=10-12)	     */
   QHEAD    read_que;	   /* 08 Packet layers holding queue for incoming.   */
   QHEAD    writ_que;	   /* 10 Packet layers holding queue for outgoing.   */
   QHEAD    hold_que;	   /* 18 Packet layers holding queue for retransmit. */
   int	    dat_window;    /* 20 send-ahead window, default in x_config.d.   */
/*	    X.25 international fa   cilities on subscription basis   */
   int	    pkt_window;    /* 22 send-ahead cntrol+data window, default = 8. */
   int	    pkt_size;	   /* 24 packet size, default in x_config.d.	     */
   byte     early_rep;	   /* 26 send acknowledgement after x packets receiv */
   byte     x_lci;	   /* 27 X.PC logical channel number		     */
   byte     lc_vc;	   /* 28 (switched) virtual call, not permanent VC.  */
   byte     lc_2way;	   /* 29 two-way switched calls 		     */
   byte     in_error;	   /* 2A packet requires error diagnostic	     */
   byte     next_reply;    /* 2B next reply pack_in sends, REP_REJ/RNR/RR    */
/*	    The following are cleared by a restart, clear or reset,
	    except retrys variables for packet layer's receive state  */
   int	    recv_state;    /* 2C recv state confirms delivery of RR,REJ,RNR  */
   int	    vrl;	   /* 2E recv lowest  seq number for this window     */
   int	    vr; 	   /* 30 recv state var, incrm per recv, output P(R) */
   int	    vrh;	   /* 32 recv highest seq number for packet window   */
   int	    qued_in;	   /* 34 number of packets queued (to in/read que)   */
   int	    data_in;	   /* 36 numb data packets queued, since recv rotate */
   unsigned in_crc2;	   /* 38 input CRC2 total, from CRC1 to buffer end   */
   byte     in_pktindex;   /* 3A packet index made from pkt ID		     */
   byte     int_sent;	   /* 3B TRUE is Intr sent			     */
   byte     rnr_sent;	   /* 3C TRUE is RNR sent. reason is in left nibble  */
   byte     rej_sent;	   /* 3D TRUE reject packet sent.		     */
/*	    variables for packet layer's send state  */
   int	    send_state;    /* 3E send state stops output, if exception cond. */
   int	    vsl;	   /* 40 lowest  P(S) sent, awaiting acknowledgement */
   int	    vs; 	   /* 42 send state var, incrm per send, output P(S) */
   int	    vsh;	   /* 44 highest P(S) sent, awaiting acknowledgement */
   int	    qall_out;	   /* 46 number of packets queued (to writ/out/hold).*/
   int	    qued_out;	   /* 48 numb data packets queued (to writ/out/hold).*/
   unsigned out_crc2;	   /* 4A output CRC2 total, from CRC1 to buffer end  */
   byte     int_recv;	   /* 4C TRUE is Intr received			     */
   byte     rnr_recv;	   /* 4D TRUE is RNR received.			     */
   byte     rej_recv;	   /* 4E TRUE reject packet recv.		     */
   byte     retry;	   /* 4F packet send N1, after T1 timeout ends.      */
   byte     retry20[9];    /* 50 retry counts for packet types (R20-R28)     */
/*	    ***********   COMMENTED  OUT  !!!  **********
	    The following are to be used for Virtual Calls (not in release 1)
   byte     flow_control;   *	 flow control param negotiat (window/packet) *
   byte     thru_class;     *	 throughput class param negotiat (line-speed)*
   byte     incall_bar;     *	 incomming calls barred facility	     *
   byte     outcall_bar;    *	 outgoing calls barred facility 	     *
   byte     user_group;     *	 closed user group facility		     *
   byte     rev_charge;     *	 reverse charging acceptance facility	     *
   byte     fast_select;    *	 fast select acceptance facility	     *
   byte     lc_assign;	    *	 logical channel assignment facility	     *
   byte     deliv_conf;     *	 delivery confirmation bit modific. facility *
   byte     line_speed;     *	 default throughput class facility	     *
	    *  X.25 International per-call facilities *
   byte     pc_rev_charge;  *	 reverse charging acceptance facility	     *
   byte     pc_fast_select; *	 fast select acceptance facility	     *
   byte     pc_flow_control;*	 flow control param negotiat (window/packet) *
   byte     pc_user_group;  *	 closed user group facility		     *
   byte     pc_thru_class;  *	 throughput class param negotiat (line-speed)*
   byte     pc_rpoa_gateway;*	 RPOA selection facility		     *
	       ***********	COMMENTED  OUT	!!!  ********** 	     */
}  CCB_STR, *CCB_PTR;	   /* 5A typedef ccb_type */



/* -------------------------------------------------------------------------
		   Link Control Block Structure
   The 'lcb' is the global control block that is used throughout X.PC code.
   ------------------------------------------------------------------------- */

typedef struct lcb_type {
   int	    driver_mode;   /* 00 Mode the Driver is in. 		     */
   QHEAD    in_que;	   /* 02 link input queue head, all channels	     */
   QHEAD    rep_que;	   /* 0A link reply output queue head, all channels  */
   QHEAD    out_que;	   /* 12 link write output queue head, all channels  */
   QHEAD    time_que;	   /* 1A link timer queue head, all channels	     */
   QHEAD    err_que;	   /* 22 error queue for System Manager error message*/
   int	    ch_incall;	   /* 2A incoming calls ok: bit n for channel n.     */
   int	    ch_outcall;    /* 2C outgoing calls ok: bit n for channel n.     */
   int	    ch_open;	   /* 2E channel is open at all (set and used by LINK*/
   int	    xmit_busy;	   /* 30 prevent link_write from starting 2nd output */
   int	    xpc_nrdy;	   /* 32 when TRUE, all chans must send RNR, stop rec*/
   int	    in_synch;	   /* 34 input sychronized after CRC1		     */
   int	    in_size;	   /* 36 max. size of current input frame.	     */
   int	    in_count;	   /* 38 count of bytes in input frame (incl. link hd*/
   QUE_PTR  inq_entry;	   /* 3A current input queue entry (before enqueue)  */
   BUF_PTR  inb_ptr;	   /* 3C pointer to put_byte's current bufferlet,    */
   unsigned in_crc;	   /* 3E input CRC total, computed as receiving      */
   QUE_PTR  outq_entry;    /* 40 current output queue entry (after dequeue)  */
   BUF_PTR  outb_ptr;	   /* 42 pointer to get_byte's current bufferlet.    */
   int	    outb_index;    /* 44 get_byte's index into outb_ptr.             */
   unsigned out_crc;	   /* 46 output CRC total, computation area.	     */
   int	    htime;	   /* 48 time 1-128 for baud rates 9600,4800,..,1200,*/
   int	    toss_cnt;	   /* 4A toss n bytes before trying re-sych in pack m*/
   int	    pkt_inproc;    /* 4C flag is TRUE when pack_in is processing a pk*/
   int	    chr_framtim;   /* 4E used by 'chr_framend'-time to start new fram*/
   int	    char_timout;   /* 50 # of events between call to chr_framtim.    */
   int	    xmit_blocked;  /* 52 TRUE when charmode input sees XOFF, PAD sets*/
   CCB_PTR  ccbp[16];	   /* 54 a pointer to channel n's CCB, unless NULL.  */
   int	time_on;	   /* 74 timer on (bit TIM_XMIT), reset for each fram*/
   int	lin_error;	   /* 76 current line error status, reset for each fr*/
   int	mod_status;	   /* 78 current modem status,	    reset for each fr*/
   int	num_break;	   /* 7A total number of BREAK received, reset at sta*/
   int	num_overrun;	   /* 7C total number of overrun errors, reset at sta*/
   int	num_framerr;	   /* 7E total number of framing errors, reset at sta*/
   int	num_chrlost;	   /* 80 total number of input char lost, reset at st*/
   int	num_outlost;	   /* 82 total number of output int lost, reset at st*/
   BUF_ENTR in_buff;	   /* 84 space for supervis (RR,RNR,REJ) input buffer*/
   BUF_ENTR out_buff;	   /* A4 space for supervisory output bufferlet      */
}  LCB_STR, *LCB_PTR;	   /* C4 typedef lcb_type */

extern LCB_STR lcb;

