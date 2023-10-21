/*	dhureg.h	1.1	02/09/85	*/

/* 
 * DHU-11 device register definitions.
 */
struct dhudevice {
	union {
		short	dhucsr;		/* control-status register */
		char	dhucsrl;	/* low byte for line select */
	} un;
	short	dhurbuf;		/* receive buffer register */
	short	dhulpr;			/* line parameter register */
	union {
		short dhufdata;		/* tx fifo data reg */
		struct {
			char dhufsize;	/* tx fifo size */
			char dhulstat;	/* line status register */
		}
	} un2;
	short	dhulctrl;		/* line control register */
	u_short	dhutbf1;		/* transmisster buffer 1 */
	short	dhutbf2;		/* tx buffer 2 */
	u_short	dhutbfct;		/* tx buffer count */
};

/* Bits in dhucsr */
#define	DHU_TI	0100000		/* transmit interrupt */
#define	DHU_TIE	0040000		/* transmit interrupt enable */
#define	DHU_DFL	0020000		/* diagnostic fail */
#define	DHU_TDE	0010000		/* tx dma error */
#define	DHU_TXL	0007400		/* transmit line */
#define	DHU_RI	0000200		/* receiver interrupt */
#define	DHU_RIE	0000100		/* receiver interrupt enable */
#define	DHU_MR	0000040		/* master reset */
#define	DHU_SK	0000020		/* skip self test */

/* Bits in dhulpr */
#define	BITS6	010
#define	BITS7	020
#define	BITS8	030
#define	TWOSB	0200
#define	PENABLE	040
#define	EPAR	0100

#define	DHU_IE	(DHU_TIE|DHU_RIE)

/* Bits in dhurbuf */
#define	DHU_PE		0010000		/* parity error */
#define	DHU_FE		0020000		/* framing error */
#define	DHU_DO		0040000		/* data overrun */

/* Bits in dhulctrl */

#define	TXABORT		001		/* abort dma in progress */
#define	RXENABLE	004		/* receiver enable */
#define	TXBREAK		010		/* transmit break */

/* Bits in dhutbf2 */

#define	TXDMAST		0200		/* start dma operation */
#define	TXENABLE	0100000		/* enable transmitter */

