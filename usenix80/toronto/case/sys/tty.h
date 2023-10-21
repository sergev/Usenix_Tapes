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
	char	t_delct;	/* number of delimiters in raw q */
	char	t_col;		/* printing column of device */
	char	t_erase;	/* erase character */
	char	t_kill;		/* kill character */
	int	t_state;	/* internal state, not visible externally */
	char	t_char;		/* character temporary */
	int	t_speeds;	/* output+input line speed */
	int	t_dev;		/* device name */
};

char partab[];			/* ASCII table: parity, character class */

#define	TTIPRI	10
#define	TTOPRI	20

/*
 *	Special characters changed for Case.
 *	Output flow control added.
 *
 *	Bill Shannon   12/23/77
 */
#define	CERASE	'#'		/* default special characters */
#define	CEOT	004
#define	CKILL	'@'
#define CQUIT	002		/* ^B - quit signal */
#define CINTR	003		/* ^C - interrupt signal */
#define CFLUSH	017		/* ^O - flush output */
#define CFREEZE	023		/* ^S - freeze output */
#define CUFREEZ	021		/* ^Q - unfreeze output */
#define CKILLC	025		/* ^U - kill for crt's */
#define CERASEC	0177		/* RUBOUT - erase for crt's */
#define CBELL	007		/* BELL */
#define CBACK	010		/* ^H - backspace */

/* limits */
#define	TTHIWAT	50
#define	TTLOWAT	30
#define	TTYHOG	256

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
#define	MODEM	040000		/* modem control mode for printers */
#define	BRKCTRL	0100000		/* break on control characters */

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
#define FREEZE	0200		/* Freeze output */
#define FLUSH	0400		/* Flush output */
