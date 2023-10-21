/*
 * Definitions for Versatec Matrix printer/plotter driver
 *
 * see mpl.c & mp.c for driver proper.
 */
#define	NBUFS	5		/* maximum # of system buffers in use */
#define	MPOPRI	10		/* output priority (except during close) */
#define	MPLWAT	(NBUFS-1)*2	/* each buffer has at least one 'D' & 'l' command */
#define	MPHWAT	50
/*
 * Internal state.
 */
struct mp
  {	int m_flags;		/* see defines below */
	int *m_acs;		/* address of active csr */
	int m_cmd;		/* last command pulled from m_cmdq */
	int m_pri;		/* current sleep priority */
	int m_nbufs;		/* # of system buffers in use */
	int m_pagesize;		/* # of lines in page */
	int m_tofline;		/* # of lines from TOF to 1st print line */
	int m_ejline;		/* # of printing lines before auto-eject */
	int m_topeat;		/* # of input lines to eat at TOF */
	int m_vcc;		/* virtual column counter */
	int m_acc;		/* actual column counter */
	int m_alc;		/* actual line counter */
	int m_ilc;		/* input line counter */
	struct mpbuf *m_wbp;	/* top half's working buffer pointer */
	struct mpbuf *m_actf;	/* first buf in active buffer queue (pull) */
	struct mpbuf *m_actl;	/* last buf in active buffer queue (push) */
	struct clist m_tmpq;	/* temporary command queue */
	struct clist m_cmdq;	/* active command queue */
	struct clist m_canq;	/* canonical character processing queue */
  };
/*
 * mp.m_flags:
 */
#define	OPEN	01
#define	TEXT	02		/* plotter is in text mode (top half) */
#define	TMID	04		/* in middle of reading Text */
#define	RMID	010		/* in middle of reading Raster */
#define	IND	020		/* indent lines 1 tab (dev 1) */
#define PRINTER	040		/* one of the printer devices is open */
#define	ROLL	0100		/* roll paper is loaded */
#define	ERR	0200		/* error condition exists */
#define RETRY	0400		/* waiting for not READY to clear */
/*
 * Overlay template for buf struct
 */
struct mpbuf
  {	int m_flags;
	struct buf *m_forw;
	struct buf *m_back;
	struct mpbuf *m_next;	/* next in active queue */
	int m_used;		/* # of bytes used in buffer */
	dev_t m_dev;
	int m_wfree;		/* # of unallocated words in buffer */
	caddr_t m_addr;
#define m_extra m_un.m_ext
	union {
		int m_ext;
		daddr_t m_junk;
	}m_un;
	char m_xmem;
	char m_error;
	int m_resid;		/* # of raster bytes yet to be iomoved */
  };
/*
 * Format of raster and text data in buffer
 */
struct raster
  {	int r_len;		/* # of words in r_data[] */
	char r_data[];		/* raster dow row proper */
  };
/*
 * Versatec hardare defs
 */
#define MPRVECT	0174	/* raster interrupt vector */
#define	MPTVECT	0200	/* text interrupt vector */
#define	MPADDR	0177500

struct
  {	int mprbc;	/* raster byte count */
	int mpxmem;	/* dma memory extension */
	int mptbc;	/* text byte count */
	int mpma;	/* memory address */
	int mprcs;	/* raster control & status */
	int mprdr;	/* raster data register */
	int mptcs;	/* text control & status */
	int mptdr;	/* text data register */
  };
/*
 * csr bits:
 */
#define SPP	01	/* enable simultaneous print/plot */
#define RESET	02	/* remote reset */
#define CLRBUF	04	/* clear dot buffer */
#define EOT	010	/* remote end of transmission */
#define FF	020	/* remote form feed */
#define LINE	040	/* remote line terminate */
/* #define IENABLE	0100	/* interrupt enable for READY or ERROR */
#define READY	0200	/* matrix is ready for data byte */
#define DMABSY	020000	/* DMA data transfer in progress */
#define DTCIEN	040000	/* interrupt enable for DMA byte count == 0 */
#define ERROR	0100000	/* pwr off, no paper, off-line, non-exist mem */

/*
 * Misc.
 */
#define	FORM	014
#define ESC	033
#define	MAXCOL	132
#define	PAGESIZE 66		/* software conventional page length */
#define PAGELEN	68		/* actual # of lines in 8.5" page */
#define EJLINE	60		/* conventional # of printing lines (pr) */
#define EJAUTO	64		/* hardware # of printing lines */
#define TOPEAT	1		/* # of lines to discard at top in ESC mode */
#define TOFLINE	2		/* # conventional # of added lines at top */
#define TOFAUTO	2		/* # of lines from fanfold to auto eject */
#define	RSIZE	132		/* # of words in largest raster */
#define ROFFSET ((char *)&((struct raster *)0->r_data[0]))	/* offset of data
				   from start of raster structure */
