/*
 * dmfreg.h
 *
 * DMF32 register definitions.
 *
 *  "dmfc" refers to the dmf32 as a whole,
 *  "dmfa" refers to the async portion of the dmf32,
 *  "dmfs" refers to the sync portion of the dmf32,
 *  "dmfd" refers to the dr11 portion of the dmf32, and
 *  "dmfl" to the lp portion of the dmf32.
 *
 * (ro) - read only
 * (wo) - write only
 */

/*
 * dmf32 control register definitions
 */
struct dmfdevice {
	short	dmfc_config;	/* configuration control status reg. */
	short	dmfc_maint;	/* diagnostic control status register */
	short	dmfs_receive;	/* synchronous receive register */
	short	dmfs_xmit;	/* synchronous transmit register */
	short	dmfs_misc;	/* synchronous miscellaneous register */
	short	dmfs_indrct;	/* synchronous indirect register */
	short	dmfa_csr;	/* asynch. control status register */
	short	dmfa_lparm;	/* asynch. line parameter register */
	short	dmfa_rbuf;	/* asynch. receive buffer register */
	union {
		u_short	word;		/* word */
		u_char	bytes[2];	/* bytes */
	} dmfa_ir;		/* indirect register */
	short	dmfl_ctrl;	/* line printer control register */
	short	dmfl_indrct;	/* line printer indirect register */
	short	dmfd_ctrl;	/* dr11 control register */
	short	dmfd_outbuf;	/* dr11 output buffer register */
	short	dmfd_inbuf;	/* dr11 input buffer register */
	short	dmfd_indrct;	/* dr11 indirect register */
};

#define dmfa_sato dmfa_rbuf	/* receive silo alarm timeout (wo) */

/*
 * aliases for asynchronous indirect control registers
 */
#define IR_TBUF		000	/* transmit buffer register (wo)*/
#define	IR_RMS		000	/* recieve modem status register (ro)*/
#define	IR_TSC		000	/* transmit silo count register (ro)*/
#define IR_LCTMR	010	/* line control and transmit modem */
#define IR_TBA		020	/* trans. buf. addr register */
#define IR_TCC		030	/* transmit char. count */

#define dmfa_tbf	dmfa_ir.bytes[0]	/* transmit buffer (wo) */
#define dmfa_tbf2	dmfa_ir.word		/* tb for 2 chars. (wo) */

#define dmfa_rms	dmfa_ir.bytes[1]	/* receive modem status (ro) */
#define dmfa_tsc	dmfa_ir.bytes[0]	/* transmit silo count (ro) */

#define dmfa_lctmr	dmfa_ir.word	/* line control and transmit modem */

#define dmfa_tba	dmfa_ir.word	/* transmit buffer address */

#define dmfa_tcc	dmfa_ir.word	/* transmit character count */

/*
 * bits in dmfc_config register
 */
#define dmfc_flg	0100000	/* Asynch.flag */

/*
 * bits in dmfa_csr register
 */
#define DMFA_TRDY 	0100000	/* transmit ready */
#define DMFA_TIE  	0040000	/* transmit interrupt enable */
#define DMFA_NXM  	0030000	/* non-existant memory */
#define DMFA_LIN  	0003400	/* transmit line number */
#define DMFA_RRDY 	0000200	/* receiver data available */
#define DMFA_RIE  	0000100	/* receiver interrupt enable */
#define DMFA_RESET	0000040	/* master reset */
#define DMFA_IAD  	0000037	/* indirect address register */

#define DMFA_SIZ	32	/* size of output silo (per line) */
#define DMFA_IE		(DMFA_TIE|DMFA_RIE)

/*
 * bits in dmfa_lparm register
 */
#define DMFA_6BT	0010	 /* 6 bits per character */
#define DMFA_7BT	0020	 /* 7 bits per character */
#define DMFA_8BT	0030	 /* 8 bits per character */
#define DMFA_PEN	0040	 /* parity enable */
#define DMFA_EPR	0100	 /* even parity */
#define DMFA_SCD	0200	 /* stop code */
#define DMFA_XTE	0170000	 /* transmit rate */
#define DMFA_RRT	0007400	 /* receive rate */
#define DMFA_LSL	0000007	 /* line select */

/*
 * baud rates
 */
#define BR_50		000
#define BR_75		001
#define BR_110		002
#define BR_134_5	003
#define BR_150		004
#define BR_300		005
#define BR_600		006
#define BR_1200		007
#define BR_1800		010
#define BR_2000		011
#define BR_2400		012
#define BR_3600		013
#define BR_4800		014
#define BR_7200		015
#define BR_9600		016
#define BR_19200	017

/*
 * bits in dmfa_rbuf register
 */
#define DMFA_DSC	0004000	/* data set change */
#define DMFA_PE		0010000	/* parity error */
#define DMFA_FE		0020000	/* framing error */
#define DMFA_DO		0040000	/* data overrun */
#define DMFA_DV		0100000	/* data valid */
#define DMFA_RL		0003400	/* line */
#define DMFA_RD		0000377	/* data */
#define DMFA_AT		0000377	/* alarm timeout */

/*
 * bits in dmfa_rms register (i.e., dmfa_rmstsc >> 8)
 */
#define DMFA_USR	0004	/* user modem signal (pin 25) */
#define DMFA_SR		0010	/* secondary receive */
#define DMFA_CTS	0020	/* clear to send */
#define DMFA_CAR	0040	/* carrier detect */
#define DMFA_RNG	0100	/* ring */
#define DMFA_DSR	0200	/* data set ready */

/*
 * bits in dmfa_tms register
 */
#define DMFA_USW	0001	/* user modem signal (pin 18) */
#define DMFA_DTR	0002	/* data terminal ready */
#define DMFA_RAT	0004	/* data signal rate select */
#define DMFA_ST		0010	/* secondary transmit */
#define DMFA_RTS	0020	/* request to send */
#define DMFA_BRK	0040	/* pseudo break bit */
#define DMFA_PMT	0200	/* preempt output */

#define DMFA_ON		(DMFA_DTR|DMFA_RTS)
#define DMFA_OFF	0

/*
 * bits in dmfa_lcr register
 */
#define DMFA_MIE	0040	/* modem interrupt enable */
#define DMFA_FLS	0020	/* flush transmit silo */
#define DMFA_RBK	0010	/* real break bit */
#define DMFA_RE		0004	/* receive enable */
#define DMFA_AUT	0002	/* auto XON/XOFF */
#define DMFA_TE		0001	/* transmit enable */
#define DMFA_CF		0300	/* control function */

#define DMFA_LCE	(DMFA_MIE|DMFA_RE|DMFA_TE)

/*
 * bits in dmfa_tcc register
 */
#define DMFA_HA		0140000	/* high address bits */

/*
 * bits in dm lsr, copied from dh.c
 */
#define DM_USR		0001000	/* usr modem sig, not a real DM bit */
#define DM_DSR		0000400	/* data set ready, not a real DM bit */
#define DM_RNG		0000200	/* ring */
#define DM_CAR		0000100	/* carrier detect */
#define DM_CTS		0000040	/* clear to send */
#define DM_SR		0000020	/* secondary receive */
#define DM_ST		0000010	/* secondary transmit */
#define DM_RTS		0000004	/* request to send */
#define DM_DTR		0000002	/* data terminal ready */
#define DM_LE		0000001	/* line enable */

/*
 * bits in the line printer control registers
 */
#define DMFL_PEN	0000001	/* print enable */
#define DMFL_RST	0000002	/* master reset */
#define DMFL_FMT	0000004	/* format control */
#define DMFL_xx1	0000030	/* unused */
#define DMFL_MNT	0000040	/* mainenance mode on */
#define DMFL_IE		0000100	/* intr enable */
#define DMFL_PDN	0000200	/* print done bit */
#define DMFL_IDR	0003400	/* indirect reg */
#define DMFL_xx2	0004000	/* unused */
#define DMFL_CNV	0010000	/* connect verify */
#define DMFL_DVR	0020000	/* davfu ready */
#define DMFL_OFL	0040000	/* printer offline */
#define DMFL_DER	0100000	/* dma error bit */
#define DMFL_SIZ	512	/* max chars per dma */
#define DMFL_DCL	132	/* default # of cols/line */
#define DMFL_DLN	66	/* default # of lines/page */
