/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/
/* -------------------------------------------------------------------------
	   Virtual Terminal PAD Control Block Structures Definitions
			      filename = pad.h

   This include file contains the definition of the structures of the control
   blocks used throughout the VTPAD code.  
   -------------------------------------------------------------------------
	Date	Change	By    Reason
      05/10/84	  00	curt  Initial Generation of file.
      07/31/84	  01	curt  Version for Release 1.00.
      08/31/84	  02	curt  Many name changes, also Release 1.02.
      04/03/85	  03	curt  Added MCI echo/edit control fields and buffers.
      07/28/85	  05	curt  Completed MCI echo/edit changes.
      10/02/86    06    Ed M  Crunched p_padstr.s, p_padfnc.d, p_padlyr.d, and 
                              p_qbit.d into this file.
   ------------------------------------------------------------------------- */



/* -------------------------------------------------------------------------
		  Parameter Pointer Block Structure
   This structure maps the layout of a parameter block used to pass parameters
   from the application to the driver.	The 'param' fields will be offsets,
   within the segment noted in 'param_seg', of the user variables or arrays
   that contain the additional information needed for a particular call.
   ------------------------------------------------------------------------- */
typedef  struct   ppb_type {
   int	 *param_seg;	   /* segment in which optional parameters reside.   */
   int	 *func_code;	   /* Pointer to Function code (service to perform). */
   int	 *device;	   /* Pointer to Device number for this function.    */
   int	 *channel;	   /* Pointer to Channel number for this function.   */
   int	 *status;	   /* Pointer to Status returned by this function.   */

   int	 *param1;	   /* Pointer to Parameter 1 (used to reach value).  */
   int	 *param2;	   /* Pointer to Parameter 2 (used to reach value).  */
   int	 *param3;	   /* Pointer to Parameter 3 (used to reach value).  */
   int	 *param4;	   /* Pointer to Parameter 4 (used to reach value).  */
}  PPB_STR, *PPB_PTR;	   /* typedef ppb_type	*/

extern PPB_STR params;

/* -------------------------------------------------------------------------
		  Device Status Structure
   This structure contains information about a devices' state, version, etc.
   The application may request a copy of the structure.
   ------------------------------------------------------------------------- */

typedef  struct dss_type {
   int	 state; 	   /* device state: installed, reset/wait, char, pkt.*/
   int	 version;	   /* version of X.PC/VTPAD this device implements.  */
   int	 revision;	   /* revision within above version implemented.     */
   int	 pad_avail;	   /* PAD capabilities supported by this device.     */
   int	 port_numb;	   /* Async port #: 0 (none), 1 (COM1), or 2 (COM2). */
   int	 PVC_channels;	   /* # of Permanent Virtual Call channels available.*/
   int	 INC_channels;	   /* # of Incoming Virtual Call channels available. */
   int	 TWOC_channels;    /* # of Two-way Virtual Call channels available.  */
   int	 OUTC_channels;    /* # of Outgoing Virtual Call channels available. */
}  DSS_STR, *DSS_PTR;	   /* typedef dss_type */



/* -------------------------------------------------------------------------
		   Port Control Block Structure
   The 'pcb' structure contains information about an ASYNC port, use by VTPAD.
   ------------------------------------------------------------------------- */
typedef struct pcb_type {
   int	 baud;		/* baud rate chip is currently set to.		     */
   int	 data_bits;	/* Number of bits of data in each Async. frame.      */
   int	 parity;	/* No parity, odd, even, mark, space parity flag.    */
   int	 stop_bits;	/* Number of stop bits in each Async. frame.	     */
   int	 xon_xoff;	/* TRUE: character mode is xon/xoff flow controlled. */
   int	 DTE_port;	/* TRUE: run packet layer as DTE, otherwise as DCE.  */

   int	 pad_xoffed;	/* TRUE: network has auto-XOFFed the pad and appl.   */
   int	 net_xoffed;	/* TRUE: pad char chan HAS auto-XOFFed the network.  */
   int	 xoff_point;	/* char count at time of last XOFF transmission.     */
   int	 xoff_timer;	/* # of events between XOFF checks.		     */
   int	 char_parity;	/* parity in effect while in character mode state.   */
   int	 char_databits; /* # data bits in effect while in "     "     "  .   */
   int	 data_error;	/* TRUE if a parity or framing error has occured.    */
}  PCB_STR, *PCB_PTR;	/* typedef pcb_type */

/* this define creates a machine-dependent offset, showing length of params  */
#define PORT_PRMLEN 12




/* -------------------------------------------------------------------------
	      Terminal (channel) Characteristics and Control Block
   This structure contains fields used to define the characteristics of the
   PAD logical port, as well as fields used for runtime control of the port.
   ------------------------------------------------------------------------- */
typedef  struct   termchar_type {
   int	 echo_ok;		 /* TRUE, application allows echo enable.    */
   int	 echo_enb;		 /* TRUE, port running under echo rules.     */
   int	 def_echo;		 /* TRUE, currently in deferred echo mode.   */
   int	 echomci_on;		 /* TRUE, doing MCI permanent local echo.    */
   int	 editmci_on;		 /* TRUE, doing MCI local edit processing.   */
   int	 ech_ctli;		 /* TRUE, echo ^I. If FALSE, defer echo.     */
   int	 ech_ctlh;		 /* TRUE, echo ^H. If FALSE, defer echo.     */
   int	 ech_escp;		 /* TRUE, echo ESC.  If FALSE, defer echo.   */
   int	 ech_lfoncr;		 /* TRUE, echo free LF for each CR input.    */
   int	 ech_cronlf;		 /* TRUE, echo CR:RUB for each LF input.     */
   int	 forwrd_chr;		 /* if output, will cause send of held chars.*/
   int	 forwrd_tim;		 /* if expires, will cause send of held char.*/

   int	 io_active;		 /* TRUE, have done I/O lately. 	     */
   int	 green_out;		 /* TRUE, have sent a green ball.	     */
   int	 red_out;		 /* TRUE, have sent a red ball. 	     */
   int	 parity_enb;		 /* TRUE, only handle 7 bits (xmit/recv 8!!).*/

   char  backspc;		 /* local edit: echo backspace/space/backspac*/
   char  linedel;		 /* local edit: erase line, echo CR, LF.     */
   char  linagin;		 /* local edit: echo CR, LF, all of editbuf. */
}  TRMC_STR, *TRMC_PTR; 	 /* typedef termchar_type */



/* -------------------------------------------------------------------------
		    Channel Control Block
   This control block contains the information about an application's channel.
   It contains input and output queues, the call state, and the terminal char.
   ------------------------------------------------------------------------- */
typedef  struct   chcb_type {
   int	    call_state;       /* state the logical channel is in currently.  */
   int	    clear_code;       /* when channel cleared, reason given in packet*/
   int	    chan_type;	      /* is either PERM_CH or VIRT_CH.		     */
   int	    pkt_chan;	      /* packet layer's concept of channel number.   */

   int	    recv_ready;       /* TRUE, packet data ready to be read by appl. */
   int	    in_bytecnt;       /* # of bytes queued up for application read.  */
   QHEAD    in_que;	      /* 'packets' received from the remote computer.*/
   QUE_PTR  aprd_que;	      /* Appl. read gets data from this que entry.   */
   BUF_PTR  aprd_buf;	      /* Appl. read first bufferlet in aprd_que.     */
   int	    aprd_ind;	      /* Appl. read start index within aprd_buf.     */
   int	    aprd_tot;	      /* total bytes read from this packet so far.   */
   int	    break_recvd;      /* TRUE: break was received since last I/O req.*/

   int	    xmit_enble;       /* TRUE, appl. may write to packet layer.      */
   int	    out_bytecnt;      /* # of bytes queued for packet layer output.  */
   QHEAD    out_que;	      /* 'packets' to be sent to the remote computer.*/
   QUE_PTR  apwr_que;	      /* Appl. write puts data into this que entry.  */
   BUF_PTR  apwr_buf;	      /* Appl. write first bufferlet in apwr_que.    */
   int	    apwr_ind;	      /* Appl. write start index within apwr_buf.    */
   int	    apwr_tot;	      /* total bytes written into this packet so far.*/

   QUE_PTR  perm_echque;      /* if !NULL, permanent que/buf for echoing use.*/
   QUE_PTR  curr_echque;      /* if !NULL, current que putting echo char into*/
   BUF_PTR  curr_echbuf;      /* if !NULL, current buf putting echo char into*/

   int	    *seg_timint;      /* if !NULL, code seg and offset of routine to */
   int	    *off_timint;      /* be run when a timer expires.		     */
   int	    *seg_timupd;      /* if !NULL, data seg and offset of address to */
   int	    *off_timupd;      /* set to 0 when a timer expires. 	     */
   int	    *seg_ckpupd;      /* if !NULL, data seg and offset of address to */
   int	    *off_ckpupd;      /* set to 0 when a checkpoint function finishes*/
   TRMC_STR tc; 	      /* Terminal charactistics supported on this ch.*/
}  CHCB_STR, *CHCB_PTR;



/* -------------------------------------------------------------------------
			Device Control Block Structure
   A 'dcb' is a Device control block, and one copy will be allocated per
   device supported, and it indicates the state and proper uses of the device.
   A 'device' is that logical unit accessible by the XPC driver, supporting a
   physical link via a communication port, and up to 16 channels on that port.
   ------------------------------------------------------------------------- */
typedef struct dcb_type {
   DSS_STR  device;	   /* Structure describing the status of the device. */
   PCB_STR  port;	   /* Structure describing the I/O port selected.    */
   int	    chan_cnt;	   /* current # of connected channels.		     */

   CHCB_STR ch[16];	   /* a Pad Control Block for each channel.	  */
   CHCB_PTR logch_map[16]; /* N = PKTchan, logch_map[N] points to ch[PADchan]*/

   char     io_buff[BUF_MAX];	/* buffer used to do I/O to/from user buff.  */
   int	    ed_active;		/* one channel may run MCI edit at a time.   */
   int	    ed_pos;		/* next byte to use for edit use w/ed_buff   */
   char     ed_buff[BUF_MAX];	/* buffer used by single channel in MCI edit.*/
   char     *ed_DSbuff; 	/* Data Segment that Ed_buff is in.	     */

   int	    bpnd_pkout;    /* max # bytes unwritten can que before xmit FALSE*/
   int	    bmax_xoff;	   /* max # bytes unread can queue before XOFF/trash.*/
   int	    bmin_xoff;	   /* min # bytes unread with XOFF on before XON.    */
   int	    qmax_xoff;	   /* max # of ques in use before XOFF processing.   */
   int	    qmin_xoff;	   /* max # of ques in use before XOFF processing.   */

   int	    *seg_mdmupd;   /* if !NULL, data seg and offset of address where */
   int	    *off_mdmupd;   /* the modem stat reg will be written on 8250 chg.*/

}  DCB_STR, *DCB_PTR;	   /* typedef dcb_type */

extern DCB_STR dcb;

/* -------------------------------------------------------------------------
		X.PC Driver/PAD Function Call Constants
   This file contains defines for use by the linkage module or application
   program that wants to ask the driver to do some function.  The FUNCTION
   CODE values are use with the function code parameter, the RETURN CODE
   values will be set into the status parameter.

		     FUNCTION CODE FIELD VALUES:
   ------------------------------------------------------------------------- */
#define  CLEAR_DEV    0 	 /* Clear the device indicated. 	     */
#define  READ_DEVST   1 	 /* Read the status of the device indicated. */
#define  RESET_DEV    2 	 /* Reset the device indicated. 	     */
#define  SET_CHRMOD   3 	 /* Set the device indicated to char. mode.  */
#define  SET_PKTMOD   4 	 /* Set the device indicated to pkt. mode.   */
#define  SET_PORTPRM  5 	 /* Set the parameters of the port indicated.*/
#define  REP_PORTPRM  6 	 /* Read the parameters of the port indicated*/
#define  SET_DTR_ON   7 	 /* Set the DTR signal ON.		     */
#define  RESET_DTR    8 	 /* Set the DTR signal OFF.		     */
#define  SET_RTS_ON   9 	 /* Set the RTS signal ON.		     */
#define  RESET_RTS   10 	 /* Set the RTS signal OFF.		     */
#define  TEST_DSR    11 	 /* Report the DSR signal: ON or OFF.	     */
#define  TEST_DCD    12 	 /* Report the Carrier Signal: ON or OFF.    */
#define  TEST_RING   13 	 /* Report the Ring Indicator: ON or OFF.    */
#define  SEND_BREAK  14 	 /* Send the appropriate BREAK for channel.  */
#define  REP_IOSTAT  15 	 /* Report the I/O status for the channel.   */
#define  READ_DATA   16 	 /* Read data from the channel indicated.    */
#define  WRITE_DATA  17 	 /* Write data to the channel indicated.     */
#define  REP_CHNSTAT 18 	 /* Report the 'call' status of the channel. */
#define  CALLRQ_SND  19 	 /* Send a call request, enter pending accpt.*/
#define  CALLCL_SND  20 	 /* Send a call clear, enter pending confirm.*/
#define  SET_CALLANS 21 	 /* Set call answer, wait for call request.  */
#define  CALLAC_SND  22 	 /* Send a call accept, enter connected.     */
#define  READ_CALDAT 23 	 /* Read the data associated with call req.  */
#define  SET_INTRP   24 	 /* Enable invocation of appl. code on event.*/
#define  RESET_INTRP 25 	 /* Disable invocation of appl. code on event*/
#define  SET_UPDT    26 	 /* Enable auto-update of flag on event.     */
#define  RESET_UPDT  27 	 /* Disable auto-update of flag on event.    */
#define  REP_PADPARM 28 	 /* Read the PAD parameter indicated, return.*/
#define  SET_PADPARM 29 	 /* Set the PAD parameter indicated to value.*/
#define  INPUT_FLUSH 30 	 /* throws away all un-read input on a chan. */
#define  OUTPT_FLUSH 31 	 /* throws away all un-written output on chan*/
/*efine  REP_LNKSTAT 32 	  * report link stats (# errors, chars, etc.)*/

/* -------------------------------------------------------------------------
			  DEVICE STATES:
   ------------------------------------------------------------------------- */
#define  WAIT_RESET	1	 /* Device is in the 'wait reset' state.     */
#define  CHARACTER	2	 /* Device is in character state.	     */
#define  PACKET 	3	 /* Device is in packet state.		     */

/* -------------------------------------------------------------------------
		       PACKET CHANNEL STATES:
   ------------------------------------------------------------------------- */
#define  DISCONNECT	0	 /* Channel is disconnected.		     */
#define  CALLACC_PND	1	 /* Channel is pending call accept packet.   */
#define  CALL_ACCEPT	2	 /* Channel has received call accept packet. */
#define  CONNECTED	3	 /* Channel is connected.		     */
#define  CLEARCNF_PND	4	 /* Channel is pending clear confirm packet. */
#define  INCOMCAL_PND	5	 /* Channel is pending an incoming call pkt. */
#define  INCOME_CALL	6	 /* Channel has received incoming call pkt.  */
#define  ACCEPT_PND	7	 /* Channel is pending acceptance of call.   */
#define  CALL_CLEARED	8	 /* Channel call was cleared by remote.      */


/* -------------------------------------------------------------------------
		      CALL CLEARING CAUSE CODES:
   ------------------------------------------------------------------------- */
#define  CLR_NORMAL	0	 /* normal end of session, no errors.	     */
#define  CLR_NO_PORTS	1	 /* no ports available for login.	     */
#define  CLR_UNAVAIL	2	 /* host unavailable.			     */
#define  CLR_NO_ACCESS	3	 /* no access allowed to host.		     */
#define  CLR_NETUNAVAIL 4	 /* network unavailable.		     */
#define  CLR_FORM_ERR	5	 /* format error in call request data.	     */
#define  CLR_NAME_ERR	6	 /* username is in error in call request.    */
#define  CLR_ADDR_ERR	7	 /* address is in error in call request.     */
#define  CLR_PSWD_ERR	8	 /* password is in error in call request.    */
#define  CLR_TIMEOUT	9	 /* timeout waiting for clear confirm.	     */
#define  CLR_RESET     10	 /* packet level reset of a channel	     */
#define  CLR_RESTART   11	 /* packet level restart, all chans lost     */
#define  CLR_CRESTART  12	 /* packet level restart, return to char mode*/
#define  CLR_PKTLOST   13	 /* packet level timeout, remote XPC lost    */
#define  CLR_MDMLOST   14	 /* packet level modem, DSR or CD lost	     */


/* -------------------------------------------------------------------------
		       EVENT CODE VALUES:
   ------------------------------------------------------------------------- */
#define  TIMER_DONE	1	 /* update field when timer expires.	     */
#define  MODEM_CHG	2	 /* update field when modem signals change.  */
#define  CHECKPOINT	3	 /* update field when no output data queued. */

#define  EVENT_PSEC	6	 /* for timer, multiply # seconds times this.*/


/* -------------------------------------------------------------------------
		PAD PARAMETER SETTING VALUES:
   ------------------------------------------------------------------------- */
#define  NO		0	 /* use to turn OFF Boolean parameters.      */
#define  YES		1	 /* use to turn ON Boolean parameters.	     */

#define  ECHO_ENABLE	1	 /* YES, echo processing takes place.	     */
#define  CTRL_I_ECHO	2	 /* If ECHO_ENABLE and YES, echo ^I locally. */
#define  CTRL_H_ECHO	3	 /* If ECHO_ENABLE and YES, echo ^H locally. */
#define  ESC_ECHO	4	 /* If ECHO_ENABLE and YES, echo ESC locally.*/
#define  LFONCR_ECHO	5	 /* If ECHO_ENABLE and YES, free LF for CR.  */
#define  CRONLF_ECHO	6	 /* If ECHO_ENABLE and YES, free CR for LF.  */
#define  FRWRD_CHAR	7	 /* If ECHO_ENABLE, send held chars on char. */
#define  FRWRD_TIME	8	 /* if ECHO_ENABLE, will send if chars held. */
#define  PARITY_ENABLE 16	 /* If YES, treat data as 7 bits, I/O 8-bit. */


/* -------------------------------------------------------------------------
		     PORT PARAMETER SETTING VALUES:
   ------------------------------------------------------------------------- */
#define  BAUD_9600	7	    /* BAUD RATE SETTINGS...		     */
#define  BAUD_4800	6
#define  BAUD_2400	5
#define  BAUD_1200	4
#define  BAUD_600	3
#define  BAUD_300	2
#define  BAUD_150	1
#define  BAUD_110	0

#define  NO_PARITY	0	    /* PARITY SETTINGS...		     */
#define  ODD_PARITY	1
#define  EVEN_PARITY	2
#define  MARK_PARITY	3
#define  SPAC_PARITY	4

#define  DATA_5BITS	5	    /* NUMBER OF DATA BITS SETTINGS...	     */
#define  DATA_6BITS	6
#define  DATA_7BITS	7
#define  DATA_8BITS	8
#define  STOP_1BIT	0	    /* NUMBER OF STOP BITS SETTINGS...	     */
#define  STOP_15BIT	1
#define  STOP_2BIT	2

#define  PORT_DCE	0	    /* DXE SETTINGS...			     */
#define  PORT_DTE	1

/* -------------------------------------------------------------------------
		 RETURN CODE (STATUS) FIELD VALUES:
   ------------------------------------------------------------------------- */
#define  FUNC_COMPLETE	 0	 /* Function completed successfully, no error*/
#define  ILL_FUNCTION	 1	 /* Function code not recognized, no action. */
#define  ILL_DEVICE	 2	 /* Device Number of request not valid.      */
#define  ILL_CHANNEL	 3	 /* Channel Number of request not valid.     */
#define  ILL_WAITRQ	 4	 /* Illegal request, given wait reset state. */
#define  ILL_CHARRQ	 5	 /* Illegal request, given char. mode state. */
#define  ILL_PACKRQ	 6	 /* Illegal request, given packet mode state.*/
#define  ILL_PORTNUM	 7	 /* Port Address of request not valid.	     */
#define  ILL_BAUD	 8	 /* Baud rate of request not valid.	     */
#define  ILL_PARITY	 9	 /* Parity of request not valid.	     */
#define  ILL_BTDATA	10	 /* # of data bits of request not valid.     */
#define  ILL_BTSTOP	11	 /* # of stop bits of request not valid.     */
#define  ILL_DTEDCE	12	 /* Must specify either DTE or DCE config.   */
#define  PORT_ACTIVE	13	 /* Tried to reset port(n), without reset(0).*/
#define  IO_OVERFLOW	14	 /* Tried to write > 256 bytes this call.    */
#define  IO_BLOCKED	15	 /* Tried to write while flow control active.*/
#define  ILL_DATATYP	16	 /* Type of data written was not valid.      */
#define  ILL_CALFORM	17	 /* Invalid format for Call request function.*/
#define  ILL_CLRCODE	18	 /* Invalid Call Clearing cause code.	     */
#define  ILL_CALSTAT	19	 /* Illegal request, given call state.	     */
#define  NO_CHANSAVAIL	20	 /* No channels of type requested available. */
#define  ILL_PADPARAM	21	 /* Invalid Pad Parameter specified.	     */
#define  PCHANS_OPEN	22	 /* Can't leave Packet mode: channels open.  */
#define  ILL_EVENT	23	 /* Bad event-code, not implemented.	     */
#define  TIMER_RUNNING	24	 /* A timer is already running (int/upd).    */
#define  NO_RESOURCES	25	 /* No way to start a timer, no que entry.   */
#define  CHKPNT_RUNNING 26	 /* A checkpoint is already running (int/upd)*/


/* -------------------------------------------------------------------------
   Constants used through the PAD layer of the X.PC Driver.
   ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
   These are for decision making process in echo-control logic routines.
   ------------------------------------------------------------------------- */
#define  MCI_ECHO(chcb)    (chcb->tc.echomci_on)
#define  MCI_EDIT(chcb)    ((chcb->tc.echomci_on) && (chcb->tc.editmci_on))

#define  ECHO_ON(chcb) ((chcb->tc.echomci_on)||(chcb->tc.echo_enb&&(!chcb->tc.def_echo)))
#define  ECHO_OFF(chcb)    ((!chcb->tc.echo_enb) && (!chcb->tc.def_echo))
#define  TYMLCL_ECHO(chcb)  ((chcb->tc.echo_enb) && (!chcb->tc.def_echo))
#define  TYMDFR_ECHO(chcb)  ((!chcb->tc.echo_enb) && (chcb->tc.def_echo))
#define  GREEN_OUT(chcb)   (chcb->tc.green_out)
#define  RED_OUT(chcb)	   (chcb->tc.red_out)
#define  NO_BALLOUT(chcb)  ((!chcb->tc.green_out) && (!chcb->tc.red_out))
#define  NOBALL_OUT(chcb)  ((!chcb->tc.green_out) && (!chcb->tc.red_out))

#define  BALL_TIMER	   (EVNT_PSEC*12)
#define  NOIO_TIMER	   (EVNT_PSEC*6)
#define  NO_IOTIMER	    NOIO_TIMER

#define  MUST_EXIT   TRUE		  /* when Q-bit receive forces exit. */

#define  OPEN_CHAN   1			  /* adjust channel count upwards.   */
#define  CLOS_CHAN   (-1)		  /* adjust channel count downwards. */
#define  INIT_VALS   2			  /* initialize control values.      */

#define  DATLEN_FLD  u.cmd.p_id 	  /* store length until pack_write.  */
#define  CHRERR_FLD  u.cmd.p_seqenc	  /* store error codes from linkin.  */
#define  DATERR      1			  /* OR CHRERR_FLD with to show error*/
#define  BRKRCV      2			  /* OR CHRERR_FLD with this:  BREAK.*/

#define  ERROR_CHAR  0x3F	    /* '?' marks error in character input.   */

/* -------------------------------------------------------------------------
   These are to be used as parameters to the 'net_xoffprc' routine.
   ------------------------------------------------------------------------- */
#define  TIMER	 1			  /* run off of a timer.	     */
#define  CHECK	 2			  /* check during write oper.	     */
#define  FLUSH	 3			  /* ending xoff, clean up.	     */

#define  NORM_DATA 0			  /* packet layer notion of data type*/

/* -------------------------------------------------------------------------
   These values are used with for XOFF/XON logic, when processing is enabled.
   ------------------------------------------------------------------------- */
#define  XON   0x11
#define  XOFF  0x13


/* -------------------------------------------------------------------------
			Q-bit Data packet 'types' for VTPAD control

   The values defined here are for use with 'Q-bit' type data packets, as used
   by the X.PC PAD to perform TYMNET-type signals.  The signals implemented
   include Echo control, Logical Break, and psuedo-call request/accept/clear
   packets (for Permanent Virtual Circuit channels).

   The values indicated will be assigned to the data byte position in a 'short
   packet' header (packets with no data will consist of only a header). For
   those signals that require data (such as call request) the data length
   indicates how many bytes will be held in the second part of a 'long packet'
   (packets with 1 or more bytes of data will consist of header + trailer).
   ------------------------------------------------------------------------- */
#define  QENB_ECHO    0x01	 /* data len = 0, begin echoing this channel.*/
#define  QDSB_ECHO    0x02	 /* data len = 0, end echoing this channel.  */
#define  QENT_DEFECH  0x03	 /* data len = 0, begin deferred echo for ch.*/
#define  QLEV_DEFECH  0x04	 /* data len = 0, end deferred echo for ch.  */
#define  QGREEN_BALL  0x05	 /* data len = 0, 'prepared to end deferred'.*/
#define  QRED_BALL    0x06	 /* data len = 0, 'cancel any green ball'.   */

#define  QBRK_BEG     0x07	 /* data len = 0, begin logical break for ch.*/
#define  QBRK_END     0x08	 /* data len = 0, end logical break for ch.  */

#define  QCALL_REQ    0x09	 /* data len = length of login string.	     */
#define  QCALL_ACC    0x0A	 /* data len = length of call accept string. */
#define  QCALL_CLR    0x0B	 /* data len = 1, for clear cause code.      */
#define  QCLEAR_CNF   0x0C	 /* data len = 0, clear request confirmed.   */

#define  QYELLOW_BALL 0x0D	 /* data len = 0, send orange when no data.  */
#define  QORANGE_BALL 0x0E	 /* data len = 0, no data left unread by appl*/

#define  QMCI_PECHO   0x0F	 /* data len = 0, MCI enable permanent echo. */
#define  QMCI_DECHO   0X10	 /* data len = 0, MCI disable permanent echo.*/
#define  QCHR_FRWRD   0X11	 /* data len = 1, char to force forward.     */
#define  QTIM_FRWRD   0X12	 /* data len = 1, timeout to force forward.  */
#define  QMCI_YEDIT   0X13	 /* data len = 0, MCI enable local edit mode.*/
#define  QMCI_NEDIT   0X14	 /* data len = 0, MCI disable local edit mode*/
