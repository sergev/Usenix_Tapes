# include "graph.h"
# include <stdplt.h>
grid(strp,args)
char *strp;
int args;
  {
	argp = &args;
	_fgrid(strp);
  }
_fgrid(strp)
char *strp;
  {
	struct axtype *p;
	char astring[100];

	zinit("%$ %R",strp,astring);
	yyparse();
	Do_min(_xdata,X);
	Do_min(_ydata,Y);
	Do_nice(&_ax[X],X,1);
	Do_nice(&_ax[Y],Y,1);
	p = &_ax[X];
	p->x = min[X];
	p->y = min[Y];
	p++;
	p->x = min[X];
	p->y = min[Y];
	_grid(&_ax[X],&_ax[Y]);
  }
