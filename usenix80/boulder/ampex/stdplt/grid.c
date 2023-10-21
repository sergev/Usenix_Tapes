# include "graph.h"
# include <stdplt.h>
_grid(x,y)
struct axtype *x,*y;
  {
	float aray[4];
	float disp;
	float xdisp,ydisp;
	int numtics,i;
	double xdist(),ydist();
	int gridflg;

	gridflg = (x->flags&GRID) | (y->flags&GRID);
	if(GRID3 == gridflg)
		return;
	aray[0] = aray[1] = aray[2] = aray[3] = .004;
	if(x->mtics == 9)
	  {
		disp = x->ticinc;
		numtics = x->len/x->ticinc + .05;
	  }
	else
	  {
		numtics = x->len/(x->ticinc * x->mtics) + .05;
		disp = x->ticinc * x->mtics;
	  }
	xdisp = x->x;
	vector(xdisp,y->y,xdisp,y->y + y->len);
	if((gridflg == GRID0) || (gridflg == GRID1))
	  {
		fat(2);
		if((x->x < 0) && (x->x + x->len > 0))
			vector(0,y->y,0,y->y + y->len);
		fat(0);
	  }
	if(gridflg == GRID1)
		dash(aray);
	for(i = 0;i < numtics;i++)
	  {
		if(gridflg != GRID2)
			vector(xdisp,y->y,xdisp,y->y + y->len);
		xdisp =+ disp;
	  }
	dash(0);
	vector(xdisp,y->y,xdisp,y->y + y->len);
	if(y->mtics == 9)
	  {
		numtics = y->len/y->ticinc + .05;
		disp = y->ticinc;
	  }
	else
	  {
		numtics = y->len/(y->ticinc * y->mtics) + .05;
		disp = y->ticinc * y->mtics;
	  }
	ydisp = y->y;
	vector(x->x,ydisp,x->x + x->len,ydisp);
	if((gridflg == GRID0) || (gridflg == GRID1))
	  {
		fat(2);
		if((y->y < 0) && (y->y + y->len > 0))
			vector(x->x,0,x->x + x->len,0);
		fat(0);
	  }
	if(gridflg == GRID1)
		dash(aray);
	for(i = 0;i < numtics;i++)
	  {
		if(gridflg != GRID2)
			vector(x->x,ydisp,x->x + x->len,ydisp);
		ydisp =+ disp;
	  }
	dash(0);
	vector(x->x,ydisp,x->x + x->len,ydisp);
  }
