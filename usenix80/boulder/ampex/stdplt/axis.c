# include "graph.h"
# include <stdplt.h>
_axis(p)
struct axtype *p;
  {
	double disp;
	double x,y;
	float xtic,ytic;
	double max;
	double total;
	int i;
	double fabs();
	double xdist();
	double ydist();
	double pow();
	double floor();
	int ticflg;
	int labflg;

	ticflg = p->flags&TICS;
	labflg = p->flags&LABS;
	cwidth(.009,"/");
	fat(2);
	if(p->type == X)
	  {
		xtic = 0.;
		ytic = ydist(.01,"/");
		if(ticflg == TIC)
			ytic =* -1;
		if(ticflg == TIC2)
			ytic =* -2;
		vector(p->x,p->y,p->x + p->len,p->y);
		fat(1);
		move(p->x + (p->len / 2),p->y);
		if(p->flags&NAME)
			cmove(0.,3.);
		else
			cmove(0.,-3.);
		if(p->lab)
			_adjtextf(0,'c',"%s",p->lab);
		total = p->x;
		max = p->x + p->len;
	  }
	else
	  {
		xtic = xdist(.01,"/");
		if(ticflg == TIC)
			xtic =* -1;
		if(ticflg == TIC2)
			xtic =* -2;
		ytic = 0.;
		vector(p->x,p->y,p->x,p->y + p->len);
		fat(1);
		move(p->x,p->y + (p->len / 2));
		if(p->flags&NAME)
			cmove(10.,0.);
		else
			cmove(-10.,0.);
		if(p->lab)
			_adjtextf(0,'v',"%s",p->lab);
		total = p->y;
		max = p->y + p->len;
	  }
	fat(0);
	disp = p->ticinc;
	x = p->x;
	y = p->y;
	for(i = 0;total < max + disp/100;i++)
	  {
		if(p->mtics == 9) /* Log axis */
			disp = p->ticinc * lincon[(i%p->mtics)];
		move(x,y);
		total =+ disp;
		if(i%p->mtics)
		  {
			if(!(ticflg == TIC3))
			  {
				if(ticflg == TIC2)
					rmove(-xtic/4.,-ytic/4.);
				rdraw(xtic/2.,ytic/2.);
			  }
			if(p->type == X)
			  {
				x =+ disp;
			  }
			else
			  {
				y =+ disp;
			  }
		  }
		else
		  {
			if(!(ticflg == TIC3))
			  {
				if(ticflg == TIC2)
					rmove(-xtic/2.,-ytic/2.);
				rdraw(xtic,ytic);
			  }
			if(p->type == X)
			  {
				move(x,y);
				/* Make zero 0, not .00000000014... */
				if(fabs(x) < disp/2)
					x = 0.;
				x =+ disp;
				if(labflg == LAB2)
					continue;
				if(labflg == LAB1)
					cmove(0.,1.5);
				else
					cmove(0.,-1.5);
				if(p->flags & LOGAXIS)
					_adjtextf(1,'c',"  %d",(int)floor(.5 + x - disp));
				else
					_adjtextf(0,'c',"%g",x - disp);
				/* x allready incremented by disp */
			  }
			else
			  {
				move(x,y);
				/* Make zero 0, not .00000000014... */
				if(fabs(y) < disp/2)
					y = 0.;
				y =+ disp;
				if(labflg == LAB2)
					continue;
				if(labflg == LAB1)
				  {
					cmove(1.4,.2);
					if(p->flags & LOGAXIS)
						_adjtextf(1,'l',"  %d",(int)floor(.5 + y - disp));
					else
						_adjtextf(0,'l',"%g",y - disp);
				  }
				else
				  {
					cmove(-1.,.2);
					if(p->flags & LOGAXIS)
						_adjtextf(1,'r',"  %d",(int)floor(.5 + y - disp));
					else
						_adjtextf(0,'r',"%g",y - disp);
				  }
				/* y allready incremented by disp */
			  }
		  }
	  }
  }
