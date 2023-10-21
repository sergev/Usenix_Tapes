/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/

/* -------------------------------------------------------------------------
	       X.PC Bufferlet/Queue Structures Definitions
			      filename = p_bufque.s

   This module contains the definition of structures used to access and control
   the queue and bufferlet data structures used by the X.PC programs.
   -------------------------------------------------------------------------
     Date  change    By    Reason
   06/06/84  01     curt   Initial conversion to PAD use.
   10/03/84  02     curt   Changed 'ent_cnt' and 'size_chk' fields to integer
			   in the 'buf_head' and 'que_head' typedefs.
   10/02/86  03     Ed M   Combined old "p_bufque.s," "p_define.d," and
			   "p_config.d" into one file "driver.h" and added 
			   external references that were in some ".e" files.
   ------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------
			some TYPEDEF statements

   'fncptr' defines a type that is a pointer to a function that returns an
      integer result.  Variables of this type may be assigned a value using
      the varname=routinename construct, and the routine may be called by
      using the (*varname) construct.
   'byte' defines a type that is 8 bits wide.
   ------------------------------------------------------------------------- */
typedef  int   (*fncptr)();
typedef  char  byte;


/* -------------------------------------------------------------------------
			BUFFERLET pool definitions

   The 'bufq_ent' structure defines the layout of a member of the bufferlet
   pool.  Each bufferlet entry has a pointer to the next buffer in the list
   a count of bytes currently in the buffer, and byte-oriented storage.
   ------------------------------------------------------------------------- */
#define  TOTBFLTSIZ   34 	       /* TOTAL bytes for entry, MAKE EVEN!! */
#define  BFLT_DATA_SIZE TOTBFLTSIZ-4   /* pointer and counter take 4 bytes.  */

typedef struct	 bufq_ent {
   struct   bufq_ent *bnext;	       /* pointer to next buffer (or NULL).  */
   int	    num_bytes;		       /* # bytes used in this bufferlet.    */
   byte     data[BFLT_DATA_SIZE];      /* data storage area (zero based).    */
}  BUF_ENTR, *BUF_PTR;	  /* typedef bufq_ent */

/* -------------------------------------------------------------------------
   These show the indices within 'bufferlet->data' for packet header fields.
   ------------------------------------------------------------------------- */
#define B_STX	0
#define B_SIZE	1
#define B_GFLCI 2
#define B_PRPS	3
#define B_PKTID 4
#define B_CRC	5
#define B_BYTE0 6
#define B_BYTE1 7



/* -------------------------------------------------------------------------
   Bufferlet List Header Blocks contain a pointer to the first and last entry
   of the bufferlets.  If either is NULL (both should be if one is), there
   are no bufferlets available.  If the two are equal, there is one bufferlet
   in the queue.  The size check field may be used to find bugs.
   ------------------------------------------------------------------------- */

typedef struct	 buf_head {
   BUF_PTR  first;		    /* pointer to a bufferlet, first in list */
   BUF_PTR  last;		    /* pointer to a bufferlet, last in list. */
   int	    ent_cnt;		    /* current number of entries in queue.   */
   int	    siz_chk;		    /* must verify size when enque/deque.    */
}  BHEAD, *BHEADPTR;	 /* typedef buf_head */

extern BHEAD avail_buff;


/* -------------------------------------------------------------------------
		 Command Queue Entry Definition

   A 'cmdq_ent' definition is used to define the data structure of the
   'command list queue' elements.  Each has a pointer to any associated buffer
   areas, 8 bits that reflect the status of the bufferlet list associated with
   the queue entry, and several protocol-specific fields.
   ------------------------------------------------------------------------- */
struct	 cmdq_ent {
   BUF_PTR  bfirst;		       /* first buffer in bufferlet chain.   */
   byte     b_state;		       /* state of the buffer (0 means none) */
   byte     p_gfilci;		       /* General Format Indicator for pkt.  */
   byte     p_seqenc;		       /* Packet recv and send sequence nos. */
   byte     p_id;		       /* packet type identifier.	     */
}  ;  /* cmdq_ent */

/* -------------------------------------------------------------------------
   These defines are masks to get bit fields from within 'b_state'.
   ------------------------------------------------------------------------- */
#define B_RECVD  0x01		       /* recv. buffer=1, send buffer=0.     */
#define B_RETRAN 0x02		       /* retransmission after send timeout. */
#define B_QUED	 0x04		       /* bufferlist moved to proper queue.  */
#define B_STRTD  0x08		       /* I/O operation has been started.    */
#define B_CMPLTD 0x10		       /* I/O operation has been completed.  */
#define B_ACKED  0x20		       /* recvd=send ack, send=wait ack.     */
#define B_RETRND 0x40		       /* ready to return to available list. */
#define B_APPLIC 0x80		       /* Application originated this packet.*/

/* -------------------------------------------------------------------------
   These defines are masks to get to fields within protocol header fields, and
   may be applied to:
    the ccb->in_buff[] array, or,
    the cmd.p_gfilci or cmd.p_sequence fields, or,
    the first bufferlet's data[B_GFLCI] or data[B_PRPS] fields.
   ------------------------------------------------------------------------- */
#define C_BIT 0x80		       /* Control Bit			     */
#define D_BIT 0x40		       /* Delivery Confirmation Bit	     */
#define Q_BIT 0x20		       /* Qualifier Bit 		     */
#define M_BIT 0x10		       /* More data (in next packet) Bit     */
#define P_LCI 0x0F		       /* Logical Channel Identifier for pkt.*/
#define P_RECV 0xF0		       /* Packet Receive sequence field.     */
#define P_SEND 0x0F		       /* Packet Send sequence field.	     */

/* -------------------------------------------------------------------------
		   TIMER Queue Entry Definition

   The 'timq_ent' structure defines the layout for those fields that are
   related to the timer processing.  The structure fits within a 'que_ent'
   structure as referenced by timer processing routines.  The value of the
   'time_units' variable differs by position in the queue: the first entry
   will contain the # of time units remaining before the timeout occurs; the
   remaining entries show # of time units after the previous timer expires
   that the current entry is due to expire (delta time past previous).
   ------------------------------------------------------------------------- */
struct	 timq_ent {
   int	    units;	 /* # time units or delta # time units from prev.    */
   fncptr   routine;	 /* function to execute when t_units = zero.	     */
   int	    param;	 /* value passed as parameter to t_routine.	     */
}  ;  /* timq_ent */


/* -------------------------------------------------------------------------
			      QUEUE control Structures

   The 'que_ent' definition is used throughout.  The data beyond the que_ent
   type pointer depends upon the tag used to reference the structures contained
   within the union declaration.
   ------------------------------------------------------------------------- */
typedef struct	 que_ent {
   struct   que_ent  *qnext;		  /* pointer to next queue entry.    */
   union {
      struct   cmdq_ent cmd;		  /* command queue entries.	     */
      struct   timq_ent time;		  /* timer queue entries.	     */
   }  u;
      /*
	 For variable of type 'que_ent' named 'que_member', can reach field in
	 `cmdq_ent' type area using 'que_member.u.cmd.field'.  If have a
	 pointer variable of type '*que_ent' named 'que_member', can reach
	 field in 'cmdq_ent' type area using 'que_member->u.cmd.field'.
      */
}  QUE_ENTR, *QUE_PTR;	   /* typedef que_ent */


/* -------------------------------------------------------------------------
		    QUEUE HEADER block structure
   Queue Header Blocks contain a pointer to the first queue entry and the
   last queue entry.  If either is NULL (both should be if one is), there
   are no entry in the queue. If the two are equal, there is one entry
   in the queue.  The size check field may be used to find bugs, someday...
   ------------------------------------------------------------------------- */
typedef struct	 que_head {
   QUE_PTR  first;		    /* pointer to a que_ent, first in list.  */
   QUE_PTR  last;		    /* pointer to a que_ent, last in list.   */
   int	    ent_cnt;		    /* current number of entries in queue.   */
   int	    siz_chk;		    /* must verify size when enque/deque.    */
}  QHEAD, *QHEADPTR;  /* que_cntrl */

extern QHEAD avail_que;

/* -----------------------------------------------------------------------
   Defines for use with get_que()
   ----------------------------------------------------------------------- */
#define	YES_BUFF 1
#define NO_BUFF  0


/* ------------------------------------------------------------------------
    DEFINES and MACROs for use in the X.PC product 'C' code.
   ------------------------------------------------------------------------ */

#define  FALSE	  0
#define  TRUE	  1
#define  NULL	  0

#define  nothing
#define  void	  int		 /* means function does not return value.    */

#ifndef  max
#define  max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef  min
#define  min(a,b) ((a)<=(b)?(a):(b))
#endif

#define  DEBUG	  0		 /* some packet layer files generate extra   */
				 /* code when this flag set to value of 1!!! */
/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/

/* -------------------------------------------------------------------------
   These fields are used to define information about the Device or PAD.
   ------------------------------------------------------------------------- */
#define  DEV_VERSION	3     /* MCI coded: X.PC, 1986, 3.xx banner */
#define  DEV_REVISION	2     /* for v.r = 3.02, 09/08/86, RNC */
#define  PAD_TYPE	1

/* -------------------------------------------------------------------------
   These allow the range of logical channels to be broken down into 'classes'.
   The range LO_INC to HI_INC shows which logical channels are available for
   incoming calls, the range LO_TWOC to HI_TWOC shows which are available for
   calls that are outgoing or incoming, the range LO_OUTC to HI_OUTC shows which
   logical channels are available for outgoing calls, and the range HI_PVC to
   LO_PVC show the range for Permanent Virtual Circuits.  When a 'class' is not
   allowed, specify 0-0 for the range.	The 'count' fields show the total # of
   channels that are available for a given class of channnel.
   ------------------------------------------------------------------------- */
#define  LO_INC 	0
#define  HI_INC 	0
#define  CNT_INC_CHAN	0
#define  LO_TWOC	0
#define  HI_TWOC	0
#define  CNT_TWOC_CHAN	0
#define  LO_OUTC	0
#define  HI_OUTC	0
#define  CNT_OUTC_CHAN	0
#define  LO_PVC 	1
#define  HI_PVC        15
#define  CNT_PVC_CHAN  15

/* -------------------------------------------------------------------------
   The count values determine the number of bufferlets in the bufferlet pool,
   and the number of queue entries in the queue pool (they are used by the
   BQPOOL module to allocate arrays).  The LO/HI values are threshold points:
   when bufferlets/queues available becomes below LO threshold, LOW bits set to
   indicate; when available goes back above HI threshold, LOW bits removed.
   ------------------------------------------------------------------------- */
#define  BUFPLCNT 128		/* bufferlets */
#define  QUEPLCNT 64		/* queues */
#define  QUE_LOW 1		 /* flag lcb.xpc_nrdy: # que entries too low,*/
#define  BUF_LOW 2		 /* flag lcb.xpc_nrdy: # buff entries too low*/
#define  QLO_THRES   8		 /* low threshold to flag running out of ques*/
#define  QHI_THRES  12		 /* high threshold to flag enough ques again.*/
#define  BLO_THRES   8		 /* low threshold to flag running out of bufs*/
#define  BHI_THRES  12		 /* high threshold to flag enough bufs again.*/

/* -------------------------------------------------------------------------
   These values define how a Packet channel terminal characteristics are
   configured when set to the default state.  'init_termchr' uses values.
   ------------------------------------------------------------------------- */
#define  DEF_ok_echo	 TRUE	       /* is OK to enable echo if host wants.*/
#define  DEF_enb_echo	 FALSE	       /* echo not enabled until host wants. */
#define  DEF_def_echo	 FALSE	       /* once host says, start in deferred. */
#define  DEF_ctli_ech	 FALSE	       /* PAD will not echo TABS locally, EDM*/
#define  DEF_ctlh_ech	 FALSE	       /* PAD will not echo BS locally, EDM. */
#define  DEF_escp_ech	 FALSE	       /* PAD will not echo ESC locally, EDM.*/
#define  DEF_lfoncr_ech  TRUE	       /* TYMNET standard echo free LF on CR.*/
#define  DEF_cronlf_ech  FALSE	       /* don't worry about this function.   */
#define  DEF_chr_forwrd  0x0D	       /* forward on carriage return: echoio.*/
/* if chcb->tc.forwrd_set is FALSE, these are used going in/out of local echo*/
#define  DEF_tim_forwrd  0	       /* forward timer for remote echo  */
#define  DEF_ech_forwrd  12	       /* forward timer for local echo	*/
#define  DEF_bspace	 '\0'          /* backspace char for mci editing.    */
#define  DEF_lindel	 '\0'          /* line delete char for mci editing.  */
#define  DEF_linagn	 '\0'          /* line redisplay char for mci editing*/

#define  DEF_parity_enb  TRUE
#define  DEF_pktlim	 128	       /* limit on number of bytes in one pkt*/

#define  DEF_echo_mci	 FALSE	       /* MCI permanent echo off at start.   */
#define  DEF_edit_mci	 FALSE	       /* MCI line edit off at start.	     */
#define  MAX_EDIT	 1	       /* limits number of channels editing. */
#define  EDBUF_MAX	 128	       /* limits MCIPAD edit buffer length.  */
#define  BUF_MAX	 256	       /* limits driver's i/o buffer length. */


/* -------------------------------------------------------------------------
   These are to indicate the field that the 'chanst_upd' routine should update.
   ------------------------------------------------------------------------- */
#define  UPD_RECV    1
#define  UPD_XMIT    2
#define  UPD_RESET   3		/* reset received - one channel */
#define  UPD_RSTRT   4		/* restart received - all channels reset    */
#define  UPD_CRSTRT  5		/* restart end packet mode, go to char mode */
#define  UPD_PKTLOST 6		/* packet mode timeout, remote XPC lost     */
#define  UPD_MDMLOST 7		/* modem failure, DSR or CD lost	    */

/* -------------------------------------------------------------------------
   Constants for first parameter to link_write routines.
   ------------------------------------------------------------------------- */
#define LWRIT_NUL 0		/* will cause xmit to start up if not active.*/
#define LWRIT_OUT 1		/* unsequenc replys (RR,RNR,REJ), no retrans */
#define LWRIT_REP 2		/* sequenced packets, with retransmission */


/* -------------------------------------------------------------------------
   EVNT_PSEC defines how many times a second the timer queue will be serviced.
   The minimum timeout possible is "1/EVNT_PSEC th" of a second.
   ------------------------------------------------------------------------- */
#define  EVNT_PSEC   6
#define  HALF_SEC    EVNT_PSEC/2
#define  ONE_SEC     EVNT_PSEC
#define  TWO_SEC     EVNT_PSEC*2
#define  THREE_SEC   EVNT_PSEC*3

