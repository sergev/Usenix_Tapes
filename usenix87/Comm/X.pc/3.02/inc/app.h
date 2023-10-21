/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.	     *
*****************************************************************************/
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
} DSS_STR, *DSS_PTR; /* typedef dss_type */


typedef struct pcb_type {
   int	 baud;		/* baud rate chip is currently set to.		     */
   int	 data_bits;	/* Number of bits of data in each Async. frame.      */
   int	 parity;	/* No parity, odd, even, mark, space parity flag.    */
   int	 stop_bits;	/* Number of stop bits in each Async. frame.	     */
   int	 xon_xoff;	/* TRUE: character mode is xon/xoff flow controlled. */
   int	 DTE_port;	/* TRUE: run packet layer as DTE, otherwise as DCE.  */
} PCB_STR, *PCB_PTR;  /* typedef pcb_type */
#define PORT_PRMLEN 12	/* 12 bytes to hold 6 fields of 'port parameter blk'.*/


typedef  struct   ppb_type {
   int	 *param_seg;	   /* segment in which optional parameters reside.   */
   int	 *func_code;	   /* Function code indicating service to perform.   */
   int	 *device;	   /* The device number for this service call.	     */
   int	 *channel;	   /* The channel number for this service call.      */
   int	 *status;	   /* Status of call is returned in this field.      */
   int	 *param1;	   /* Pointer to Parameter 1 (used to reach value).  */
   int	 *param2;	   /* Pointer to Parameter 2 (used to reach value).  */
   int	 *param3;	   /* Pointer to Parameter 3 (used to reach value).  */
   int	 *param4;	   /* Pointer to Parameter 4 (used to reach value).  */
} PPB_STR, *PPB_PTR;	/* typedef ppb_type  */


#define  LINE_LEN  132
#define  INARA_LN  (3*LINE_LEN)/2	 /* add some space for little stuff */
#define  MAX_PKTSZ 128

typedef struct	 channel {
   int	 is_open;
   int	 channel;
   int	 func_code;
   int	 status;
   int	 param1;
   int	 param2;
   int	 param3;
   int	 param4;
   int	 timer_upd;
   int	 call_state;
   int	 xmit_status;
   int	 recv_status;
   int	 firstrow;
   int	 lastrow;
   int	 firstcol;
   int	 lastcol;

   char  in_arr[INARA_LN];    /* Need more room in case WRITE must wait */
			      /* See logic in "data_read(p_driver)" ifs */
   char  out_arr[LINE_LEN];

} CHNL_STR;	/* a.k.a "ca" */

extern	 CHNL_STR  ca[];
extern	 PPB_STR   params;
extern	 DSS_STR   device;
extern	 PCB_STR   port;

/* -------------------------------------------------------------------------
   This  defines constants used throughout the 'p_appl.exe' program.
   ------------------------------------------------------------------------- */

#define  REQST_DONE(a)	(ca[a].status==FUNC_COMPLETE)

#define  NULL_CHR  '\0'
#define  BELL_CHR  '\007'
#define  BSPAC_CHR '\010'
#define  TAB_CHR   '\011'
#define  LF_CHR    '\n'
#define  CR_CHR    '\r'
#define  EOT_CHR   '\032'
#define  ESC_CHR   '\033'
#define  SPC_CHR   '\040'
#define  Q_CHR	   '\121'
#define  LONG	  20000
#define  MEDIUM   10000
#define  SHORT	  500

#define  SYS_CH      0

#define  SHOW_IT   TRUE
#define  NO_SHOW   FALSE

#define  WAIT_IT   TRUE
#define  NO_WAIT   FALSE
#define  NO_CHAR   0x100

#define  FALSE	  0
#define  TRUE	  1
#define  NULL	  0

#define  nothing
#define  void	  int		 /* means function does not return value.    */

#define  max(a,b) ((a)>(b)?(a):(b))
#define  min(a,b) ((a)<=(b)?(a):(b))

#define  DEBUG	  0		 /* some packet layer files generate extra   */
				 /* code when this flag set to value of 1!!! */
