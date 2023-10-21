#include "stdplt.h"

_draw(x, y)
  double x, y;
/*
 * Draw pen to (x,y) in current coordinate frame.
 */
  {
	register FRAME *dotp;
	register FILE *rstdplt;
	static char cmd[3] = { _CMD|'d', _CMD|'m', 0 };
	char *cmdp;		/* ptr to current draw/move command */
	float xnew, ynew;	/* future dotp->_x, _y */
	float xdraw, ydraw;	/* point to draw to in "." coords */
	float xcatnew, ycatnew;	  /* future dotp->_xcat, _ycat */
	float xcatdraw, ycatdraw; /* point to draw to in "" coords */

	dotp = _dotp;
	rstdplt = stdplt;
	cmdp = cmd;

	if (dotp->_fat != _fat)
	  {	putc(_CMD|'f', rstdplt);
		putw(_fat = dotp->_fat, rstdplt);
	  }
	xdraw = xnew = x;
	ydraw = ynew = y;
	  {	register float *ctp;

		ctp = dotp->_tcat;
		xcatdraw = xcatnew = _T(ctp, x, y) + dotp->_xocat;
		ycatdraw = ycatnew = _T(ctp, x, y) + dotp->_yocat;
	  }
	if (dotp->_fflags & _DOWIND) 	/* do current frame window */
	  {	register int dotclip;

		dotclip = _clip(&dotp->_x, &dotp->_y, &xdraw, &ydraw, dotp->_wind);
		if (dotclip < 0) goto nodraw;
		if (dotclip & 1)
		  {	register float *ctp;

			ctp = dotp->_tcat;
			dotp->_xcat = _T(ctp, dotp->_x,dotp->_y) + dotp->_xocat;
			dotp->_ycat = _T(ctp, dotp->_x,dotp->_y) + dotp->_yocat;
		  }
		if (dotclip & 2)
		  {	register float *ctp;

			ctp = dotp->_tcat;
			xcatdraw = _T(ctp, xdraw, ydraw) + dotp->_xocat;
			ycatdraw = _T(ctp, xdraw, ydraw) + dotp->_yocat;
		  }
	  }
	if (_clip(&dotp->_xcat, &dotp->_ycat, &xcatdraw, &ycatdraw, _rwind) >= 0)
	  {	_domove();
		if(dotp->_fflags&_DODASH)	/* do dash line */
		  {	register float *protop;	/* ptr to current proto segment */
			float dx, dy, dr;	/* how much to go */
			float dphase;		/* dist along prototype */
			float gap;	/* distance to go in current segment */
			extern double fmod(), sqrt();
	
			protop = dotp->_dproto;
			dx = xcatdraw - dotp->_xcat;
			dy = ycatdraw - dotp->_ycat;
			dr = sqrt(dx*dx + dy*dy);
			dotp->_dphase = fmod((dphase = dotp->_dphase) + dr, protop[3]);
			if (dr < PELSIZ)
				return;
			dx /= dr;	/* normalize by length */
			dy /= dr;
			while (dphase > *protop)    /* find starting segment */
			  {	protop++;
				if (*++cmdp == 0) cmdp = cmd;
			  }
			while (dr > (gap = *protop - dphase)) /* do complete segments */
			  {	dotp->_xcat += gap*dx;	/* go to end of this segment */
				dotp->_ycat += gap*dy;
				putc(*cmdp, rstdplt);
				putw((unsigned)((long) dotp->_xcat), rstdplt);
				putw((unsigned)((long) dotp->_ycat), rstdplt);
				dr -= gap;	/* account for dist covered */
				dphase = *protop++;	/* do next segment */
				if (*++cmdp == 0) cmdp = cmd;
				if (protop > &dotp->_dproto[3])	/* back to seg 0 */
				  {	protop = dotp->_dproto;
					dphase = 0.;
				  }
			  }
		  }
		putc(*cmdp, rstdplt);
		putw(_x = (long) xcatdraw, rstdplt);
		putw(_y = (long) ycatdraw, rstdplt);
	  }
nodraw:	dotp->_x = xnew;
	dotp->_y = ynew;
	dotp->_xcat = xcatnew;
	dotp->_ycat = ycatnew;
  }
