#
/*
 * Low segment of Versatec Matrix Printer/Plotter driver
 *
 * see mp.c for remainder.
 */
#include "../h/param.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/mp.h"

struct mp mp;


mpstart()
/*
 * If possible, start output.  Commands normally come from mp.m_cmdq,
 * but if an error condition has just cleared, the last one is retried.
 */
  {
	register int *p, *ampcs;
	register struct mpbuf *bp;

	ampcs = mp.m_acs;
	p = &mp.m_flags;
	while ((*ampcs&READY) && (mp.m_cmd == 0 || (*p&RETRY)))
	  {	*p =& ~RETRY;
		if (mp.m_cmd == 0 && (mp.m_cmd = getc(&mp.m_cmdq)) <= 0)
		  {	mp.m_cmd = 0;
			return;
		  }
		switch(mp.m_cmd)
		  {
		  case 'l':		/* terminate line */
			*ampcs =| LINE;
			return;

		  case 'B':		/* blank text line */
			MPADDR->mptdr = ' ';
			return;

		  case 'b':		/* blank raster */
			MPADDR->mprdr = 0;
			return;

		  case 'f':		/* form feed */
			*ampcs =| FF;
			return;

		  case 'c':		/* cut paper */
			break;

		  case 'h':		/* home past wash */
			*ampcs =| EOT;
			return;

		  case 'D':		/* start NPR for text or raster data */
			bp = mp.m_actf;
			p = &bp->b_un.b_addr[bp->m_used];
			MPADDR->mpma = &p->r_data[0];
			MPADDR->mpxmem = 0;
			if (ampcs == &MPADDR->mprcs)
				MPADDR->mprbc = p->r_len*2;
			else	MPADDR->mptbc = p->r_len*2;
			*ampcs =| DTCIEN;
			return;

		  case 'r':		/* start raster mode */
			ampcs = mp.m_acs = &MPADDR->mprcs;
			*ampcs = *ampcs;
			break;

		  case 'T':		/* simultaneous text/raster */
			MPADDR->mprcs =| SPP;
			ampcs = mp.m_acs = &MPADDR->mptcs;
			*ampcs =| SPP;
			break;
		  case 't':		/* turn on text mode */
			MPADDR->mprcs =& ~SPP;
			ampcs = mp.m_acs = &MPADDR->mptcs;
			*ampcs =& ~SPP;
			break;
		  }
		mp.m_cmd = 0;
	  }
  }


mpintr()
/*
 * Service interrupt.  Errors are noted in mp.m_flags, and mpstart()
 * is called.  Top half is poked if command queue is running low--
 * note that MPLWAT is set so that wakeups don't occur if there is
 * any chance of all the buffers being full.
 */
  {
	register int *p;
	register struct mpbuf *bp;
	register int *ampcs;

	ampcs = mp.m_acs;
	p = &mp.m_flags;
	if (mp.m_cmd != 0)
	  {	if ((*ampcs&ERROR) && (*ampcs&READY) == 0)
		  {	*p =| ERR|RETRY;
			if (mp.m_cmd == 'D')	/* abort DMA */
			  {	*ampcs =& ~DTCIEN;
				MPADDR->mprbc = 0;
				MPADDR->mptbc = 0;
				*ampcs =& ~ERROR;
			  }
			printf("err on Versatec\ncmd:%s er%o\n", &mp.m_cmd, *ampcs);
			wakeup(&mp.m_cmdq);	/* for close */
			return;
		  }
		*ampcs =& ~ERROR;	/* was error during DMA */
		*p =& ~ERR;
		if (mp.m_cmd == 'D' && (*p&RETRY) == 0)
		  {	*ampcs =& ~DTCIEN;
			if (*ampcs&SPP)		/* switch to raster mode */
				ampcs = mp.m_acs = &MPADDR->mprcs;
			bp = mp.m_actf;
			p = &bp->m_used;
			*p =+ ROFFSET + bp->b_un.b_addr[*p].r_len*2;
			if (*p >= 512 - bp->m_wfree*2)
			  {	mp.m_actf = bp->m_next;
				brelse(bp);
				mp.m_nbufs--;
			  }
		  }
		if ((mp.m_flags&RETRY) == 0)
			mp.m_cmd = 0;
	  }
	mpstart();
	if (mp.m_cmdq.c_cc <= MPLWAT)
		wakeup(&mp.m_cmdq);
  }
