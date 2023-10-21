#
/*
 * Versatec driver (dma only).
 *
 * Toronto calling conventions.  Modified at Purdue: physio used instead
 * of grabcore stuff;  write call added.
 */

#include "../param.h"
#include "../user.h"
#include "../conf.h"
#include "../buf.h"

#define VCDMA   0177500
#define	VCADDR	(VCDMA+010)
#define	INTENB	0100
#define	READY	0200
#define ERROR   0100000
#define	ENDLINE	040
#define CLEARBUF 04
#define	RESET	02
#define	CLOSED	0
#define	PRINT	1
#define	PLOT	2
#define PLOTOPER 3
#define	WRITE	4
#define	WAIT	5

#define VCPRIO  7

struct {
	int state;
	int error;
	int active;
	int base;
	int count;
	struct buf bufh;
} vc;

struct {
	int plcsr;
	int pldbr;
	int prcsr;
	int prdbr;
};

struct {
	int plbcr;
	int pbae;
	int prbcr;
	int pbaddr;
};

int *vccsr { &(VCADDR->prcsr) };

extern lbolt;

vcopen(dev,flag)
{
	if(vc.state != CLOSED)
	{
		u.u_error = ENXIO;
		return;
	}
	if(dev.d_minor) {
		vc.state = PRINT;
		vccsr = &(VCADDR->prcsr);
	} else {
		vc.state = PLOT;
		vccsr = &(VCADDR->plcsr);
	}
	if(*vccsr<0)
	{
		u.u_error = ENXIO;
		return;
	}
	vc.error = 0;
	vc.active = 0;
	*vccsr = RESET;
	*vccsr = INTENB;
}

vcclose(dev,flag)
{
	vcwait();
	*vccsr = 0;
	vc.active = 0;
	vc.state = CLOSED;
}

vcwrite(dev)
{
	vcwait();
	vc.base = u.u_base;
	vc.count = u.u_count;
	vcout();
}

vcsgtty(dev,v)
	int *v;
{
	register int *vv;

	if(vv=v)
	{
		*vv++ = vc.state;
		*vv++ = 112*64;         /* old toronto fixed buffer size */
		*vv = vc.error;
	}
	else
	{
		vv = u.u_arg;
		while(*vccsr<0)
		{
			vc.error++;
			*vccsr = INTENB;
			sleep(&lbolt,VCPRIO);
		}
		vcwait();
		switch(*vv++)
		{
		case PRINT:
			if(vc.state != PRINT)
			{
				vccsr = &(VCADDR->prcsr);
				vc.state = PRINT;
				vcoper(CLEARBUF);
			}
			break;

		case PLOT:
			if(vc.state != PLOT)
			{
				vccsr = &(VCADDR->plcsr);
				vc.state = PLOT;
				vcoper(CLEARBUF);
			}
			break;

		case PLOTOPER:
			if(vc.state!=PLOT)
			{
				u.u_error = EINVAL;
				return;
			}
			vcoper((*++vv)&076);	/* note switch(*vv++) above */
			break;

		case WRITE:
			vc.base = *vv++;
			vc.count = *vv;
			vcout();
			break;

		case WAIT:
			break;

		default:
			u.u_error = EINVAL;
		}
	}
}

vcwait()
{
	spl4();
	while((*vccsr & (READY|ERROR)) == 0)
		sleep(&vccsr,VCPRIO);
	spl0();
}


vcoper(bits)
{
	*vccsr = INTENB | bits;
}


vcint()
{
	wakeup(&vccsr);
	if(vc.active) {
		if(*vccsr<0) vc.bufh.b_flags =| B_ERROR;
		vc.active = 0;
		iodone(&vc.bufh);
	}
	if(*vccsr<0)
		*vccsr =& ~ERROR;
}


vcstrategy(bp)
register struct buf *bp;
{
	spl4();
	vc.active++;
	VCDMA->pbaddr = bp->b_addr;
	VCDMA->pbae = ((bp->b_xmem&03)<<4);
	*(vc.state == PRINT ? &(VCDMA->prbcr) : &(VCDMA->plbcr)) = vc.count;
	bp->b_resid = 0;
	spl0();
}


vcout()
{
	u.u_base = vc.base;
	u.u_count = (vc.count+1) & ~1;
	physio(vcstrategy, &vc.bufh, NODEV, B_WRITE);
}


