/*
 * Disk driver for SI Fujitsu 160Mb disk
 * using direct emulation.
 *
 * Jeffrey Kodosky
 * National Instruments
 */

#include "../hd/param.h"
#include "../hd/buf.h"
#include "../hd/conf.h"
#include "../hd/user.h"
#include "../hd/systm.h"

struct {
	int     hmcs1;  /* control/status 1 */
	int     hmwc;   /* word count */
	int     hmba;   /* bus address */
	int     hmda;   /* desired track/sector address */
	int     hmcs2;  /* control/status 2 */
	int     hmds;   /* drive status */
	int     hmer1;  /* error 1 */
	int     hmas;   /* attention summary */
	int     hmla;   /* look ahead: not emulated */
	int     hmdb;   /* data buffer */
	int     hmmr1;  /* maintenance: not emulated */
	int     hmdt;   /* drive type */
	int     hmsn;   /* serial number */
	int     hmof;   /* offset */
	int     hmdc;   /* desired cylinder address */
	int     hmhr;   /* holding: not emulated */
	int     hmmr2;  /* maintenance: not emulated */
	int     hmer2;  /* error 2 */
	int     hmec1;  /* error correction 1: not emulated */
	int     hmec2;  /* error correction 2: not emulated */
	int     hmbae;  /* bus address extension */
	int     hmcs3;  /* control/status 3 */
};

#define HMADDR  0176700

#define SPT     32              /* sectors per track */
#define TPC     10              /* tracks per cylinder */

#define BPC     (TPC*SPT)       /* blocks per cylinder */

struct {
	char    *nblocks;
	int     cyloff;
} hm_sizes[] {
	200*BPC,        0,      /* cyl   0 thru 199 */
	200*BPC,        200,    /* cyl 200 thru 399 */
	200*BPC,        423,    /* cyl 423 thru 622 */
	200*BPC,        623,    /* cyl 623 thru 822 */
	23 *BPC,        400,    /* cyl 400 thru 422 */
	0,              0,      /* unused */
	0,              0,      /* unused */
	0,              0       /* unused */
};

struct  devtab  hmtab;
struct  buf     rhmbuf;

int     hmok;           /* controller responds to Unibus address */
int     hmn= 1;         /* number of drives */

#define GO      01      /* hmcs1: drive commands */
#define RCLR    010
#define PRESET  020

#define IENABLE 0100
#define READY   0200
#define MOL     010000  /* hmds: medium on-line */
#define SC      0100000 /* hmcs1: special condition */

#define DCK     0100000 /* hmer1: data check error */
#define UNS     040000  /* hmer1: drive unsafe */
#define ECH     0100    /* hmer1: non-correctable ECC error */

#define CLR     040     /* hmcs2: controller clear */

/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define trksec  av_back
#define cylin   b_resid

hmopen(dev,flag){
	if(!hmok){
		if(tkword(HMADDR)){ u.u_error= ENXIO; return; }
		hmok++;
		HMADDR->hmcs2= CLR;
		HMADDR->hmcs1= RCLR|GO;
		HMADDR->hmcs1= PRESET|GO;
	}       }

hmclose(dev){ }


hmstrategy(abp) struct buf *abp;{
	register struct buf *bp;
	register char *p1, *p2;

	bp= abp;
#ifdef PDP1170
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
#endif
	p1= &hm_sizes[bp->b_dev.d_minor&07];
	if(bp->b_dev.d_minor >= (hmn<<3) || bp->b_blkno >= p1->nblocks || !hmok) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return; }
	bp->av_forw= 0;
	bp->cylin= p1->cyloff;
	p1= bp->b_blkno;
	p2= lrem(p1,SPT);
	p1= ldiv(p1,SPT);
	bp->trksec= (p1%TPC)<<8 | p2;
	bp->cylin=+ p1/TPC;
	spl5();
	if((p1=hmtab.d_actf)==0)
		hmtab.d_actf= bp;
	else {  for(; p2=p1->av_forw; p1=p2)
			if(p1->cylin<=bp->cylin && bp->cylin<p2->cylin
				|| p1->cylin>=bp->cylin && bp->cylin>p2->cylin)
				break;
		bp->av_forw= p2;
		p1->av_forw= bp; }
	if(hmtab.d_active==0)
		hmstart();
	spl0(); }


hmstart(){
	register struct buf *bp;

	if((bp=hmtab.d_actf)==0)
		return;
	hmtab.d_active++;
	HMADDR->hmcs2= bp->b_dev.d_minor>>3;
	HMADDR->hmdc= bp->cylin;
	rhstart(bp, &HMADDR->hmda, bp->trksec, &HMADDR->hmbae); }

hmintr(){
	register struct buf *bp;
	register int i;

	if(hmtab.d_active==0)
		return;
	bp= hmtab.d_actf;
	hmtab.d_active= 0;
	if(HMADDR->hmcs1&SC){
		if(HMADDR->hmer1&UNS) panicking++;
		deverror(bp, HMADDR->hmcs2, HMADDR->hmcs1);
		printf("hm err: ds=0%o er1=0%o er2=0%o cs3=0%o da=0%o dc=0%o\n",
			HMADDR->hmds,HMADDR->hmer1,HMADDR->hmer2,HMADDR->hmcs3,
			HMADDR->hmda,HMADDR->hmdc);
		if(HMADDR->hmer1&UNS)
			panic("hm unsafe");
		HMADDR->hmcs2= CLR;
		HMADDR->hmcs1= RCLR|GO;
		HMADDR->hmcs1= PRESET|GO;
		if(++hmtab.d_errcnt<=10){
			hmstart();
			return; }
		bp->b_flags=| B_ERROR; }
	hmtab.d_errcnt= 0;
	hmtab.d_actf= bp->av_forw;
	bp->b_resid= HMADDR->hmwc;
	iodone(bp);
	hmstart(); }


hmread(dev){
	if(hmphys(dev))
		physio(hmstrategy, &rhmbuf, dev, B_READ, B_BLOCK); }

hmwrite(dev){
	if(hmphys(dev))
		physio(hmstrategy, &rhmbuf, dev, B_WRITE, B_BLOCK); }

hmphys(dev){
	register c;

	c= lshift(u.u_offset, -9);
	c=+ ldiv(u.u_count+511, 512);
	if(c>hm_sizes[dev.d_minor&07].nblocks){
		u.u_error= ENXIO;
		return 0; }
	return 1; }
