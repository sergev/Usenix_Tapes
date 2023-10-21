# include "graph.h"
# include <stdplt.h>
dispts(strp,args)
char *strp;
int args;
  {
	argp = &args;
	_dispts(strp,1);
  }
_dispts(strp,doscale)
char *strp;
int doscale;
  {
	char astring[100];
	extern xyflg;	/* is this needed? */

	zinit("%$ %D",strp,astring);
	yyparse();
	pair();
	Do_disp();
  }
