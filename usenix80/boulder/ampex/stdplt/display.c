# include "graph.h"
# include <stdplt.h>
_display(xp,yp,dashflg)
struct _data *xp,*yp;
int dashflg;
  {
	int i;
	float xmin,xmax,ymin,ymax;
	float dummy;
	double xdata,ydata,xdisp,ydisp;
	double temp;
	double LOG();
	int flag;
	union ptr *x,*y;
	int n,xflag,yflag,lard;
	char symb;

	x = xp->dptr;
	y = yp->dptr;
	xflag = (xp->flgs&DATATYPE) | (_ax[X].flags&LOGAXIS);
	yflag = (yp->flgs&DATATYPE) | (_ax[Y].flags&LOGAXIS);
	n = 0;
	if(xp->npts > 0)
		n = xp->npts;
	else
		n = yp->npts;
	if(n <= 0)
		return;
	if(xp->symble)
		symb = xp->symble;
	else
		symb = yp->symble;
	if(xp->blubber > yp->blubber)
		lard = xp->blubber;
	else
		lard = yp->blubber;
	if(xp->ddash)
		dashflg = xp->ddash;
	else if(yp->ddash)
		dashflg = yp->ddash;
	dash(dashflg);
	if((min[X] < max[X]) || (min[Y] < max[Y]))
	  {
		if(min[X] < max[X])
		  {
			xmin = min[X];
			xmax = max[X];
		  }
		else
		  {
			xmin = -INFINITY;
			xmax = INFINITY;
		  }
		if(min[Y] < max[Y])
		  {
			ymin = min[Y];
			ymax = max[Y];
		  }
		else
		  {
			ymin = -INFINITY;
			ymax = INFINITY;
		  }
		window(xmin,xmax,ymin,ymax);
	  }
	if(xflag == NODATA)
	  {
		xdisp = (max[X] - min[X])/n;
		xdata = min[X] - xdisp;
	  }
	if(yflag == NODATA)
	  {
		ydisp = (max[Y] - min[Y])/n;
		ydata = min[Y] - ydisp;
	  }
	fat(lard);
	for(i = 0;i < n;i++)
	  {
		switch(xflag&DATATYPE)
		  {
			case NODATA:
				xdata =+ xdisp;
				break;
			case DOUBLE:
				xdata = *x.dp++;
				break;
			case FLOAT:
				xdata = *x.fp++;
				break;
			case INTEGER:
				xdata = *x.ip++;
				break;
			case LONG:
				xdata = *x.lp++;
				break;
			case SHORT:
				xdata = *x.hp++;
				break;
			case UNSIGNED:
				xdata = *x.up++;
				break;
		  }
		switch(yflag&DATATYPE)
		  {
			case NODATA:
				ydata =+ ydisp;
				break;
			case DOUBLE:
				ydata = *y.dp++;
				break;
			case FLOAT:
				ydata = *y.fp++;
				break;
			case INTEGER:
				ydata = *y.ip++;
				break;
			case LONG:
				ydata = *y.lp++;
				break;
			case SHORT:
				ydata = *y.hp++;
				break;
			case UNSIGNED:
				ydata = *y.up++;
				break;
		  }
		if(xflag&LOGAXIS)
			xdata = LOG(xdata);
		if(yflag&LOGAXIS)
			ydata = LOG(ydata);
		plot(xdata,ydata,i);
		if(symb)
		  {
			cmove(-.25,-.25);
			fat(2);
			textf("%c",symb);
			fat(lard);
			cmove(.25,.25);
		  }
	  }
	fat(0);
	window(-INFINITY,INFINITY,-INFINITY,INFINITY);
	dash(0);
  }
