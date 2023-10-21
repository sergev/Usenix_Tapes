# include "graph.h"
# include <stdplt.h>
Do_nice(p,xory,Sscale)
struct axtype *p;
int xory;
int Sscale;
  {
	double ticinc;
	double _nicetics();
	double ydist();
	double xdist();
	float length;
	int logflg;
	double LOG();

	logflg = 0;
	if(p->flags & LOGAXIS)
	  {
		min[xory] = LOG(min[xory]);
		max[xory] = LOG(max[xory]);
		logflg++;
	  }
	if(xory == X)
	  {
		do
		  {
			if(Sscale)
				length = 1.;
			else
				length = (max[X] - min[X]);
			ticinc = 8. * xdist(.009,"/") * length;
			ticinc = _nicetics(&min[X],&max[X],ticinc,&p->mtics,logflg);
		  }while(ticinc <= 8. * xdist(.009,"/") * length);
	  }
	if(xory == Y)
	  {
		do
		  {
			if(Sscale)
				length = 1.;
			else
				length = (max[Y] - min[Y]);
			ticinc = 8. * ydist(.009,"/") * length;
			ticinc = _nicetics(&min[Y],&max[Y],ticinc,&p->mtics,logflg);
		  }while(ticinc <= 8. * ydist(.009,"/") * length);
	  }

	p->len = max[xory] - min[xory];
	if(p->mtics == 9)
	  {
		p->ticinc = ticinc;
	  }
	else
		p->ticinc = ticinc / p->mtics;
	p->type = xory;
  }
Do_scale(xory)
int xory;
  {
	float sx,sy,ox,oy;

	sx = sy = 1.;
	ox = oy = 0.;
	if(xory == X)
	  {
		sx = 1./(max[X] - min[X]);
		ox = -min[X];
	  }
	else
	  {
		sy = 1./(max[Y] - min[Y]);
		oy = -min[Y];
	  }
	scale(sx,sy);
	origin(ox,oy);
  }
