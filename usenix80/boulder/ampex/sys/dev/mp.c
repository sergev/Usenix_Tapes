#
/*
 * High segment of Versatec Matrix Printer/Plotter driver
 *
 *	John Nickolls, July 1977
 *	Stanford University
 *
 * Edit History:
 *
 * 08/18/77	john	here we go
 * 01/25/78	 "	conversion from Gould to Versatec
 * 11/1/79	mitch gart	version 7 conversion
 *%%
 */
#include "../h/param.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/mp.h"

struct mp mp;

mpopen(dev, flag)
/*
 * Minor devices:
 *	0: mpf		fanfold plotter
 *	1: mlpf		fanfold printer
 *	2: mpr		roll plotter
 *	3: mlpr		roll printer
 */
  {
	register int *p;

	if (mp.m_flags&OPEN)
	  {	u.u_error = EBUSY;
		return;
	  }
	for (p = &mp; p < &mp + 1; )
		*p++ = 0;
	mp.m_acs = &MPADDR->mptcs;
	MPADDR->mptcs = RESET;
	mp.m_pri = MPOPRI;
	mp.m_flags = (minor(dev)&2)? (OPEN|TEXT|ROLL): (OPEN|TEXT);
	mpput('t');
	mpput('f');
	if (minor(dev)&1)	/* line printer */
	  {	mp.m_flags =| (PRINTER); /* or in IND if lpr doesn't indent */
		mpcanon(FORM);
	  }
	MPADDR->mptcs =| IENABLE;
  }


mpclose(dev, flag)
  {
	register struct mpbuf *bp, *bp1;

	mp.m_pri = -1;		/* must not bomb out of close */
	if ((mp.m_flags&ERR) == 0)
	  {	if (mp.m_flags&PRINTER)
		  {	mpcanon(FORM);
			if (mp.m_flags&ROLL)
			  {	mpcanon(0);
				mpcanon('\n');
			  }
		  }
		mpput('h');
		mpput(0);
		if (mpflush())
		  {	spl5();
			while (mp.m_cmdq.c_cc && (mp.m_flags&ERR) == 0)
			  {	mpstart();
				sleep(&mp.m_cmdq, mp.m_pri);
			  }
		  }
	  }
	spl5();
	for (bp1 = mp.m_actf; bp = bp1; )
	  {	bp1 = bp->m_next;
		brelse(bp);
	  }
	if (bp = mp.m_wbp)
		brelse(bp);
	while(getc(&mp.m_tmpq) >= 0) ;
	while(getc(&mp.m_cmdq) >= 0) ;
	while(getc(&mp.m_canq) >= 0) ;
	MPADDR->mptcs = RESET;
	mp.m_flags =& ~OPEN;
	spl0();
  }


mpwrite(dev)
  {
	register int offset, c;
	register struct mpbuf *bp;
	int count, extra, c1, *p;

	if (mp.m_flags&PRINTER)
	  {	while((c = cpass()) >= 0)
			mpcanon(c);
		return;
	  }
	if ((u.u_base|u.u_count)&1)
	  {	u.u_error = EFAULT;
		return;
	  }

	while (u.u_count)
	  {	if(mp.m_flags&TMID)	/* middle of text */
		  {	while ((c = cpass()) > 0)
				mpcanon(c1 = c);
			if (c == 0)
			  {	if (c1 != '\n')
					mpcanon('\n');
				mp.m_flags =& ~TMID;
				if (u.u_base&1)
					cpass();
			  }
		  }
		else if (mp.m_flags&RMID)	/* middle of raster */
		  {	if (mpmove())
			  {	mp.m_flags =& ~RMID;
				mpput('D');	/* NPR data transfer */
				mpput('l');
			  }
		  }
		else			/* new work */
		  {	count = cpass()&0377;
			offset = cpass()&0377;

			if (count == 0377)	/* escape code in offset */
			  {	switch(offset)
				  {
				  case 'f':	/* form feed */
				  case 'h':	/* home past wash */
				  case 'c':	/* cut paper */
					mpput(offset);
					break;

				  case 't':	/* text */
					if ((mp.m_flags&TEXT) == 0)
						mpput(offset);
					mp.m_flags =| (TEXT|TMID);
					break;

				  case 'T':	/* simultaneous text & raster */
					mpput(offset);
					mp.m_flags =| (TEXT|TMID);
					break;
				  }
			  }
			else			/* raster */
			  {	if (mp.m_flags&TEXT)
					mpput('r');
				mp.m_flags =& ~TEXT;
				if (count == 0)
				  {	mpput('b');
					mpput('l');
				  }
				else
				  {	if ((extra = offset+count-RSIZE) > 0)
					  {	if (offset >= RSIZE)
							offset = RSIZE;
						count = RSIZE - offset;
					  }
					else
						extra = 0;
					bp = mpalloc((ROFFSET)/2 +
						offset + count);
					bp->b_un.b_addr[bp->m_used].r_len =
						offset + count;
					bp->m_used =+ ROFFSET;
					p = &bp->b_un.b_addr[bp->m_used];
					bp->m_used =+ offset*2;
					while (offset--)
						*p++ = 0;
					bp->m_extra = extra*2;
					bp->m_resid = count*2;
					mp.m_flags =| RMID;
				  }
			  }
		  }
	  }
  }


mpcanon(c)
  int c;
/*
 * Canonical character interpretations.
 * backspace acts like it does on a CRT, FORM moves to top of form (TOF),
 * blank lines at top & bottom of page are compressed to one form feed.
 * Assumes plotter is already in text mode.
 */
  {
	register int i, *avcc;
	register struct mpbuf *bp;

	avcc = &mp.m_vcc;
	switch(c)
	  {
	  case ' ':
		(*avcc)++;
		break;

	  case '\b':
		if (*avcc > 0)
			(*avcc)--;
		break;

	  case '\t':
		*avcc = (*avcc + 8)& ~07;
		break;

	  case ESC:	/* nroff mode until next FORM */
		if (mp.m_flags&PRINTER)
		  {	mp.m_topeat = TOPEAT;
			mp.m_pagesize = PAGESIZE;
			mp.m_ejline = (mp.m_flags&ROLL)? PAGESIZE-TOPEAT: EJAUTO;
			mp.m_tofline = (mp.m_flags&ROLL)? TOFAUTO: 0;
		  }
		break;

	  case FORM:
	mpff:
		mp.m_ilc = -1;
		mp.m_topeat = 32767;
		mp.m_pagesize = 0;
		mp.m_ejline = EJLINE;
		mp.m_tofline = (mp.m_flags&ROLL)? TOFAUTO+TOFLINE: TOFLINE;
	  case '\n':
		if (mp.m_flags&PRINTER)
		  {	if (mp.m_ilc >= PAGESIZE)
				mp.m_ilc = 0;
			mp.m_ilc++;
			if (mp.m_acc == 0 && mp.m_alc == 0)
				goto mpcr;	/* convert blank lines at TOF to FF */
			mp.m_alc++;
			if (mp.m_alc >= mp.m_ejline)
				c = FORM;	/* page overflow */
		  }
		*avcc = mp.m_acc;	/* # of printed-on columns */
		mp.m_acc = 0;		/* line is now complete */
		if ((*avcc&01) != 0)	/* force even buffer length */
		  {	putc(' ', &mp.m_canq);
			(*avcc)++;
		  }
		if (*avcc)
		  {	bp = mpalloc(i = (ROFFSET + *avcc + 1)/2);
			bp->b_un.b_addr[bp->m_used].r_len = i - 1;
			bp->m_used =+ ROFFSET;
			while ((i = getc(&mp.m_canq)) >= 0)
			  {	if (i == '\b')
					bp->m_used--;
				else
					bp->b_un.b_addr[bp->m_used++] = i;
			  }
			mpput('D');
		  }
		else
			mpput('B');
		if (c != FORM || (mp.m_flags&PRINTER) == 0)
			mpput('l');
		else
		  {	if (mp.m_flags&ROLL)
			  {	for (i = PAGELEN-mp.m_tofline-mp.m_alc+1; i > 0;  i--)
				  {	mpput('B');
					mpput('l');
				  }
			  }
			else
				mpput('f');
			mp.m_alc = 0;
			if (mp.m_pagesize == 0)
				goto mpff;
		  }
	mpcr:		/* no break */
	  case '\r':
		*avcc = (mp.m_flags&IND)? 8: 0;
		break;

	  default:
		if (mp.m_alc == 0 && mp.m_acc == 0 && mp.m_flags&PRINTER)
		  {	if (mp.m_flags&ROLL)	/* draw dotted line */
			  {	bp = mpalloc(ROFFSET/2 + RSIZE);
				bp->b_un.b_addr[bp->m_used].r_len = RSIZE;
				bp->m_used =+ ROFFSET;
				for (i = 0; i < RSIZE; i++)
				  {	bp->b_un.b_addr[bp->m_used++] = 0377;
					bp->b_un.b_addr[bp->m_used++] = 0;
				  }
				mpput('r');
				mpput('D');
				mpput('l');
				mpput('t');
			  }
			for (i = mp.m_tofline; i--; )
			  {	mpput('B');
				mpput('l');
			  }
			for (i = mp.m_ilc; i > mp.m_topeat; i--)
			  {	mpput('B');
				mpput('l');
				mp.m_alc++;
			  }
		  }
		if (*avcc < MAXCOL)
		  {	while ( mp.m_acc > *avcc)
			  {	putc('\b', &mp.m_canq);
				mp.m_acc--;
			  }
			while (mp.m_acc < *avcc)
			  {	putc(' ', &mp.m_canq);
				mp.m_acc++;
			  }
			putc(c, &mp.m_canq);
			mp.m_acc++;
		  }
		(*avcc)++;
		break;
	  }
  }


mpput(c)
/*
 * Put a command byte on the temporary command queue.
 * The data associated with a 'D' command must be complete!
 * The temp queue may be flushed to the active queue.
 */
  {
	if (mp.m_tmpq.c_cc >= MPHWAT && !mpflush())
	  {	if (mp.m_pri == MPOPRI)
			printf("mpput\n");
		else
			return;	/* bad buffer ok during close */
	  }
	spl5();
	while (mp.m_cmdq.c_cc >= MPHWAT &&
	    (mp.m_pri == MPOPRI || (mp.m_flags&ERR) == 0))
	  {	mpstart();	/* this is mostly for non-buffer commands */
		sleep(&mp.m_cmdq, mp.m_pri);
	  }
	spl0();
	putc(c, &mp.m_tmpq);
  }


struct mpbuf *mpalloc(nwords)
/*
 * Allocate nwords contiguous words in a buffer.
 * mp.m_wbp and the return value will point to a mpbuf structure,
 * with the allocated region starting at bp->b_un.b_addr[bp->m_used].
 * The buffer will be considered busy until bp->m_used == 512 - bp->m_wfree*2.
 */
  {
	register struct mpbuf *bp;

	if ((bp = mp.m_wbp) && bp->m_wfree >= nwords)
	  {	bp->m_wfree =- nwords;
		return(bp);
	  }
	if (!mpflush())
		printf("mpalloc\n");
	spl5();
	while(mp.m_nbufs >= NBUFS &&
	    (mp.m_pri == MPOPRI || (mp.m_flags&ERR) == 0))
	  {	mpstart();
		sleep(&mp.m_cmdq, mp.m_pri);
	  }
	spl0();
	bp = mp.m_wbp = geteblk();
	bp->m_next = 0;
	bp->m_used = 0;
	bp->m_wfree = 256 - nwords;
	bp->m_extra = bp->m_resid = 0;
	mp.m_nbufs++;
	return(bp);
  }


int mpflush()
/*
 * If possible, flush temporary command queue to active command queue
 * and link working buffer on active buffer queue.
 * Return true if a flush was accomplished.
 */
  {
	register int c;
	register struct mpbuf *bp;

	if (bp = mp.m_wbp)
	  {	if (bp->m_used != 512 - bp->m_wfree*2)
			return(0);
		bp->m_used = 0;
		bp->m_next = mp.m_wbp = 0;
		spl5();
		if (mp.m_actf == 0)
			mp.m_actf = bp;
		else
			mp.m_actl->m_next = bp;
		mp.m_actl = bp;
		spl0();
	  }
	while ((c = getc(&mp.m_tmpq)) >= 0)
		putc(c, &mp.m_cmdq);
	return(1);
  }


int mpmove()
/*
 * Move mp.m_wbp->m_resid bytes from u.u_base to working buffer.
 * When done, throw away bp->m_extra bytes.
 * Return true if move count is exhausted and extra disposed of.
 */
  {
	register int count, extra;
	register struct mpbuf *bp;

	bp = mp.m_wbp;
	count = bp->m_resid;	/* bytes left to iomove */
	if ((bp->m_resid =- u.u_count) < 0)
		bp->m_resid = 0;
	if (count = min(u.u_count, count))
	  {	iomove(&(bp->b_un.b_addr[bp->m_used]), count, B_WRITE);
		swab(bp->b_un.b_addr + bp->m_used, count); /* bassackward Versatec */
	  }
	bp->m_used =+ count;
	if (bp->m_resid == 0 && (extra = min(bp->m_extra, u.u_count)))
	  {	u.u_base =+ extra;
		u.u_offset =+ extra;
		u.u_count =- extra;
		bp->m_extra =- extra;
	  }
	return(bp->m_resid == 0 && bp->m_extra == 0);
  }


swab(ap, c)
  char *ap;
  int c;
/*
 * Swap 'c' bytes in place, starting at 'ap'.
 */
  {	register char *p, *q, tmp;

	c =>> 1;	/* word count */
	for (p = q = ap; c--; )
	  {	tmp = *p++;
		*q++ = *p++;
		*q++ = tmp;
	  }
  }
