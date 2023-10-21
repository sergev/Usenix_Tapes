/*
 * RX01 Floppy Disk Driver
 *
 * Jeffrey Kodosky
 * National Instruments
 *
 * rx0/1 are mapped using DEC RT interleave and skew (2:1 sector interleave
 * and 6 sector skew between tracks), and track 0 is not used, so there are
 * 494 blocks numbered 0-493.
 *
 * rx2/3 are unmapped (a block is 4 consecutive sectors, with sector 1 of
 * track n+1 following sector 26 of track n), and all 77 tracks are used,
 * so there are 500.5 blocks numbered 0-499.
 */

#include "../hd/param.h"
#include "../hd/proc.h"
#include "../hd/buf.h"
#include "../hd/conf.h"
#include "../hd/user.h"

#define RXADDR  0177170
#define RXPRI   (-20)
#define RXPATIENCE 1000

#define ERROR   0100000
#define INIT    040000
#define TR      0200
#define IE      0100
#define DONE    040

#define GO      1
#define FILL    0
#define EMPTY   2
#define WTSECT  4
#define RDSECT  6

struct  devtab  rxtab;
struct {
	int     rxcs;
	int     rxdb;
	};
struct  buf     rrxbuf;

int     rx_size[] { 494, 494, 500, 500 };
int     rxok;

rxopen(dev,flag){
	register int d;

	if(!rxok)
		if(tkword(RXADDR)){ u.u_error= ENXIO; return; }
		else rxok++;
	d= dev.d_minor;
	if(d<0 || d>3) u.u_error= ENXIO;
	}

rxclose(dev){ }

rxstrategy(abp){
	register struct buf *bp;
	register int d;

	bp= abp;
	d= bp->b_dev.d_minor;
	if(bp->b_blkno<0 || bp->b_blkno + (-bp->b_wcount>>8) >rx_size[d]){
		bp->b_flags=| B_ERROR;
		iodone(bp);
		return; }
	bp->av_forw= 0;
	spl5();
	if(rxtab.d_actf==0) rxtab.d_actf= bp;
	else rxtab.d_actl->av_forw= bp;
	rxtab.d_actl= bp;
	if(rxtab.d_active==0) rxstart();
	spl0(); }

rxstart(){
	register struct buf *bp;
	register int lsn, ofst;
	int i;

	rxtab.d_active++;
	while(bp= rxtab.d_actf){
		for(i=(-bp->b_wcount>>6), ofst=0, lsn=bp->b_blkno*4;  i>0 ; i--, lsn++, ofst=+ 128)
			for(rxtab.d_errcnt=0; rxsect(bp,lsn,ofst)<0; )
				if(++rxtab.d_errcnt>10) break;
		rxtab.d_actf= bp->av_forw;
		if(rxtab.d_errcnt){
			rxtab.d_errcnt= 0;
			bp->b_flags=| B_ERROR; }
		iodone(bp); }
	rxtab.d_active= 0; }

rxsect(abp,lsn,ofst){
	register struct buf *bp;
	register int i;
	int track, sector, rw;

	bp= abp;
	rw= bp->b_flags&B_READ;
	track= lsn/26;
	sector= lsn%26;
	if(bp->b_dev.d_minor<2){
		sector= ((sector>=13? 1:0) + 2*sector + 6*track) %26;
		track++; }
	sector++;
	if(rw==B_WRITE){
		for(i=0, RXADDR->rxcs= IE|FILL|GO; i<128; i++)
			if(rxwait()>=0) RXADDR->rxdb= bp->b_addr[ofst+i];
			else return (-1);
		if(rxdone()<0) return (-1); }
	if(rxmove(track,sector,rw,bp->b_dev.d_minor) < 0) return (-1);
	if(rw==B_READ){
		for(i=0, RXADDR->rxcs= IE|EMPTY|GO; i<128; i++)
			if(rxwait()>=0) bp->b_addr[ofst+i]= RXADDR->rxdb;
			else return (-1);
		if(rxdone()<0) return (-1); }
	return 0; }

rxmove(track,sector,rw,d){
	RXADDR->rxcs= IE | (rw==B_WRITE? WTSECT:RDSECT) | (d&1)<<4 | GO;
	if(rxwait()) return (-1);
	RXADDR->rxdb= sector;
	if(rxwait()) return (-1);
	RXADDR->rxdb= track;
	return rxdone(); }

rxwait(){
	extern int lbolt;
	register int i;
	for(i=0; (RXADDR->rxcs&TR)==0; i++)
		if(RXADDR->rxcs&ERROR) return rxinit();
		else if(i>RXPATIENCE) sleep(&lbolt,RXPRI);
	return 0; }

rxdone(){
	register int i;
	for(i=0; (RXADDR->rxcs&DONE)==0; i++)
		if(i>RXPATIENCE) sleep(&rxtab,RXPRI);
	if(RXADDR->rxcs&ERROR) return rxinit();
	RXADDR->rxcs= 0;
	return 0; }

rxintr(){
	RXADDR->rxcs= 0;
	wakeup(&rxtab); }

rxinit(){
	deverror(rxtab.d_actf, RXADDR->rxcs, RXADDR->rxdb);
	RXADDR->rxcs= IE|INIT;
	return (-1); }

rxread(dev){
	physio(&rxstrategy, &rrxbuf, dev, B_READ, B_BLOCK>>2);
	}

rxwrite(dev){
	physio(&rxstrategy, &rrxbuf, dev, B_WRITE, B_BLOCK>>2);
	}
