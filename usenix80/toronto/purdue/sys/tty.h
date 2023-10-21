#define XSTTY /* use extended flag bits in speeds word */
#define EP	/* run with ep handler (requires XSTTY to be defined) */
#define PUMP.	/* PUMP support code in tty driver. (requires XSTTY) */
#define APL     /* Support code for APL overstrikes (needs XSTTY) */
/*
 * A clist structure is the head
 * of a linked list queue of characters.
 * The characters are stored in 4-word
 * blocks containing a link and 6 characters.
 * The routines getc and putc (m45.s or m40.s)
 * manipulate these structures.
 */
struct clist
{
	int	c_cc;		/* character count */
	int	c_cf;		/* pointer to first block */
	int	c_cl;		/* pointer to last block */
};

/*
 * A tty structure is needed for
 * each UNIX character device that
 * is used for normal terminal IO.
 * The routines in tty.c handle the
 * common code associated with
 * these structures.
 * The definition and device dependent
 * code is in each driver. (kl.c dc.c dh.c)
 */
struct tty
{
	struct	clist t_rawq;	/* input chars right off device */
	struct	clist t_canq;	/* input chars after erase and kill */
	struct	clist t_outq;	/* output list to device */
	int	t_flags;	/* mode, settable by stty call */
	int	*t_addr;	/* device address (register or startup fcn) */
	int	t_speeds;	/* output+input line speed */
	int	t_dev;		/* device name */
	char	t_delct;	/* number of delimiters in raw q */
	char	t_col;		/* printing column of device */
	char	t_erase;	/* erase character */
	char	t_kill;		/* kill character */
# ifndef XSTTY
	char	t_state;	/* internal state */
# endif
	char	t_char;		/* character temporary */
#ifdef XSTTY
	char	t_xxoff;	/* buf offset (0-77) for PAR mapping */
	int	t_xxpid;	/* pid of job with xx device open */
	int	t_xxpar;	/* if nz, contains PAR to map to user */
	char	*t_bp;		/* pointer to large I/O buffer */
	int	t_state;	/* internal state */
# ifdef APL
	char    t_achr;         /* prev output char for APL overstrike */
# endif
#endif
};

char partab[];			/* ASCII table: parity, character class */

#define	TTIPRI	10
#define	TTOPRI	20
#define PUMPRI	-2		/* must be negative */
#define DPRI	-1		/* must be negative */

/* default special characters */
#define	CERASE	010		/* Control-H */
#define	CEOT	004		/* Control-D */
#define	CKILL	030		/* CAN, Control-X */
#define	CQUIT	034		/* FS, cntl shift L */
#define	CINTR	0177		/* DEL */
#define CHOLD	0033		/* ESC -- suspend output */
#define CACK	0006		/* ACK - currently same as suspend output */
#define CBS     010             /* Hardware backspace - control-H */
# ifdef APL
#define CAPL_BS '^'             /* APL backspace (not erase) for overstrike */
# endif

/* limits */
#define	TTHIWAT	200
#define	TTLOWAT	120
#define	TTYHOG	256
# ifdef XSTTY
#define DTHRESH	70
# endif


/*
 * Delay algorithms
 */
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB1	002000
#define	TAB2	004000
#define	TAB3	006000
#define	FF1	040000

/*
 * speeds
 */
#define B75	2
#define	B110	3
#define B134	4
#define	B150	5
#define B200	6
#define	B300	7
#define B600	8
#define B1200	9
#define B1800	10
#define B2400	11
#define B4800	12
#define	B9600	13
#define B19200	1	/* 50 baud on our DH11 is 19.2kb */
#define B38400	14	/* exta is 38.4kb on our DH11 */
#define B0001	14
#define B0002	15

/* modes */
#define	HUPCL	01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	NLDELAY	001400
#define	TBDELAY	006000
#define	CRDELAY	030000
#define	VTDELAY	040000
#define BSDELAY 0100000

#ifdef XSTTY
/* extra mode bits in high order 12 bits of speed word.
 * Note only 4 bits (3-0) are used to set speed, ispeed and 
 * ospeed are both set to speed.
 * -ghg 7/4/77.
 */

#define SPEED	017		/* index into baud rate table */
#define TOEOB	0100		/* end of block timeout processing for PUMP */
#define SPY	0200		/* spy on this device (stty write only) */
#define PUMP	0400		/* special processing for PUMP */
#define SUSNL	01000		/* suspend output on newline output */
#define BLITZ	02000		/* blitz - abort all i/o until close */
#define APLMOD  010000          /* special processing for APL (overstrikes) */
#define HRAW	020000		/* half raw - wakeup on control char */
#endif

/* Hardware bits */
#define	DONE	0200
#define	IENABLE	0100

/* Internal state bits */
#define	TIMEOUT	01		/* Delay timeout in progress */
#define	WOPEN	02		/* Waiting for open to complete */
#define	ISOPEN	04		/* Device is open */
#define	SSTART	010		/* Has special start routine at addr */
#define	CARR_ON	020		/* Software copy of carrier-present */
#define	BUSY	040		/* Output in progress */
#define	ASLEEP	0100		/* Wakeup when output done */
#define HOLD	0200		/* output suspended */
# ifdef XSTTY
#define DRSVD	0400		/* device reserved by another handler */
#define TIMACT	01000		/* PUMP EOB timer active */
#ifdef EP
#define EPRNTR	02000		/* set if electrostic printer */
#endif
#define	DWRIT	04000		/* interlock for large buffers (ttwrt) */
#define	DWANT	010000		/* wakeup when DWRIT goes off */
# ifdef APL
#define APLSTRK 020000          /* last char was APL_BS (overstrike) */
# endif
# endif
