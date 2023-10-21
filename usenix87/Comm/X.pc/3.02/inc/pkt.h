/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/

/*    10/30/86	BobC  Install meaningful values for R25 and R27. */

/* -------------------------------------------------------------------------
   System parameters
   -------------------------------------------------------------------------
var #define T1	4	     * max time between frames sent and reply recvd
	use 10 @ 1200 baud, 5 @ 2400 baud, 2.5 @ 4800 baud, 1.5 @ 9600 baud
var #define T2	2	     * max time delay after input, before reply sent
	use  8 @ 1200 baud, 4 @ 2400 baud, 2.0 @ 4800 baud, 1.0 @ 9600 baud  */
#define N1  128 	/* max number of bytes in a packet		     */
#define N2  10		/* max number of retransmissions		     */

/* -------------------------------------------------------------------------
    Miscellaneous X.PC protocol constants.
   ------------------------------------------------------------------------- */
#define STX 0x02		/* in all link layer frames */
#define IGNORE 0x80;		/* ignore XPC input packet */
#define INPUT_CRC 0x1D0F	/* input CRC total (includes CRC1 or CRC2)   */
#define CRC_POLYNOM 0x11021	/* Polynomial value used to generate CRC tbl.*/
#define INIT_CRC    0xFFFF	/* Initial value to begin CRC comp. with.    */
#define XPC_WRITE 0xF0		/* in gfilci for XPC calling pack_write */
#define MODULO 16		/* packet sequence numbering is 4 bits wide */
/*
   the following apply to any channel, in lcb.time_on
*/
#define TIM_XINT 0x01		/* lcb.timeon timeout for lost xmit inter */
#define TIM_EXPR 0x02		/* lcb.timeon timeout expired lost xmit inter*/

/* -------------------------------------------------------------------------
   Time-out constants in seconds.
   ------------------------------------------------------------------------- */
#define T20 180 	/* restart request */
#define T21 200 	/* call request */
#define T22 180 	/* reset request */
#define T23 180 	/* clear request */
#define T24 060 	/* window status */
#define T25 150 	/* window rotation */
#define T26 180 	/* interrupt packet */
#define T27 060 	/* reject packet */
#define T28 300 	/* registration request */

/* -------------------------------------------------------------------------
   Re-try constants for each time-out.
   ------------------------------------------------------------------------- */
#define R20 1		/* restart request */
#define R21 0		/* call request */
#define R22 1		/* reset request */
#define R23 1		/* clear request */
#define R24 0		/* window status */
#define R25 4		/* window rotation */
#define R26 0		/* interrupt packet */
#define R27 4		/* reject packet */
#define R28 1		/* registration request */

/* -------------------------------------------------------------------------
    these are used in ccb->cur_state
   ------------------------------------------------------------------------- */
#define R0  0	      /* restart ready */
#define R1  1	      /* restart ready */
#define R2  2	      /* DTE sent restart */
#define R3  3	      /* DCE sent restart */
#define P1  4	      /* call set-up and clearing ready */
#define P2  5	      /* DTE sent call request */
#define P3  6	      /* DCE sent call request */
#define P4  7	      /* data transfer */
#define P5  8	      /* both sent call request (call collision) */
#define P6  9	      /* DTE sent clear request */
#define P7 10	      /* DCE sent clear request */
#define D1 11	      /* flow control ready */
#define D2 12	      /* DTE sent reset request */
#define D3 13	      /* DCE sent reset request */

/* -------------------------------------------------------------------------
   Packet IDs may be shifted right 1 (bit 0 always on), and test odd / even:
     use even in jump table rr,rnr,rej,err,diag,...
     use odd for jump table int,intc,callr,callc,clrr...
   ------------------------------------------------------------------------- */
#define NORMAL_DATA 0x00	 /* used by PAD routines to send normal pkts.*/

#define RR_PKT	  0x01
#define INT_PKT   0x23
#define RNR_PKT   0x05
#define INTC_PKT  0x27
#define REJ_PKT   0x09
#define CALLR_PKT 0x0B
/*define ERR_PKT  0x0D */
#define CALLC_PKT 0x0F
#define DIAG_PKT  0xF1
#define CLRR_PKT  0x13
/*define ERR_PKT  0x15 */
#define CLRC_PKT  0x17
/*define ERR_PKT  0x19 */
#define RSETR_PKT 0x1B	/* bit 5 off: reset, on: restart request */
/*define ERR_PKT  0x1E */
#define RSETC_PKT 0x1F	/* bit 5 off: reset, on: restart confirm */

#define REGR_PKT  0xF3	/* registration request */
#define REGC_PKT  0xF7	/* registration confirm */
#define RSTRR_PKT 0xFB	/* restart request */
#define RSTRC_PKT 0xFF	/* restart confirm */

/* -------------------------------------------------------------------------
   Constants for indexing on packet type, after (pack_in's) pack_check.
   15-17 are outgoing state machine.  18 for packets that are not processed.
   ------------------------------------------------------------------------- */
#define DATA_TYP    1		/* Normal data contained in buffer */
#define INT_TYP     2		/* Interrupt Packet packet type */
#define INTC_TYP    3		/* Interrupt Confirm packet type */
#define RSETR_TYP   4		/* Reset Request packet type */
#define RSETC_TYP   5		/* Reset Confirm packet type */
#define CALLR_TYP   6		/* Call Request packet type  */
#define CALLC_TYP   7		/* Call Connect packet type  */
#define CLRR_TYP    8		/* Clear Request packet type */
#define CLRC_TYP    9		/* Clear Confirm packet type */
#define REGR_TYP   10		/* Registration Request packet type */
#define REGC_TYP   11		/* Resistration Confirm packet type */
#define RSTRR_TYP  12		/* Restart Request packet type */
#define RSTRC_TYP  13		/* Restart Confirm packet type */
#define DIAG_TYP   14		/* Diagnostic Packet type    */
#define RR_TYP	   15		/* Receive Ready packet     */
#define RNR_TYP    16		/* Receive Not Ready packet */
#define REJ_TYP    17		/* Reject Packet	    */
#define ERR_TYP    18		/* Unknown packet type or out-of-protocol */

/* -------------------------------------------------------------------------
    timeouts that are running in ccb->time_run
   ------------------------------------------------------------------------- */
#define TIM_CHRI 0x400		/* timeout for char mode input end frame*/
#define TIM_XMIT 0x200		/* timeout for transmit, chr/pkt mode	*/
#define TIM_RNR  0x100		/* timeout for RNR sent, try clear it	*/
#define TIM_SEND  0x80		/* timeout for send (and reply returned)*/
#define TIM_REP   0x40		/* timeout for reply (recv active)	*/
#define TIM_RR	  0x20		/* timeout for RR (recv idle)		*/
#define TIM_RSTR  0x10		/* timeout for restart			*/
#define TIM_CALL  0x08		/* timeout for call			*/
#define TIM_CLR   0x04		/* timeout for clear			*/
#define TIM_RSET  0x02		/* timeout for reset			*/
#define TIM_INTR  0x01		/* timeout for interrupt		*/

/* -------------------------------------------------------------------------
    circuit state field in ccb->circ_state
   ------------------------------------------------------------------------- */
#define CIR_OPEN 0x80		/* circuit has done OPEN		*/
#define CIR_TERM 0x40		/* circuit is doing terminate restart	*/
#define CIR_RSTR 0x20		/* circuit has done restart		*/
#define CIR_CALL 0x10		/* circuit has done call		*/
#define CIR_CLR  0x08		/* circuit has done clear		*/
#define CIR_RSET 0x04		/* circuit has done reset		*/
#define CIR_ACTV 0x02		/* circuit active; packets sent/recved	*/

/* -------------------------------------------------------------------------
    send state field in ccb->send_state; if non-zero pack_write uses writ_que
   ------------------------------------------------------------------------- */
#define SEN_HOLD 0x80		/* RNR recv - waiting for RR recv	     */
#define SEN_TIME 0x40		/* Retransmiss after timeout.	waiting ack  */
#define SEN_INTR 0x10		/* Interrupt request was sent.	waiting Conf */

#define SEN_RTRN 0x20		/* Retransmiss after REJ recv.	waiting ack  */
#define RTR_HOLD FALSE		/* BobC, 9/8/86 - no stop for REJ until ???  */

/* -------------------------------------------------------------------------
    recv state field in ccb->recv_state
   ------------------------------------------------------------------------- */
#define REP_RNR  0x80		/* RNR sent. repeat if more input received */
#define REP_REJ  0x40		/* REJ sent - waiting for confirm (seq pkt)  */
#define REP_RR	 0x20		/* RR sent - waiting.  confirm ends RNR/REJ  */
/* sequenced input will confirm RR/REJ was received, but RNR will timeout    */

/* -------------------------------------------------------------------------
    Not Ready state filed in ccb->rnr_sent, rnr_recv
   ------------------------------------------------------------------------- */
#define RNR_RCV  0x80		/* rnr packet received, send reasons follow  */
#define RNR_XPC  0x40		/* xpc has low que/buf count		     */
#define RNR_WIN  0x20		/* window is full, cannot accept more	     */
/*	RNR_SEQ  0x0F		   sequence number field */

/* -------------------------------------------------------------------------
    restart cause field in restart packets
   ------------------------------------------------------------------------- */
#define RST_DTEORG   0		/* DTE originated */

#define RST_LOCPE    1		/* local procedure error */

#define RST_NETCNG   3		/* network congestion */
#define RST_NETOPR   7		/* network operational */
#define RST_REGCANC  127	/* registration/cancellation confirmed */

/* -------------------------------------------------------------------------
    Clear cause field in clear packets
   ------------------------------------------------------------------------- */
#define CLR_DTEORG   0		/* DTE originated */

#define CLR_NUMBUSY  1		/* number busy */
#define CLR_DOWN     9		/* out of order */
#define CLR_REMPE    17 	/* remote procedure error */
#define CLR_RCHGNSUB 25 	/* reverse charging accept not subscribed */
#define CLR_INCDEST  33 	/* incomplete destination */
#define CLR_FSELNSUB 41 	/* fast select accept not subscribed */
#define CLR_SHIPABS  57 	/* ship absent */

#define CLR_INVFACIL 3		/* invalid facility request */
#define CLR_ACCBAR   11 	/* access barred */
#define CLR_LOCPE    19 	/* local procedure error */

#define CLR_NETCNG   5		/* network congestion */
#define CLR_NOTOBTN  13 	/* not obtainable */
#define CLR_RPOADOWN 21 	/* RPOA out of order */

/* -------------------------------------------------------------------------
    Reset cause field in reset packets
   ------------------------------------------------------------------------- */
#define RES_DTEORG   0		/* DTE originated */

#define RES_DOWN     1		/* out of order */
#define RES_REMPE    3		/* remote procedure error */
#define RES_LOCPE    5		/* local procedure error */
#define RES_NETCNG   7		/* network congestion */
#define RES_REMDTEOP 9		/* remote DTE operational */
#define RES_NETOPR   15 	/* network operational */
#define RES_INCDEST  17 	/* incomplete destination */
#define RES_NETDOWN  29 	/* network out of order */

/* -------------------------------------------------------------------------
    Registration cause field in registration packets
   ------------------------------------------------------------------------- */
#define REG_REGCANC  127	/* registrationcancellation confirmed */

#define REG_LOCPE    5		/* local procedure error */
#define REG_INVFACIL 3		/* invalid facility request */

#define REG_NETCNG   7		/* network congestion */

/* -------------------------------------------------------------------------
    Severity code of messages to be logged - pesented as a diagnostic packet.
    The channel goes in the lower nibble.
   ------------------------------------------------------------------------- */
#define MS_ABORT   0x10 	/* abort condition			     */
#define MS_ERROR   0x20 	/* problem condition			     */
#define MS_WARN    0x30 	/* warning condition			     */
#define MS_INFO    0x40 	/* information condition		     */
/*   must not use: 0xF0, as this is in a real diagnostic packet 	     */

/* -------------------------------------------------------------------------
    Origin of messages to be logged (which subsystem)
   ------------------------------------------------------------------------- */
#define MO_SYSTEM  0		/* XPC common service routines */
#define MO_CHAR    1		/* XPC character mode */
#define MO_LINK    2		/* XPC link level routines */
#define MO_PACK    3		/* XPC packet level routines */

/* -------------------------------------------------------------------------
    Number of messages to be logged (within an origin)
   ------------------------------------------------------------------------- */
#define MS_QUELOW 1		/* XPC common - available queue entries low  */
#define MS_BUFLOW 2		/* XPC common - available buffer entries low */

#define MC_CLOSED 1		/* XPC character mode, input channel closed */

#define ML_GQFRAMI 1		/* XPC link - getque failed in frame_in      */
#define ML_GBPUTB  2		/* XPC link - getbuf failed in put_byte      */
#define ML_STFRAMW 3		/* XPC link - settim failed in frame_write   */

#define MP_DIAGDCE  1		/* XPC packet - diag to DCE not allowed      */
#define MP_DIAGSENT 2		/* XPC packet - diag to DTE sent	     */
#define MP_RSTRSENT 3		/* XPC packet - restart sent		     */
#define MP_CLRSENT  4		/* XPC packet - clear sent		     */
#define MP_RSETSENT 5		/* XPC packet - reset sent		     */

/* -------------------------------------------------------------------------
   PROTOCOL CONTROL VALUES:  These values are assigned to the 'ccb' fields
   that control the protocol values used.  Examples are window size, number
   of bytes allowed in packets.  Use ccb fields because value negotiatable,
   these fields are for use by initializing routines in 'x_lcbdcl.c'.
   ------------------------------------------------------------------------- */
#define  MAX_WNDWSZ  8		 /* maximum window size (default).	     */
#define  DAT_WNDWSZ  4		 /* maximum data packets withing max. window.*/
#define  MAX_PKTSZ 128		 /* maximum number of data bytes in packet.  */
#define  DEAD_MODE    0 	 /* Driver_mode initialized to this value.   */
#define  CHAR_MODE    4 	 /* Causes I/O to be enabled, channel 0.     */
#define  PACK_FLAG    8 	 /* AND shows packet mode, whether DTE/DCE.  */
#define  PDTE_MODE    8 	 /* Allow packet channel OPENs, DTE config.  */
#define  PDCE_MODE   12 	 /* Allow packet channel OPENs, DCE config.  */


/* -------------------------------------------------------------------------
   These values identify the resources a channel will support, and are used
   when a channel is OPENed.  Take value from calltype and add to channel type.
   To reopen channels after error, AND in RECOVR_FLG to re-connect to old chan.
   ------------------------------------------------------------------------- */
#define  OUT_CALLS    1 	 /* Outgoing Calls-enabled calltype.	     */
#define  IN_CALLS     2 	 /* Incoming Calls-enabled calltype.	     */
#define  TWO_CALLS    3 	 /* Incoming/Outgoing Calls-enabled calltype.*/

#define  VC_CHANNEL   4 	 /* Virtual Ciruit - ADD calltype option.    */
#define  PVC_CHANNEL  8 	 /* Permanent Virtual Circuit, no calltype.  */
#define  DG_CHANNEL  12 	 /* Datagram channel, no calltype.	     */

#define  RECOVR_FLG  16 	 /* AND with final choice to show recovering.*/

#define  CHAR_CHAN    0 	 /* channel used for character channel I/O.  */
