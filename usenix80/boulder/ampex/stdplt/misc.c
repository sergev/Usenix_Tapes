# include "graph.h"
# include <stdplt.h>
_adjtextf(log,adj, fmt, args)
int log;
  char adj;		/* adjustment code */
  char *fmt;
/*
 * do a textf vertically centered on the current y-position, and
 * either left, center, or right adjusted on the current x-position.
 */
  {	struct _iobuf sbuf;
	char str[STRLEN];
	float xold, yold, len;

	sbuf._flag = _IOWRT+_IOSTRG;
	sbuf._ptr = str;
	sbuf._cnt = STRLEN;
	sbuf._base = -1;
	sbuf._file = -1;

	_doprnt(fmt, &args, &sbuf);
	len = charw*(STRLEN - sbuf._cnt);
	putc('\0', &sbuf);
	xold = xpos;
	yold = ypos;

	switch(adj)
	  {
	  case 'l':
		rmove(0., -.5*charh); break;
	  case 'c':
		rmove(-.5*len, -.5*charh); break;
	  case 'r':
		rmove(-len, -.5*charh); break;
	case 'v':
		cmove(0.,.5*len/charw);
		_vertextf(str);
		move(xold, yold);
		return;
	  default:
		_err("adjtextf: bad adjustment %c\n", adj);
	  }
	if(log)
	  {
		cmove(0.,-.3);
		textf("10");
		cmove(0.,.3);
	  }
	textf("%s", str);
	move(xold, yold);
  }

_vertextf(p)
char *p;
  {
	while(*p)
		textf("%c\n",*p++);
  }


double
_nicetic(min,max,ticinc,mtic,logflg)
double *max,*min,ticinc;
int *mtic;
  {
	double newmin, newmax;
	int n;
	double val, temp;
	double log(), pow(), floor(), ceil(),xdist(),ydist();
	double  fabs();

/* Derermines tic spacing */
	n = floor((log(ticinc))/(LOG10));
	temp = ticinc/(pow(10.,(double)n));
	if(temp <=1.) val = 1.;
	else if (temp <=2.) val = 2.;
	else if (temp <=5.) val = 5.;
	else if (temp <10.) val = 10.;
	if(logflg && (n < 0))
	  {
		if(temp > 1.)
			val = 10;
		*mtic = 9;
	  }
	else
		*mtic = val;


/* Adjusts min and max to match tic inc */
	ticinc = val * pow(10.,(double)(n));
	newmin = floor((*min / ticinc) + EPSILON) * ticinc;
	newmax = ceil((*max / ticinc) -  EPSILON) * ticinc;
	*min = newmin; *max = newmax;
	return(ticinc);
}
_square(xl,xh,yl,yh)
float xl,xh,yl,yh;
  {
	origin(xl,yl);
	if(xh <= xl)
	  {
		printf("Warning: scale for graph");
		printf(" radicaly small in X direction\n");
		xh = xl + 1;
	  }
	if(yh <= yl)
	  {
		printf("Warning: scale for graph");
		printf(" radicaly small in Y direction\n");
		yh = yl + 1;
	  }
	scale(xh-xl,yh-yl);
  }
_error(str)
char *str;
  {
	printf("%s\n",str);
  }
_feror(str)
char *str;
  {
	printf("%s\n",str);
	exit();
  }
double
LOG(val)
double val;
  {
	return(log(val) / LOG10);
  }
