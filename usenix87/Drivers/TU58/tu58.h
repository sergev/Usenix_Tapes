/*
 * TU58 Radial Serial Protocol
 */

#define CMDLEN	10		/* # of chars after mcount & before chksum */
#define DATALEN 128		/* max # chars after  "		"	"  */

#define	u_char	char
#define	u_short	unsigned short

/* kc 7/85 */
/* pk_flag changed to int so that char->int of 0377 in tu58 doesn't yield -1 */

/*
 * Command packet
 */
struct packet {
	int	pk_flag;	/* packet type */ /* was u_char kc 7/85 */
	u_char	pk_mcount;	/* message count */
	int	pk_align;	/* This should align things so that there
					isn't a hole following pk_sw
				   Proper alignment is required */
	u_char	pk_op;		/* operation code */
	u_char	pk_mod;		/* modifier */
	u_char	pk_unit;	/* unit number */
	u_char	pk_sw;		/* switches */
	u_short	pk_seq;		/* sequence number, always zero */
	u_short	pk_count;	/* byte count for read or write */
	u_short	pk_block;	/* block number for read, write, or seek */
	u_short	pk_chksum;	/* checksum */
};

#define pk_succ	pk_mod		/* defs for end packet */
#define pk_stat	pk_block

/*
 * States
 */
#define	TUS_QUIT	0	/* exit tu58 */
#define	TUS_IDLE	1	/* initialized, no transfer in progress */
#define	TUS_TTH		2	/* tu58 to host */
#define	TUS_HTT		3	/* host to tu58 */
#define	TUS_WAIT	4	/* waiting for continue */

/*
 * Packet Flags
 */
#define TUF_NULL	0
#define	TUF_DATA	1		/* data packet */
#define	TUF_CMD		2		/* command packet */
#define	TUF_INITF	4		/* initialize */
#define TUF_BOOT	010		/* boot */
#define	TUF_CONT	020		/* continue */
#define TUF_XON		021		/* flow control */
#define	TUF_XOFF	023		/* flow control */

/*
 * Op Codes
 */
#define	TUOP_NOOP	0		/* no operation */
#define	TUOP_INIT	1		/* initialize */
#define	TUOP_READ	2		/* read block */
#define	TUOP_WRITE	3		/* write block */
#define	TUOP_SEEK	5		/* seek to block */
#define TUOP_DIAGNOSE	7		/* run micro-diagnostics */
#define	TUOP_END	0100		/* end packet */

/*
 * Mod Flags
 */
#define TUMD_WRV        1               /* write with read verify */
#define TUMD_128	0x80		/* special addressing mode */

/*
 * End packet success codes
 */
#define TUE_SUC	0
#define TUE_PAR 0376			/* failed at end of medium */
#define TUE_BADU 0370			/* bad unit */
#define TUE_BADF 0367			/* no cartridge (file) */
#define TUE_WPRO 0365			/* write protected */
#define TUE_DERR 0357			/* data check error */
#define TUE_SKRR 0340			/* seek error */
#define TUE_BADO 0320			/* bad op code */
#define TUE_BADB 0311			/* bad block number */

#define	NTU58	2			/* Number of drives to emulate */
