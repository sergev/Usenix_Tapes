#
/*
 *	comtal display driver
 */

#include "/usr/sys/param.h"
#include "/usr/sys/buf.h"
#include "/usr/sys/conf.h"
#include "/usr/sys/user.h"

struct devtab cttab;

#define CTADDR	0172430

struct {
	int	ctwc;	/* word count	*/
	int	ctba;	/* buff addr	*/
	int	ctst;	/* control stat	*/
	int	ctdb;	/*   >>drdb<<	*/
};

struct	{
	int	mode;		/* software copy of display word */
	int	roll;		/* roll flag;  0 => don't roll	 */
	char	open[4];	/* unit in use flags 	 */
	int	last_com;	/* last command issued 	 */
	struct	{
		int x;		/* tracking cross - x 	 */
		int y;		/* tracking cross - y	 */
	} tc;
}ct;

#define PMEM	3	/* psudo color minor device num. */
#define MAXP	511	/* maximum picture address	 */
#define MAXF	512	/* function memory address	 */
#define MAXG	529	/* max gov addr			 */

#define ONLINE	002000	/* control & status info	 */
#define IENB	0100
#define DISP	(0 | IENB)
#define IMAGE	(3 | IENB)
#define PROC	(5 | IENB)
#define TARG	(7 | IENB)
#define GO2	010

#define GOV	01000	/* "drdb" info */
#define FMEM	040000
#define XBIT	040000

#define ROLL	02000
#define READ	0100000
#define BLOCK	020000
/*page*/
ctopen(dev)
{
	register int unit;

	unit = dev.d_minor;
	if(cttab.d_active == -1 ||ctabort(0) || ct.open[unit] ){
		u.u_error = ENXIO;
		return;
	}
	ct.open[unit]++;
}

ctclose(dev)
{
	register int unit;

	unit = dev.d_minor;
	if(ctabort(0) ){
		u.u_error = ENXIO;
		return;
	}
	ct.open[unit] = 0;
}
/*page*/
ctstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register int unit,blk;

	bp = abp;
	unit = bp->b_dev.d_minor;
	blk = bp->b_blkno;

	/* make sure that this buffer is ok to use */

	if(ctabort(0) || !ct.open[unit] || blk > MAXG){
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}

	/* queue it */

	bp->av_forw = 0;
	spl5();
	if(cttab.d_actf == 0)
		cttab.d_actf = bp;
	else
		cttab.d_actl->av_forw = bp;
	cttab.d_actl = bp;
	if(cttab.d_active == 0)
		ctstart();
	spl0();
}

ctintr()
{
	register struct buf *bp;
	register i;

	if(cttab.d_active > 1){			/* if graphics overlay */
		cttab.d_active--;
		CTADDR->ctwc = 32;
		CTADDR->ctst = ct.last_com;	/* re-issue last com */
		return;
	}
	bp = cttab.d_actf;
	cttab.d_errcnt = 0;
	cttab.d_active = 0;
	if(bp==0)
		return;
	cttab.d_actf = bp->av_forw;
	bp->b_resid = 0;
	iodone(bp);
	ctstart();
}
/*page*/
ctstart()
{
	register struct buf *bp;
	register int com,db;

/*
 *	This device startup routine is overly complicated because
 *	of the bizarre nature of the COMTAL interface.
 */
	if((bp=cttab.d_actf) == 0)
		return;
	cttab.d_active = 1;
	if(bp->b_dev.d_minor == PMEM) {
		/* psudo color */
		CTADDR->ctwc = -64;
		com = PROC|GO2;
		db = BLOCK;
		goto start;
	}
	if((db = bp->b_blkno) == MAXF) {
		/* function memory */
		db = (bp->b_dev.d_minor<<8) | BLOCK | FMEM;
		com = PROC | GO2;
		CTADDR->ctwc = -256;
	} else {
		CTADDR->ctwc = -256;
		com = IMAGE;
		if(bp->b_flags&B_READ)
			com =| GO2;
		if(db > MAXF) {
			/* graphics overlay */
			CTADDR->ctwc = -32;
			cttab.d_active =+ 16;
			db =| GOV;
		}
		db =| (bp->b_dev.d_minor<<11);
		if(ct.roll)
			db =| ROLL;
	}
start:
	if(bp->b_flags & B_READ)
		db =| READ;
	com =| (bp->b_xmem & 03) << 4;
	CTADDR->ctba = bp->b_addr;
	CTADDR->ctdb = db;
	CTADDR->ctst = com;
	ct.last_com = com;
}

/*page*/
ctabort(kk)
{
	register struct buf *bp;
	register int i;

	if(CTADDR->ctst & ONLINE)
		return(0);		/* A - ok */

	printf("\ncomtal is offline! (ctst=%6o)\n");

	for(i=0; i<4; i++)	
		ct.open[i] = 0;		/* lock out future i/o */

					/* flush i/o queue */
	while((bp = cttab.d_actf) != 0) {
		bp->b_flags =| B_ERROR;
		cttab.d_actf = bp->av_forw;
		iodone(bp);
	}

	cttab.d_active = -1;		/* error */
	ctrecover();			/* try to eventually get things going again */
	return(1);			/* signal error */
}

ctrecover()
{

		/* attempt to recover from comtal being offline */

	if(cttab.d_active != -1){
		printf("\ncomtal online\n");
		return;
	}
	if(CTADDR->ctst & ONLINE){
		CTADDR->ctwc = -2;	/* try to get an interupt */
		CTADDR->ctba = &ct.tc;
		CTADDR->ctdb = READ | BLOCK;
		CTADDR->ctst = PROC | GO2;
	}
	timeout(&ctrecover,0,HZ*3);	/* schedule retry */
	putchar(007);
}
