# include "graph.h"
# include <stdplt.h>
axis(strp,args)
char *strp;
int args;
  {
	argp = &args;
	_saxis(strp,1);
  }
saxis(strp,args)
char *strp;
int args;
  {
	argp = &args;
	_saxis(strp,0);
  }
_saxis(strp,doscale)
char *strp;
int doscale;
  {
	struct axtype *p;
	char astring[100];
	float penx,peny;
	extern xyflg;

	zinit("%$ %A",strp,astring);
	yyparse();
	if(xyflg == X)
		Do_min(_xdata,X);
	else
		Do_min(_ydata,Y);
	Do_nice(&_ax[xyflg],xyflg,doscale);
	if(!doscale)
		Do_scale(xyflg);
	p = &_ax[xyflg];
	if(xyflg == X)
	  {
		p->x = min[X];
		where(&penx,&peny,".");
		p->y = peny;
	  }
	else
	  {
		p->y = min[Y];
		where(&penx,&peny,".");
		p->x = penx;
	  }
	_axis(p);
	move(penx,peny);
  }
