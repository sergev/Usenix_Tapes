/* Hitachi optical disk and jukebox device driver for wicat */
/* IEEE-488 (GPIB) / multibus interface board from */
/* National Instruments is used to interface to the disk */

/* Author: Jungbo Yang */

#include <sys/param.h>
#include <sys/text.h>
#include <sys/tty.h>
#include <sys/mx.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/dir.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <signal.h>

#include "hitachi.h"
#include "hita.h"
#include "opt_fs.h"

/* #define HITADEBUG              Uncomment to turn driver debugging on */

#undef DEBUG
#ifdef HITADEBUG
#define DEBUG(args)		printf args
#else
#   define DEBUG(args)
#endif

#define TRUE                    1
#define FALSE                   0
#define odd(n)			((int)(n) & 01)
#define UNIT(devnum)            (minor(devnum))
#ifndef void
#define void                    int
#endif
#define wlowbyte(x)	(0xff & x)
#define whibyte(x)	(0xff & (x >> 8))


/* GPIB 796 registers */
typedef struct {   	        /*  bytes are swaped. you know why. */
	char    imr1;           /*  +1 interrupt mask register 1        */
	char    cdor;           /*  +0 byte out register                */
	char    spmr;           /*  +3 serial poll mode register        */
	char    imr2;           /*  +2 interrupt mask register 2        */
	char    auxmr;          /*  +5 auxiliary mode register          */
	char    admr;           /*  +4 address mode register            */
	char    eosr;           /*  +7 end of string register           */
	char    adr;            /*  +6 address register 0/1             */
	char    bcr1;           /*  +9 byte count register 1            */
	char    bcr0;           /*  +8 byte count register 0            */
	char    ccfr;           /* +11 carry cycle function register    */
	char    cr0;            /* +10 control register 0               */
	char    acr1;           /* +13 address count register 1         */
	char    acr0;           /* +12 address count register 0         */
	char    cr1;            /* +15 control register 1               */
	char    acr2;           /* +14 address count register 2         */
} ibregs ;

/*
 *  one 488 interface board with 2 optical disk drives
 */
ibregs *ib = ((ibregs *) 0xf00600);  /* only one board */

#define NHITA		1		/* one OFC */
#define NODD		2
static HITA_t hita[NODD]; 	/* two ODDs */
static HGPIB_t gpib;
#define HFMSSIZE	512	/* fms */
#define HNMINOR		8	/* max number of minor device */
static char fms[HNMINOR][HFMSSIZE];
#define HSTATSS		0xe0	/* status tables */
#define HSTATDEVID	0xe9
#define HSTATCELL	0x68
#define HSTATODDS	0xec
#define HSTATSECT	0x61
static char ss_stat[HSTAT_L_SS+2];
static char devid_stat[HSTAT_L_DEVID+2];
static char cell_stat[HSTAT_L_CELL+2];
static char odd_stat[HSTAT_L_ODDS+2];
static char sect_stat[HSTAT_L_SECT+2];
static char codd = -1;		/* current ODD */
/* proc sync */
static int sleep_cnt = 0;
static int sleep_out = 0;
static int sleep_in = 0;
/* PSleeP should be done before the critical section */
#define PSleeP	{ \
	if (hita[UNIT(dev)].h_state & HBUSY) { \
	    if (++sleep_cnt >= HMAXOPEN) { \
		--sleep_cnt; \
		u.u_error = EBUSY; \
		return; \
	    } \
	    if (++sleep_in >= HMAXOPEN) \
		sleep_in = 0; \
	    sleep( (caddr_t) ss_stat + sleep_in, HITAPRI); \
	} \
	hita[(UNIT(dev))].h_state |= HBUSY; \
}
/* done at the end but inside of the critical section */
#define PWakeuP { \
	if (sleep_cnt) { \
	    if (++sleep_out >= HMAXOPEN) \
		sleep_out = 0; \
	    wakeup( (caddr_t) ss_stat + sleep_out); \
	    sleep_cnt--; \
	} else { \
		hita[UNIT(dev)].h_state &= ~(HBUSY); \
	} \
}

/*
 * hita structure bit definitions
 */
#define HOPEN		0x1	/* state */
#define HREAD		0x1	/* state */
#define HWRITE		0x2	/* state */
#define HITASLEEP	0x4	/* state, process called sleep() */
#define HITAPHYS	0x8	/* state, user buffer is the hidden memory */
#define HEXCLUSIVE	0x10	/* state */
#define HBUSY		0x20	/* state */
#define DUAL		0x1 	/* opop dual write on */
#define DCI		0x2	/* opop delete block check inhibit */

/*
 * CRC checksum (this should really be done in hardware)
 */
#define POLY      0x8408
crc (start, end, crc1, crc2)
register  char  *start, *end, *crc1, *crc2;
{

  register c;
  register counter, data = 0xffff;          /* data will start from all one's */
  register polynom = POLY;

    while (start < end) {
	c = *start++ & 0xFF;
	if (start < end)
	    c |= (0xff00 & (*start++ << 8));
	data ^= c;
	counter = 17;
	while (--counter) {              /* loop for 16 shifts */
	    if (data & 1) {
		data >>= 1;
		data ^= polynom;
	    } else
		data >>= 1;
	}
    }
    *crc1 = (char) ~(data & 0xff);
    *crc2 = (char) ~(data >> 8) & 0xff;
    /* DEBUG(("crc1= %x, crc2= %x\n",0xff&(*crc1),0xff&(*crc2))); */
}

/*
 * Programmed I/O  (busy wait)
 * readone(), cmdone() and dataone()
 */
char readone()
{
    register char t;

    do {
         t = ib->isr1;
	 gpib.h_isr1 |= t;
    }  while ((t & HR_DI) == 0);
    return(ib->dir);
}

cmdone(cmd)
    char cmd;
{
    register char t;

    ib->cdor = cmd;
    do {
	t = ib->isr2;
        gpib.h_isr2 |= t;
    } while ((t & HR_CO) == 0);	/* busy wait until set */
}

dataone(dat)
    char dat;
{
    register char t;

    ib->cdor = dat;
    do {
	t = ib->isr1;
        gpib.h_isr2 |= t;
    }  while ((t & HR_DO) == 0);	/* busy wait until set */
}

/*
 *  wait for SRQ routine  (busy wait)
 */
srq() {
    register char t;

    DEBUG(("waiting for SRQ..."));
    ib->imr2 = HR_SRQIE;
    do  {
	t = ib->isr2;
	gpib.h_isr2 |= t;
    } while ((t & HR_SRQI) == 0);  /* busy wait until set */
    ib->imr2 &= ~HR_SRQIE;
}

/* short delay (busy wait) */
ibdelay(n) {
    int j,i,t;

    for (j=0; j<n; j++)
        for (t=i=0; i<1000; i++)
	    t++;
    return(t);
} 

/*
 * initialize the Interface board
 */
init(tl)
  char tl;
{
    char t;

    /* DEBUG(("init(%c).\n",tl)); */
    ib->cr0 = HR_LMR;		/* set local master reset */
    ib->cr0 = 0;		/* clear it */
    ib->cr1 = 0;		/* clear it */
    ib->auxmr = 2;		/* chip reset */
    t = ib->cptr;		/* clear stat by reading */
    t = ib->isr1;
    t = ib->isr2;
    ib->imr1 = ib->imr2 = ib->spmr = 0;	/* disable all interrupt */
    ib->adr = 0;		/* set MTA=0100 and MLA=040 */
    ib->adr = HR_ARS | HR_DT | HR_DL;
    ib->admr = HR_TRM1 | HR_TRM0 | HR_ADM0;
    if (tl == 't')
        ib->admr = HR_TON;
    else
        ib->admr = HR_LON;
    ib->auxmr = ICR | 5;
    ib->auxmr = PPR | HR_PPU;
    ib->auxmr = AUXRA | 0;
    ib->auxmr = AUXRB | HR_INV;
    ib->auxmr = AUXRE | 0;
    ib->cr1 = HR_MIE | HR_SC;	/* main interrupt switch on */
    ib->auxmr = AUX_PON;	/* power on */
}

/*
 * GPIB ACTIVE state
 */
active(tl)
  char tl;
{
    /* DEBUG(("active(%c).\n",tl)); */
    /* become active controller */
    ib->cr1 = HR_SC;		/* prepare for IFC and REN */
    if (tl == 't')
        ib->admr = HR_TON;
    else
        ib->admr = HR_LON;
    ib->auxmr = AUX_PON;	/* power on */

    ib->auxmr = AUX_SIFC;	/* set IFC */
    ibdelay(1); 		/* > 100 micro seconds delay */
    ib->auxmr = AUX_CIFC;	/* clear IFC */
    /* now, check to see I am active or not ***************
    if (ib->adsr & HR_CIC) {
	if (tl == 't')
		if (ib->adsr & HR_TA)
		    DEBUG(("Active talker. ok.\n"));
		else
		    DEBUG(("Active talker. NOT ok.\n"));
	else
		if (ib->adsr & HR_LA)
		    DEBUG(("Active listenner. ok.\n"));
		else
		    DEBUG(("Active listenner. NOT ok.\n"));
    }
    ***************** */
}

/* LISTEN and STANDBY for data in */
listen_sb() {
	/* DEBUG(("listen and standby for data in\n")); */
        ib->auxmr = LISTEN;
        ib->auxmr = SB;
}

/* EOI with the final data out */
eoi() {
    /* DEBUG(("eoi().\n")); */
    ib->auxmr = AUX_SEOI;
}

/*
 * POWER ON to reset some parameters, such as talker/listenner
 *     Also, this causes the state to shift from ACTIVE to PASSIVE
 */
pon() {
    /* DEBUG(("pon().\n")); */
    ib->auxmr = AUX_PON;
}

/*
 * DMA i/o with interrrupt
 */
int dma(dev,rw,addr,cnt)
    dev_t dev;
    int cnt;
    char rw;
    char *addr;
{
    register HITA_t *g = &hita[UNIT(dev)];
    int ret;
    char t;
    register int i,fcnt;
    register char *faddr;

    /* state should be the active in standby mode */
    fcnt = cnt;
    faddr = addr;

    /* set the byte count and address*/
    cnt = -cnt;
    DEBUG(("dma: %s cnt = %d\n",rw=='r'?"read":"write",cnt));
    ib->bcr0 = (char) (0xff & cnt);
    ib->bcr1 = (char) (0xff & (cnt >> 8));
    ib->acr0 = lobyte(addr);	/* physical memory addr */
    ib->acr1 = midbyte(addr);
    ib->acr2 = hibyte(addr);

    if (rw == 'r') {
	t = ib->isr1;           /* clear it */
	ib->auxmr = AUX_FH;
	ib->auxmr = AUXRA;      /* normal handshake */
	ib->ccfr = 0;
	ib->imr1 = HR_ENDIE;    /* set for reading */
	ib->imr2 = HR_DMAI;

	/* DEBUG(("dma: read: sleeping...")); */
	g->h_state |= HITASLEEP;
        ib->cr1 = HR_MIE|HR_SC;
	ib->cr0 = HR_DMAE|HR_FINIE;
	ib->cr0 = HR_DMAE|HR_FINIE|HR_GO;
	sleep( (caddr_t) g, HITAPRI);
	/* DEBUG(("dma: read: woken up.\n")); */

    } else { /* WRITE */
	t = ib->sr;             /* clear to start */
	ib->imr1 = 0;
	ib->imr2 = HR_DMAO;

	/* DEBUG(("dma: write: sleeping...")); */
	g->h_state |= HITASLEEP;
	if (rw == 'l') {	         /* the last write. send EOI */
            ib->ccfr = AUX_SEOI;         /* send EOI with the last byte */
            ib->cr1 = HR_MIE|HR_SC;
	    ib->cr0 = HR_MEMRD|HR_ECC|HR_DMAE|HR_FINIE;
	    ib->cr0 = HR_MEMRD|HR_ECC|HR_DMAE|HR_FINIE|HR_GO;
	} else {
            ib->ccfr = 0;
            ib->cr1 = HR_MIE|HR_SC;
	    ib->cr0 = HR_MEMRD|HR_DMAE|HR_FINIE;
	    ib->cr0 = HR_MEMRD|HR_DMAE|HR_FINIE|HR_GO;
	}
	sleep( (caddr_t) g, HITAPRI);
	/* DEBUG(("dma: write: woken up.\n")); */
    }
    ib->cr0 = 0;            /* clear DMA */
    /* g->h_state &= ~(HITASLEEP);   done in hitaintr() */
    gpib.h_sr    &= ~(HR_NGPIBIR|HR_NFIN|HR_DONE);

    ret = (ib->bcr1 << 8) + (unsigned char) ib->bcr0;
    DEBUG(("dma: byte leftover = %d\n",-ret));
    return(ret-cnt);
}

getstat(dev)
dev_t dev;
{
    register int i;
    char t,ret;
    register HITA_t *g = &hita[UNIT(dev)];

   /* catch the SQR */
    gpib.h_isr2 &= ~(HR_SRQI);

    /* set the ib to active Listenner */
    active('l');

    /* wait for SRQ */
    if ((gpib.h_isr2 & HR_SRQI) == 0)
        srq();

    cmdone(UNL);
    cmdone(UNT);
    cmdone(SPE);
    cmdone(g->h_mta);

    /* aux listen and standby command */
    listen_sb();

    /* read one byte status */
    ret = 0xff & readone();
    DEBUG(("getstat: %x\n",ret));

    /* set the ib back to active mode for sending commands */
    active('t');

    cmdone(SPD);
    cmdone(UNT);
    cmdone(UNL);

    /* save the status, when they are available, here */
    if (ret==0x80 || ret==0x40 || ret==0x10) {
	return(0);
    } else {
	active('l');
	cmdone(UNT);
	cmdone(UNL);
	cmdone(g->h_mta);
	listen_sb();
	gpib.h_isr1 &= ~(HR_END);
	for (i=0; i<6; i++)		/* through away the header */
	    readone();
	i=0;
        switch(0xff & ret) {
	    case HSTATSS:
	        while(1) {
		    ss_stat[i++] = readone();
	            if ((gpib.h_isr1 & HR_END) || i>HSTAT_L_SS+2)
		        break;
	        }
		break;
	    case HSTATDEVID:
	        while(1) {
		    devid_stat[i++] = readone();
	            if ((gpib.h_isr1 & HR_END) || i>HSTAT_L_DEVID+2)
		        break;
	        }
		break;
	    case HSTATCELL:
	        while(1) {
		    cell_stat[i++] = readone();
	            if ((gpib.h_isr1 & HR_END) || i>HSTAT_L_CELL+2)
		        break;
	        }
		break;
	    case HSTATODDS:
	        while(1) {
		    odd_stat[i++] = readone();
	            if ((gpib.h_isr1 & HR_END) || i>HSTAT_L_ODDS+2)
		        break;
	        }
		break;
	    case HSTATSECT:
	        while(1) {
		    sect_stat[i++] = readone();
	            if ((gpib.h_isr1 & HR_END) || i>HSTAT_L_SECT+2)
		        break;
	        }
		break;
	    default:
	        while(1) {
		    readone();
	            if (gpib.h_isr1 & HR_END) /* unknown stat. dump it */
		        break;
	        }
		break;
	}
        return(0xff & ret);
    }
}

hitainit(u)
register short u;
{
    register HITA_t *g;
    int i;

    DEBUG(("hitainit called:\n"));
    g = &hita[u];
    g -> h_state  = 0;
    g -> h_opfd  = 0;
    g -> h_opop = 0;
    g -> h_pbn = -1;	/* not initialized */
    g -> h_woff = 0;	/* empty */
    g -> h_roff = 512;
    g -> h_mta    = MTA + u;
    g -> h_mla    = MLA + u;
    sleep_cnt = sleep_in = sleep_out = 0;
}

/*
Fd is available in u.
	fd.f_flag 1==read(FREAD) and 2==write(FWRITE)
	rwflag =1 for FREAD and 2 for FWRITE and 0 for exclusive?
Allow only one WRITE open (mode 1,2 or 3)
    is hita.state HWRITE?
Check the max number of open
    is opfd < HMAXOPEN?
Increment opfd
Set hita.stat=HWRITE
*/
hitaopen(dev,rwflag)
dev_t dev;
short rwflag;
{
    register HITA_t *g;
    register struct a {
	char *fname;
	int fmode;
    } *uap;
    int i,x;
    HFMS_t *pfms;

    uap = (struct a *) u.u_ap;
    DEBUG (("HITA OPEN called, dev = %d/%d, rwflag=%x, h.state=%x, mode=%x\n",
	 major(dev), UNIT(dev),rwflag,hita[UNIT(dev)].h_state,uap->fmode));

    if (codd < 0) {		/* first open */
	for (i=0; i<NODD; i++) {
	    hitainit(i);
	    /* init fms states */
    	    for (i=0; i<HNMINOR; i++) {
		pfms = (HFMS_t *) &fms[i][0];
		pfms->fms_state = FMS_NOT_MOUNTED;
		pfms->fms_vol[0] = 0;
    	    }
	}
        gpib.h_isr1 = gpib.h_isr2 = gpib.h_sr = codd = 0;
        init('t');
        active('t');
        cmdone(DCL);
        cmdone(UNT);
        cmdone(UNL);
    }
    PSleeP
    x = spl7();

    g = &hita[UNIT(dev)];
    if (g->h_opfd <= 0)		/* IMPORTANT you do it first */
	hitainit(UNIT(dev));

    if ((UNIT(dev)) >= NODD || g->h_opfd > HMAXOPEN) {
        u.u_error = EMFILE;              /* too many open */
        goto op_ret;
    }

/* NOTE
 *    WICAT unix passes 0 for read and 2 for write.
	most of unix pass 1 for read, 2 for write and 3 for both
 *    therefore there is no way to know it is opening for mode 1 or 2
 *
 *    So, i have to get the mode from u structure!
 */
    rwflag = uap->fmode;
    /*
     * EXCLUSIVE open is done. no open is allowd
     */
    if ( (g->h_state & HEXCLUSIVE) ||
    /*
     * EXCLUSIVE open failes, when any open is already done
     */
    (((rwflag&(HREAD|HWRITE))==(HREAD|HWRITE)) && g->h_opfd) ||
    /*
     * only one open for WRITE
     */
    ((rwflag&HWRITE) && (g->h_state & HWRITE)) )
    {
        u.u_error = ENXIO;              /* Open fails */
        goto op_ret;
    }
    g->h_opfd += 1;

    g -> h_state  |= (HOPEN | (rwflag & (HREAD|HWRITE)));
    if ( (rwflag & (HREAD|HWRITE))==(HREAD|HWRITE))
        g -> h_state |= HEXCLUSIVE;

op_ret:
    PWakeuP
    splx(x);
    DEBUG(("OPEN end: h.state=%x\n",g->h_state));
}

hitaclose(dev,flag)
dev_t dev;
int flag;
{
    register HITA_t *g;
    int x;

    DEBUG (("HITA CLOSE called, dev = %d/%d opfd=%d, flag=%x, h.state=%x\n",
	 major(dev), UNIT(dev),g->h_opfd,flag,hita[UNIT(dev)].h_state));

    PSleeP
    x = spl7();

    g = &hita[UNIT(dev)];
/*  ! fd is already closed before it gets here */
/*  close is only called at the last close on a unit */
    g->h_state = 0;
    g->h_opfd = 0;
    PWakeuP
    splx(x);
    DEBUG(("CLOSE end: h.state=%x\n",g->h_state));
}

/*
 * ** NOTE **
 * The WICAT UNIX system does not pass the UNIT device number to the
 * interrupt handler as most UNIX systems do.  So we must treat one
 * interrupt as an interrupt from each.
 */
hitaintr()
{
    register int i;

    DEBUG (("HITA INTERRUPT called\n"));

    gpib.h_sr   |= ib -> sr;    /* clears FIN in sr */
    gpib.h_isr1 |= ib -> isr1;	/* clear the interrupt by reading */
    gpib.h_isr2 |= ib -> isr2;

    /* is this my interrupt? */
    if ( gpib.h_isr2 & HR_INTR ) {
	for (i=0; i<NODD; i++) {
	    if (hita[i].h_state & HITASLEEP) {
	        hita[i].h_state &= ~(HITASLEEP);
	        DEBUG (("intr: waking up\n"));
	        wakeup( (caddr_t) (&hita[i]));
	    }
	}
    }
}

/*
 * The Recent versions of UNIX (recent being later than V7)
 * have an additional parameter to physio: physio(..., minphy).
 * Although not used in the Xenix kernel, I've included it
 * here in case this driver is ported to a newer release.
 */
hitaminphys()
{
    /* No minimum i/o */
}

/*
 * Setup wbuf,rbuf and woff,roff according to the given offset
 *   Should be called to verify the current buffer is in sync
 *   at the beginning of read() and write()
 */
int set_offset(dev,offset, rwflag)
dev_t dev;
off_t offset;
char rwflag;
{
    register HITA_t *g = &hita[UNIT(dev)];
    off_t tpbn;
    int toff;

    tpbn = offset/HBLOCKSIZE;
    toff = offset%HBLOCKSIZE;

    /* DEBUG(("set_offset: pbn=%d, off=%d, g->pbn=%d\n",tpbn,toff,g->h_pbn)); */

    if (rwflag == B_READ) {
	if (g->h_pbn != tpbn)
	    if ((g->h_pbn = rfill(dev,0,tpbn,0,0)) < 0) {
		g->h_pbn = -1;
		return(-1);
	    }
	g->h_roff = toff;
    } else { /* WRITE */
	if (g->h_pbn != tpbn) {
	    if (g->h_woff)
		if (wflush(dev,0,g->h_pbn,0,0) < 0) {
		    g->h_pbn = -1;
		    return(-1);
		}
	    g->h_pbn = tpbn;
	}
	g->h_woff = toff;
    }
    return(0);
}

hitarw(dev, offset, count, flags)
dev_t dev;
off_t offset;
unsigned int count;
char flags;
{
    register HITA_t *g = &hita[UNIT(dev)];
    register struct file *fp;
    register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
    } *uap;

    DEBUG (("HITArw %s offset=%d, count=%d\n",
        flags & B_READ ? "READ":"WRITE", offset, count));

    if (count == 0)
	return;
    if ((UNIT(dev)) >= NODD)
    {
        u.u_error = ENXIO;
        return;
    }
    uap = (struct a *)u.u_ap;
    GETF(fp, uap->fdes);

    DEBUG (("HITA XFER: %d bytes\n", count));

/*
 *  IMPORTATANT! only one exit from this routine after this point
 */

/*
 * adjust the device buffer and offset
 */
    if (set_offset(dev,offset,flags) < 0)
	goto error_ret;

/*
 * count = how many more to read.
 * offset = offset from the beginning of the file
 */
    if (flags == B_READ) {
	register int t;
	if (count >= (t = HBUFSIZE-(g->h_roff))) {
	    if (t) {
                iomove(&g->h_rbuf[g->h_roff],t,B_READ);   /* empty the buffer */
	        count -= t;
	    }
	    for(; count>=HBUFSIZE; count -= HBUFSIZE) {
		if (rfill(dev,0,++g->h_pbn,0,0) < 0) {
		    g->h_pbn = -1;
		    goto ok_ret;
		}
                iomove(g->h_rbuf,HBUFSIZE,B_READ);
	    }
	    if (rfill(dev,0,++g->h_pbn,0,0) < 0) {
		    g->h_pbn = -1;
		    goto ok_ret;
		}
	}
	if (count) {
            iomove(&g->h_rbuf[g->h_roff],count,B_READ); /* read from rbuf */
	    g->h_roff +=count;
	}
    } else { /* WRITE */
	register int t;
	if (count >= (t = HBUFSIZE-(g->h_woff))) {
	    if (t) {
                iomove(&g->h_wbuf[g->h_woff],t,B_WRITE);   /* fill it up */
	        count -= t;
	    }
	    for(; count>=HBUFSIZE; count -= HBUFSIZE) {
		if (wflush(dev,0,g->h_pbn,0,0) < 0) {
		    g->h_woff = 0;
		    goto error_ret;
		}
		g->h_pbn += 1;
                iomove(g->h_wbuf,HBUFSIZE,B_WRITE);
	    }
	    if (wflush(dev,0,g->h_pbn,0,0) < 0) {
		    g->h_woff = 0;
		    goto error_ret;
	    }
	    g->h_pbn += 1;
	}
	if (count) {
            iomove(&g->h_wbuf[g->h_woff],count,B_WRITE);
	    g->h_woff +=count;
	}
    }
    goto ok_ret;

error_ret:
    u.u_error = EIO;
    g->h_pbn = -1;

ok_ret:
    return;

}

hitaread(dev)
dev_t dev;
{
    int x;
    /* DEBUG (("HITA READ called, dev = %d/%d\n", major(dev), UNIT(dev))); */
    PSleeP
    x = spl7();
    hitarw(dev,(int) u.u_offset, u.u_count, B_READ);
    PWakeuP
    splx (x);
}

hitawrite(dev)
dev_t dev;
{
    int x;
    /* DEBUG(("HITA WRITE: dev=%d/%d, count=%d\n",
		major(dev),UNIT(dev),u.u_count)); */
    PSleeP
    x = spl7();
    hitarw(dev,(int) u.u_offset, u.u_count, B_WRITE);
    PWakeuP
    splx (x);
}

ss(dev,cmd,paddr,lmta,lmla)
    dev_t dev;
    int cmd;
    char lmta,lmla;
    caddr_t paddr;
{
    register int i;
    char ret,t,crc1,crc2;
    char dd[24];
    char s[300];
    register HITA_t *g = &hita[UNIT(dev)];

    DEBUG(("SS(%d,mta=%d,mla=%d) called\n",cmd,lmta,lmla));

    /* init the ib and set it to active taker */
    active('t'); /* active */
    cmdone(UNT);
    cmdone(UNL);
    cmdone(lmla);
    pon();

    /* send the LOCK (hitachi) command as data */
    dataone(CONTROL_OP);
    dataone(SS_OR);
    switch(cmd) {
	case 's':
	    /* DEBUG(("Read secondary status\n")); */
            dataone(READSS);
	    break;
	case 't':
	    /* DEBUG(("Sector status\n")); */
            dataone(SECTOR);
	    break;
	case 'c':
	    /* DEBUG(("Cell status\n")); */
            dataone(CELLSTAT);
	    break;
	case 'o':
	    /* DEBUG(("OD status\n")); */
	    dataone(ODSTAT);
	    break;
	case 'd':
	    /* DEBUG(("Device ID status\n")); */
	    dataone(DEVID);
	    break;
    }
    dataone(~(CONTROL_OP));
    dataone(~(SS_OR));
    switch(cmd) {
	case 's':
            dataone(~(READSS));
	    break;
	case 't':
            dataone(~(SECTOR));
	    break;
	case 'c':
            dataone(~(CELLSTAT));
	    break;
	case 'o':
	    dataone(~(ODSTAT));
	    break;
	case 'd':
	    dataone(~(DEVID));
	    break;
    }
    dd[0] = SS_POP;
    for (i=1; i<18; i++)
	dd[i] = 0;
    crc(dd,&dd[17],&crc1,&crc2);
    for (i=0; i<18; i++)
	dataone(dd[i]);
    dataone(crc1);
    /* send EOI */
    eoi();
    dataone(crc2);

    if (getstat(dev)) {
	switch(cmd) {
	case 's':
	    /* copyout(ss_stat,paddr,HSTAT_L_SS);  just save it */
	    break;
	case 't':
	    copyout(sect_stat,paddr,HSTAT_L_SECT);
	    break;
	case 'c':
	    copyout(cell_stat,paddr,HSTAT_L_CELL);
	    break;
	case 'o':
	    copyout(odd_stat,paddr,HSTAT_L_ODDS);
	    break;
	case 'd':
	    copyout(devid_stat,paddr,HSTAT_L_DEVID);
	    break;
	}
	return(0);
    }
    return(EFAULT);
}

control(dev,cmd,paddr,lmta,lmla)
    dev_t dev;
    int cmd;
    char lmta,lmla;
    caddr_t paddr;
{
    register int i;
    char side = 0;
    int cell;
    char crc1,crc2,dd[24];
    char s[32];

DEBUG (("CONTROL(cmd=%c,paddr=%x,mta=%d,mla=%d) called\n",cmd,paddr,lmta,lmla));

    /* init the ib and set it to active taker */
    active('t'); /* active */

    /* init the hitachi */
    cmdone(UNT);
    cmdone(UNL);
    cmdone(lmla);

    pon();

    /* send the LOCK (hitachi) command as data */
    dataone(CONTROL_OP);
    dataone(CONTROL_OR);
    switch(cmd) {
	case 'm':
	    DEBUG(("Mount/Lock\n"));
	    cell = whibyte( ((int) paddr) );
	    if (cell < 0 || cell > 31) {
		DEBUG(("Invalid cell number (%d)\n",cell));
		return(HERR_BADCELL);
	    }
	    side = wlowbyte( ((int) paddr) );
	    if (side != 'a' && side != 'b') {
		DEBUG(("Invalid side (%x)\n",side));
		return(HERR_BADSIDE);
	    }
            dataone(MOUNT);
	    break;
	case 'd':
	    DEBUG(("Demount/Unlock\n"));
	    cell = whibyte( ((int) paddr) );
	    if (cell < 0 || cell > 31) {
		DEBUG(("Invalid cell number (%d)\n",cell));
		return(HERR_BADCELL);
	    }
	    side = 'a';
            dataone(DEMOUNT);
	    break;
	case 'l':
	    /* DEBUG(("Lock\n")); */
	    dataone(MOUNT);
	    break;
	case 'u':
	    /* DEBUG(("Unlock\n")); */
	    dataone(DEMOUNT);
	    break;
	case 'c':
	    /* DEBUG(("Change\n")); */
	    dataone(CHANGE);
	    break;
	case 'e':
	    /* DEBUG(("Eject\n")); */
	    cell = whibyte( ((int) paddr) );
	    if (cell < 0 || cell > 31) {
		DEBUG(("Invalid cell number (%d)\n",cell));
		return(HERR_BADCELL);
	    }
	    side = wlowbyte( ((int) paddr) );
	    if (side != 'i' && side != 'o') {
		DEBUG(("Invalid side (%x)\n",side));
		return(HERR_BADSIDE);
	    }
	    dataone(EJECT);
	    break;
	case 'r':
	    /* DEBUG(("Rezero\n")); */
	    dataone(REZERO);
	    break;
	case 'n':
	    /* DEBUG(("Noop\n")); */
	    dataone(NOP);
	    break;
    }
    dataone(~(CONTROL_OP));
    dataone(~(CONTROL_OR));
    switch(cmd) {
	case 'm':
            dataone(~(MOUNT));
	    break;
	case 'd':
            dataone(~(DEMOUNT));
	    break;
	case 'l':
            dataone(~(MOUNT));
	    break;
	case 'u':
            dataone(~(DEMOUNT));
	    break;
	case 'c':
	    dataone(~(CHANGE));
	    break;
	case 'e':
	    dataone(~(EJECT));
	    break;
	case 'r':
	    dataone(~(REZERO));
	    break;
	case 'n':
	    dataone(~(NOP));
	    break;
    }
    if (side) {   /* mount or demount, send cell format (2 bytes) */
	dd[0] = 0;
	dd[1] = 0;
	dd[2] = 0;
	switch(side) {
	    case 'a':
	        dd[3] = 0x40;	/* de/mount side a */
		break;
	    case 'b':
	        dd[3] = 0xc0;	/* de/mount side b */
		break;
	    case 'i':
		dd[3] = 0;	/* eject. store only */
		break;
	    case 'o':
		dd[3] = 0x40;	/* eject. eject out and store in */
	}
	dd[4] = cell;
        for (i=5; i<18; i++)
	    dd[i] = 0;
	crc(dd,dd+17,&crc1,&crc2);
        for (i=0; i<18; i++)
	    dataone(dd[i]);
	dataone(crc1);
	eoi();
        dataone(crc2);
    } else {
        for (i=0; i<18; i++)  /* need to put Cell number for mount/demount */
	    dd[i] = 0;
	crc(dd,dd+17,&crc1,&crc2);
        for (i=0; i<18; i++)  /* need to put Cell number for mount/demount */
	    dataone(dd[i]);
        dataone(crc1);
	eoi();
        dataone(crc2);
    }

    return(getstat(dev)); /* FIX it to return a correct error code */
}

/*
 * Seek (in a character device!?)
 *    Hitachi character dev is special.
 *    An extra entry in the file struct. I think its needed it, anyway.
 */
hseek(dev,paddr)
caddr_t paddr;
{
    register HITA_t *g = &hita[UNIT(dev)];
    register struct file *fp;
    register struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
    } *uap;

/* flush the read buffer whenever SEEK is done */
    g->h_roff = HBLOCKSIZE;

	uap = (struct a *)u.u_ap;
	GETF(fp, uap->fdes);
/*
 * If it is writing and the wbuffer is not empty, it's flushed.
 */
	if (fp->f_flag&FWRITE && g->h_woff)
		if (wflush(dev,0,g->h_pbn,0,0) < 0) {
		    u.u_error = EIO;
		    return;
		}
	g->h_woff = 0;
	fp->f_un.f_offset = (long)paddr * HBLOCKSIZE;
	/* DEBUG(("hseek: offset=%d, paddr=%d\n",
		fp->f_un.f_offset, (long)paddr)); */
}

/*
 * flush the wbuf to the disk (actual i/o happens here)
 *  woff = 0
 *  init wbuf with 0
 *  returns pbn to where the block is written or -1 on error
 */
off_t wflush(dev,fg,pbn,pbcnt,pphys)
dev_t dev;
char fg;
off_t pbn;
int pbcnt;
caddr_t pphys;
{
    register HITA_t *g = &hita[UNIT(dev)];
    register int j,i;
    int cell;
    char ret,crc1,crc2,dd[24];
    long offs;
    int fd,bcnt;

    DEBUG(("wflush: pbn=%d\n",pbn));

    if (!fg) {
	pbcnt = 1;
	pphys = 0;
    }
    /* init the ib and set it to active taker */
    active('t'); /* active */

    /* ready to talk to hitachi */
    /* init the hitachi */
    cmdone(UNT);
    cmdone(UNL);
    cmdone(g->h_mla);
    pon();

    /* send the LOCK (hitachi) command as data
     * Write/alt track
     */
    dataone(CONTROL_OP);
    dataone(0x1);
    dataone(0xe0);
    dataone(~(CONTROL_OP));
    dataone(~(0x1));
    dataone(~(0xe0));
    dd[0] = 0xc0;	/* POP for data transfer with no CRC */
    dd[1] = dd[2] = 0;
    dd[3] = 0xff & (pbn >> 24);		/* pbn */
    dd[4] = 0xff & (pbn >> 16);
    dd[5] = 0xff & (pbn >> 8);
    dd[6] = 0xff & pbn;
    dd[7] = 0xff & (pbcnt >> 8);  	/* block count, hibyte */
    dd[8] = 0xff & pbcnt;		/* lowbyte */
    dd[9]=dd[10]=dd[11]=dd[12] = 0;	/* offset for dual write */
    for (i=13; i<18; i++)
	dd[i] = 0;
    crc(dd,&dd[17],&crc1,&crc2);
    for (i=0; i<18; i++) {		/* send the command */
	dataone(dd[i]);
    }
    dataone(crc1);
    eoi();
    dataone(crc2);

    if (getstat(dev) == 0) {
/* this is done in getstat()
	active('t');
	cmdone(UNT);
	cmdone(UNL);
*/
	cmdone(g->h_mla);
	pon();
	dataone(0xc0);  /* data trx again, must match with POP */
	dataone(0);
	dataone(0);
	dataone(~(0xc0));
	dataone(0xff);
	dataone(0xff);
	if (fg) {
	    for(; pbcnt>=63; pbcnt -= 63, pphys += 32256) {
		if (pbcnt == 63)
			break;
		if (dma(dev,'w',pphys,32256) != 32256) {  /* 63*512 */
		        pbn = -1;
			pbcnt = 0;
			break;
		}
	    }
	    if (pbcnt) {
	        if (dma(dev,'l',pphys,pbcnt*512) != (pbcnt*512))
		    pbn = -1;
	    }
	    if (getstat(dev))
		pbn = -1;
	} else {
	    if ( ((dma(dev,'l',g->h_wbuf,HBUFSIZE)) != HBUFSIZE) ||
               getstat(dev) )
                pbn = -1;
	}
    } else {
	pbn = -1;
    }
/* done in getstat()
    active('t');
    cmdone(UNT);
    cmdone(UNL);
*/
    g->h_woff = 0;
    for (i=0; i<HBUFSIZE; i++)
	g->h_wbuf[i] = 0;
    return(pbn);
}

/*
 * fill the rbuf
 *  set hita.roff = 0
 *  returns pbn from where the data is read or -1 on error
 */
off_t rfill(dev,fg,pbn,pbcnt,pphys)
dev_t dev;
char fg;
off_t pbn;
int pbcnt;
caddr_t pphys;
{
    register int i,j;
    register HITA_t *g = &hita[UNIT(dev)];
    char ret,crc1,crc2,dd[24],con[12];

    DEBUG(("rfill: pbn=%d\n",pbn));

    if (!fg) {
	pbcnt = 1;
	pphys = 0;
    }

    /* init the ib and set it to active taker */
    active('t'); /* active */

    /* init the hitachi */
    cmdone(UNT);
    cmdone(UNL);
    cmdone(g->h_mla);
    pon();

    /* send the LOCK (hitachi) command as data */
    dataone(CONTROL_OP);
    /* Logical Read */
    dataone(RD_OR);
    dataone(RDL_NO);
    dataone( ~(CONTROL_OP));
    dataone( ~(RD_OR));
    dataone( ~(RDL_NO));

    dd[0] = 0xc0;	/* POP for data transfer with no CRC */
    dd[1] = dd[2] = 0;
    dd[3] = 0xff & (pbn >> 24);
    dd[4] = 0xff & (pbn >> 16);
    dd[5] = 0xff & (pbn >> 8);
    dd[6] = 0xff & pbn;
    dd[7] = 0xff & (pbcnt >> 8);
    dd[8] = 0xff & pbcnt;
    dd[9]=dd[10]=dd[11]=dd[12] = 0; 	/* offset */
    for (i=13; i<18; i++)
	dd[i] = 0;
    for (i=0; i<18; i++)
	dataone(dd[i]);
    crc(dd,dd+17,&crc1,&crc2);
    dataone(crc1);
    eoi();
    dataone(crc2);

    if (getstat(dev) == 0) {
	    active('l');
	    cmdone(UNT);
	    cmdone(UNL);
	    cmdone(g->h_mta);
	    listen_sb();
	    /* read and print */
	    gpib.h_isr1 &= ~(HR_END);
	    for (i=0; i<6; i++)  /* through away the header for now */
	        readone(dev);
	    if (fg) { /* fast read */
	        for(; pbcnt>=63; pbcnt -= 63, pphys += 32256) {
		    if (dma(dev,'r',pphys,32256) != 32256) {  /* 63*512 */
		        pbn = -1;
			pbcnt = 0;
			break;
		    }
	        }
	        if (pbcnt) {
	            if (dma(dev,'r',pphys,pbcnt*512) != (pbcnt*512))
		        pbn = -1;
	        }
		if (getstat(dev))
		    pbn = -1;
	    } else {
	        if( ((i=dma(dev,'r',g->h_rbuf,HBUFSIZE)) != HBUFSIZE) ||
    	           getstat(dev) )
		    pbn = -1;
	    }
    } else {
	pbn = -1;
    }
/* done in getstat()
    active('t');
    cmdone(UNT);
    cmdone(UNL);
*/
    if (!fg)
        g->h_roff = 0;
    return(pbn);
}

hflush(dev)
{
    register HITA_t *g = &hita[UNIT(dev)];
    register struct file *fp;
    register struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
    } *uap;

	uap = (struct a *)u.u_ap;
	GETF(fp, uap->fdes);
	if (fp->f_flag&FWRITE && g->h_woff)
		if (wflush(dev,0,g->h_pbn,0,0) < 0) {
		    u.u_error = EIO;
		    return;
		}
	++g->h_pbn;
	g->h_woff = 0;
}

hsrchunwtn(dev,paddr)
    dev_t dev;
    caddr_t paddr;
{
    register HITA_t *g = &hita[UNIT(dev)];
    register int i,j;
    char ret,crc1,crc2,dd[300];
    HIOCTL_t io;
    off_t pbn;

    active('t'); /* active */
    cmdone(UNT);
    cmdone(UNL);
    cmdone(g->h_mla);
    pon();
    /* Search Unwritten block */
    dataone(CONTROL_OP);
    dataone(RD_OR);
    dataone(RD_SU);
    dataone(~(CONTROL_OP));
    dataone(~(RD_OR));
    dataone(~(RD_SU));

    dd[0] = 0;		/* search unwritten command */
    dd[1] = dd[2] = 0;
    copyin( paddr, &io, sizeof(HIOCTL_t));
    /* DEBUG(("HSRCHUNWTN: pbn=%d, bcnt=%d\n",io.h_start_pbn,io.h_bcnt)); */
    dd[3] = 0xff & (io.h_start_pbn >> 24);
    dd[4] = 0xff & (io.h_start_pbn >> 16);
    dd[5] = 0xff & (io.h_start_pbn >> 8);
    dd[6] = 0xff & io.h_start_pbn;
    dd[7] = 0xff & (io.h_bcnt >> 8);
    dd[8] = 0xff & io.h_bcnt;
    for (i=9; i<18; i++)
        dd[i] = 0;
    for (i=0; i<18; i++)
	    dataone(dd[i]);
    crc(dd,dd+17,&crc1,&crc2);
    dataone(crc1);
    eoi();
    dataone(crc2);

/* get the sector status to find the result */
    if (getstat(dev) == 0x61) {
	    if (sect_stat[4] == 0 && sect_stat[5] == 0) {
		/* DEBUG(("HSRCHUNWTN: failed.\n")); */
		u.u_error = EFAULT;
	    } else {
	        pbn = 0xff & sect_stat[0];
	        pbn = (pbn << 8) + (0xff & sect_stat[1]);
	        pbn = (pbn << 8) + (0xff & sect_stat[2]);
	        pbn = (pbn << 8) + (0xff & sect_stat[3]);
	        /* DEBUG(("HSRCHUNWTN: unwritten pbn = %d\n",pbn)); */
	        io.h_start_pbn = pbn;
	        copyout(&io, paddr, sizeof(HIOCTL_t));
	    }
    } else {
        u.u_error = EFAULT;
    }
    active('t');
    cmdone(UNT);
    cmdone(UNL);
}

hdelete(dev,paddr)
    dev_t dev;
    caddr_t paddr;
{
    register HITA_t *g = &hita[UNIT(dev)];
    register int i,j;
    HIOCTL_t io;
    char ret,crc1,crc2,dd[24];

    active('t'); /* active */
    cmdone(UNT);
    cmdone(UNL);
    cmdone(g->h_mla);
    pon();
    /* Delete */
    dataone(CONTROL_OP);
    dataone(0x11);
    dataone(0x80);
    dataone(~(CONTROL_OP));
    dataone(~(0x11));
    dataone(~(0x80));

    dd[0] = 0;		/* search unwritten command */
    dd[1] = dd[2] = 0;
    copyin( paddr, &io, sizeof(HIOCTL_t)); /* hope bytes are in right order */
    /* DEBUG(("HDELETE: start_pbn=%d, bcnt=%d\n", io.h_start_pbn,io.h_bcnt)); */
    dd[3] = 0xff & (io.h_start_pbn >> 24);
    dd[4] = 0xff & (io.h_start_pbn >> 16);
    dd[5] = 0xff & (io.h_start_pbn >> 8);
    dd[6] = 0xff & io.h_start_pbn;
    dd[7] = 0xff & (io.h_bcnt >> 8);	/* block count */
    dd[8] = 0xff & io.h_bcnt;
    for (i=9; i<18; i++)
        dd[i] = 0;
    for (i=0; i<18; i++)
	    dataone(dd[i]);
    crc(dd,dd+17,&crc1,&crc2);
    dataone(crc1);
    eoi();
    dataone(crc2);

/* get the sector status to find the result */
    if(getstat(dev))
	u.u_error = EIO;
}

/*
 * fast i/o
 *  does not change the standard I/O buffer
 */
hfast(dev,paddr)
    dev_t dev;
    caddr_t paddr;
{
    register off_t fpbn;
    register int fbcnt;
    register caddr_t fpaddr;
    HIOCTL_t io;

    copyin( paddr, &io, sizeof(HIOCTL_t)); /* hope bytes are in right order */
    /* DEBUG(("HFAST: rw=%c,start_pbn=%d, bcnt=%d, phys_addr=%d\n",
	io.h_rw,io.h_start_pbn,io.h_bcnt,io.h_phys_addr)); */

    if ((fbcnt = io.h_bcnt) < 0 || (fpbn = io.h_start_pbn) < 0 ||
        (fpaddr = io.h_phys_addr) < 0) {
	u.u_error = EIO;
	return;
    }

    DEBUG(("HFAST: pbn=%d, bcnt=%d, phys_addr=%d\n",fpbn,fbcnt,fpaddr));

    if (io.h_rw == 'r') {	/* read */
	    if (rfill(dev,1,fpbn,fbcnt,fpaddr) < 0)
		u.u_error = EIO;
    } else {
	    if (wflush(dev,1,fpbn,fbcnt,fpaddr) < 0)
		u.u_error = EIO;
    }
}

hitaioctl(dev, cmd, paddr)
dev_t dev;
int cmd;
caddr_t paddr;
{
    register HITA_t *g;
    int x;
    register struct file *fp;
    register struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
    } *uap;
    HFMS_t *lfms;

DEBUG(("IOCTL: dev=%d/%d, cmd=%d, addr=%o\n",major(dev),UNIT(dev),cmd));

    PSleeP
    x = spl7();

    uap = (struct a *)u.u_ap;
    GETF(fp, uap->fdes);

    if ((UNIT(dev)) >= NODD)
    {
        u.u_error = ENXIO;
        goto ctl_ret;
    }

    g = &hita[UNIT(dev)];
    switch (cmd)
    {
	case HCLOSEWRITE:
		if (fp->f_flag & HWRITE)
		    g->h_state &= ~(HWRITE|HEXCLUSIVE);
		break;
	case HFAST:
	    	hfast(dev,paddr);
		break;
	case HLGETFMS:
		lfms = (HFMS_t *) &fms[UNIT(dev)][0];
		lfms->fms_state |= FMS_LOCK;
	    	copyout((caddr_t) &fms[UNIT(dev)][0],paddr,sizeof(HFMS_t));
		break;
	case HULFMS:
		lfms = (HFMS_t *) &fms[UNIT(dev)][0];
		lfms->fms_state &= ~FMS_LOCK;
		break;
	case HGETFMS:
	    	copyout((caddr_t) &fms[UNIT(dev)][0],paddr,sizeof(HFMS_t));
		break;
	case HPUTFMS:
		copyin(paddr,(caddr_t) &fms[UNIT(dev)][0],sizeof(HFMS_t));
		break;
	case HDELETE:
	 	hdelete(dev,paddr);
		break;
	case HSRCHUNWTN:
	 	hsrchunwtn(dev,paddr);
		break;
	case HFLUSH:
		hflush(dev);
		break;
	case HSEEK:
		hseek(dev,paddr);
		break;
	case HREZERO:
		u.u_error = control(dev,'r', paddr, g->h_mta, g->h_mla);
		break;
	case HMOUNT:
		u.u_error = control(dev,'m', paddr, g->h_mta, g->h_mla );
		break;
	case HDMOUNT:
		lfms = (HFMS_t *) &fms[UNIT(dev)][0];
		lfms->fms_state = FMS_NOT_MOUNTED;
		lfms->fms_ocnt = 0;
		u.u_error = control(dev,'d', paddr, g->h_mta, g->h_mla );
		break;
	case HLIBOPEN:
		u.u_error = control(dev,'e', paddr, g->h_mta, g->h_mla );
		break;
	case HFLIP:
		u.u_error = control(dev,'c', paddr, g->h_mta, g->h_mla );
		break;
	case HNOOP:
		u.u_error = control(dev,'n', paddr, g->h_mta, g->h_mla );
		break;
	case HSS:
	        copyout(ss_stat,paddr,HSTAT_L_SS);
		u.u_error = ss(dev,'s', paddr, g->h_mta, g->h_mla );
		break;
	case HSECTS:
	        copyout(sect_stat,paddr,HSTAT_L_SECT);
		break;
	case HODDS:
		u.u_error = ss(dev,'o', paddr, g->h_mta, g->h_mla );
		break;
	case HCELLS:
		u.u_error = ss(dev,'c', paddr, g->h_mta, g->h_mla );
		break;
	case HDEVID:
		u.u_error = ss(dev,'d', paddr, g->h_mta, g->h_mla );
		break;
        default:
            u.u_error = EINVAL;
	    break;
    }
ctl_ret:
    PWakeuP
    splx(x);
}
