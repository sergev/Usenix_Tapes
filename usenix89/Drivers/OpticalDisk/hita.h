/*	hita.h	1.0	85/04/23	*/
	
/* Hitachi optical disk driver ioctrl commands */
/* Author:  Jungbo Yang  4/23/85  */

#define HREZERO 	1	/* home the optical head */
#define HMOUNT		2	/* mount/lock */
#define HDMOUNT		3	/* demount/unlock */
#define HLIBOPEN	4	/* eject */
#define HFLIP		5	/* change */
#define HNOOP		6	/* noop */
#define HSS		7	/* read second status */
#define HODDS		8	/* read OD status */
#define HCELLS		9	/* read Cell status */
#define HDEVID		10	/* read Device ID */
#define HSEEK		11	/* seek */
#define HFLUSH		12	/* flush */
#define HSRCHUNWTN	13	/* search unwritten blocks(62) */
#define HDELETE		14	/* delete */
#define HGETFMS		15	/* get FM Structure */
#define HPUTFMS		16	/* put FM Structure */
#define HFAST		17	/* fast I/O */
#define HSECTS		18	/* sector status */
#define HEXCL		19	/* exclusive use request */
#define HCLOSEWRITE	20	/* close write ********* */
#define HLGETFMS	21	/* getfms with locking fms */
#define HULFMS		22	/* unlock fms */
/* NOTE
 * this is because close() is called only at the last close call
 * no way to know when the write operation did close()
 *   close() is redefined to do this automatically
 */
int close();
static int (*hclose)() = close;
#define close(x)	{ \
	ioctl(x,HCLOSEWRITE,0); \
	(*hclose)(x); \
}

#define gethk_cell(x,y) hk_cell(1,x,y)
#define puthk_cell(x,y) hk_cell(0,x,y)

#define RD_OR 0x2		/* read commands */
#define RD_DCI 0x1		/* bit location. OR it */
#define RD_DUAL 0x40 		/* bit location */
#define RDL_NO 0x10 		/* logical read */
#define RDP_NO 0x20 		/* physical read */
#define RD_SU 0x40  		/* search unwritten */
#define RD_SD 0x80  		/* skip deleted blocks */
/*
 * status data length
 */
#define HSTAT_L_SS	24
#define HSTAT_L_DEVID	6
#define HSTAT_L_ODDS	24
#define HSTAT_L_CELL	256
#define HSTAT_L_SECT	254

/* error codes */
#define HERR_BADCELL	1
#define HERR_BADSIDE	2

/* from vax750 */
#define	GETF(fp, fd) { \
	if ((unsigned)(fd) >= NOFILE || ((fp) = u.u_ofile[fd]) == NULL) { \
		u.u_error = EBADF; \
		return; \
	} \
}

#define HBLOCKSIZE 512   /* hitachi block size */
#define HBUFSIZE 512   /* kernel buffer size */
#define HMAXOPEN 20    /* max number of open per device */
#define HITAPRI PRIBIO+1 /* priority for sleep() just slower than fs */

typedef struct
{
    short   h_state;              /* drive status */
				/*  OPEN, WRITE */
    short   h_opfd;		/* number of open */
    char    h_mta;                /* IEEE488 MTA */
    char    h_mla;                /* IEEE488 MLA */
    short  h_opop;      /* operation options      */
                      /*    Dual write          */
                      /*    Delete check inhibit */
    off_t   h_pbn;      /* current pbn            */
                      /*    location of the next read or write */
    short  h_woff;      /* write buffer pointer   */
    short  h_roff;      /* read buffer pointer    */
    char   h_rbuf[HBUFSIZE]; /* read buffer            */
    char   h_wbuf[HBUFSIZE]; /* write buffer           */
} HITA_t;

typedef struct {
    char    h_isr1;               /* interrupt status */
    char    h_isr2;               /* interrupt status */
    char    h_sr;                 /* interface board status */
} HGPIB_t;

typedef struct {
	char    h_rw;
	off_t   h_start_pbn;
	short   h_bcnt;
	caddr_t h_phys_addr;
} HIOCTL_t;
