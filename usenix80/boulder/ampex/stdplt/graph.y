%{
# include "graph.h"
# include "stdio.h"
struct axtype *_axptr;
struct _data *_datptr;
int starflg;
int yyline;
int xyflg;
int oxyflg;
int i,j;
char *charp,*chrp;
char *sp;
int frag;
double atof();
%}
%%
root:	init '$' 'G' sgraph |
	init '$' 'A' ssaxis |
	init '$' 'D' sdisplay |
	init '$' 'R' grid;

init: /*empty*/ { i = j = 0;};
sgraph:	sgraph graph |
	graph;

graph:	xyd graf;

graf:	graf des |
	/* empty */;

des:	ax |
	shift |
	disp;

xyd:	'X'  = {	
		nary[X]++;
		xyflg = X;
		_datptr = _xdata[i++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		_datptr->flgs =| MINMAX;
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		} |
	'x'  = {	
		nary[X]++;
		xyflg = X;
		_datptr = _xdata[i++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		} |
	'Y' = {
		nary[Y]++;
		xyflg = Y;
		_datptr = _ydata[j++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		_datptr->flgs =| MINMAX;
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		} |
	'y' = {
		nary[Y]++;
		xyflg = Y;
		_datptr = _ydata[j++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		};

ssaxis: ssaxis saxis |
	saxis;

saxis:	xyd axis;

axis:	axis ax {
		if(first)
		  {
			if(xyflg != oxyflg)
			  {
				yyerror("Saxis: Non matching xy discreptor");
				exit();
			  }
		  }
		first = 1;
		oxyflg = xyflg;
		}
	| /* empty */ = {
		if(first)
		  {
			if(xyflg != oxyflg)
			  {
				yyerror("Saxis: Non matching xy discreptor");
				exit();
			  }
		  }
		first = 1;
		oxyflg = xyflg;
	  };

ax:	
	datatype |
	minmax |
	logax |
	'l' {	chrp = $1;
		if(starflg)
		  {
			if(xyflg == X)
				charp = _axptr->lab = xlabbuf;
			else
				charp = _axptr->lab = ylabbuf;
			while((*chrp == ' ') || (*chrp == '\t'))
				chrp++;
			while((*chrp != '%') && (*chrp != '\0'))
				*charp++ = *chrp++;
			*charp = '\0';
		  }
		else
			_axptr->lab = (*argp++).cp;}| 
	't' {	
		if(starflg)
			frag = atoi($1);
		else
			frag = (*argp++).ip;
		if((frag >= 0) && (frag <= 3))
			_axptr->flags |= frag;
		else
			printf("Bad Tic (%t) type %d\n",frag); } |
	'T' {	
		if(starflg)
			frag = atoi($1);
		else
			frag = (*argp++).ip;
		if((frag >= 0) && (frag <= 3))
			_axptr->flags |= frag << 2;
		else
			printf("Bad Lab (%%L) type %o\n",frag); } |
	'L' {	chrp = $1;
		_axptr->flags =| NAME;
		if(starflg)
		  {
			if(xyflg == X)
				charp = _axptr->lab = xlabbuf;
			else
				charp = _axptr->lab = ylabbuf;
			while((*chrp == ' ') || (*chrp == '\t'))
				chrp++;
			while((*chrp != '%') && (*chrp != '\0'))
				*charp++ = *chrp++;
			*charp = '\0';
		  }
		else
			_axptr->lab = (*argp++).cp;}| 
	     numpts;
shift:
	gridtype |
	's' = {
		if(starflg)
		  {
			if(xyflg == X)
				_axptr->y = atof($1);	
			else
				_axptr->x = atof($1);
		  }
		else
		  {
			if(xyflg == X)
				_axptr->y = argp->dv;
			else
				_axptr->x = argp->dv;
			argp += 4;
		  }
		_axptr->flags =| SHIFT;
	  } |
	'S' = {
		if(starflg)
		  {
			if(xyflg == X)
				_axptr->y = atof($1);	
			else
				_axptr->x = atof($1);
		  }
		else
		  {
			if(xyflg == X)
				_axptr->y = argp->dv;
			else
				_axptr->x = argp->dv;
			argp += 4;
		  }
		_axptr->flags =| SHIFT1;
	  };


sdisplay:	
	sdisplay display |
	display;

display:
	xyd dis;

dis:	dis disp |
	dis numpts |
	dis datatype |
	dis minmax |
	dis logax |
	/* empty */;

disp:	
	'c' = {
		if(starflg)
		  {
			for(sp = $1;*sp != '%';sp++)
				if(*sp == ' ')
					continue;
				else
					break;
			_datptr->symble = *sp;
		  }
		else
			_datptr->symble = (*argp++).cp;
		} |
	  'b' = {
		if(starflg)
			_datptr->blubber = atoi($1);
		else
			_datptr->blubber = (*argp++).ip;
		} |
	'e' {	chrp = $1;
		if(starflg)
		  {
			charp = _datptr->labl = calloc(1,30);
			if(charp == 0)
				_feror("No space available: calloc");
			while((*chrp == ' ') || (*chrp == '\t'))
				chrp++;
			while((*chrp != '%') && (*chrp != '\0'))
				*charp++ = *chrp++;
			*charp = '\0';
		  }
		else
			_datptr->labl = (*argp++).cp;}| 
	  'B' = {
		if(starflg)
		  {
		_datptr->ddash = calloc(1,16);
		if(_datptr->ddash == 0)
			_feror("No space available: calloc");
		if(sscanf($1,"%f%f%f%f",_datptr->ddash,_datptr->ddash+1,
		  _datptr->ddash + 2,_datptr->ddash + 3) != 4)
		  {
		   fprintf(stderr,"Not enough arguments for 'B' command\n");	
		   fprintf(stderr,"%f %f %f %f\n",*_datptr->ddash
			,*(_datptr->ddash+1),
			*(_datptr->ddash + 2),*(_datptr->ddash + 3));
			_datptr->ddash = 0;
		  }
		  }
		else
			_datptr->ddash = (*argp++).fp;
		};
numpts:	'n' {
		if(starflg)
			_datptr->npts = atoi($1);
		else
			_datptr->npts = (*argp++).ip; };
datatype:

	'd' {_datptr->flgs =| INTEGER;}|
	'i' {_datptr->flgs =| INTEGER;}|
	'f' {_datptr->flgs =| FLOAT;}|
	'F' {_datptr->flgs =| DOUBLE;}|
	'u' {_datptr->flgs =| UNSIGNED;}|
	'h' {_datptr->flgs =| SHORT;}|
	'D' {_datptr->flgs =| LONG;}|
	'I' {_datptr->flgs =| LONG;};

minmax:	'm' {
		_axptr->flags =| MINSET;
		if(starflg)
			min[xyflg] = atof($1);
		else
		  {
			min[xyflg] = argp->dv;
			argp += 4;
		  }
	     }
	|
	'M'{	_axptr->flags =| MAXSET;
		if(starflg)
			max[xyflg] = atof($1);
		else
		  {
			max[xyflg] = argp->dv;
			argp += 4;
		  }
	   } ;
gridtype: 'G' {
		if(starflg)
			frag = atoi($1);
		else
			frag = (*argp++).ip;
		if((frag >= 0) && (frag <= 3))
			_axptr->flags |= frag << 5;
		else
			printf("Bad Gridtype (%%G) type %o\n",frag);
	   } ;
logax:	'o' {
		_axptr->flags =| LOGAXIS;
	    };
grid: xyd ggrid xyd ggrid;

ggrid: ggrid rgrid |
	/* empty */;
rgrid: minmax |
	numpts |
	logax |
	gridtype |
	datatype ;
%%


yylex()
  {
	extern int yylval;
	starflg = 0;
	while((*scanp != '%') && (*scanp != '\0'))
	  {
		ychar++;
		scanp++;
	  }
	if(*scanp++ == '\0')
		return(0);
	ychar++;
	*(scanp - 1) = 0;	/* To make previous string null terminated */
	if(*scanp == '*')
	  {
		(starflg)++;
		scanp++;
		ychar++;
	  }
	if(starflg)
		yylval = scanp + 1; /* pointer to string after token */
	else
		yylval = argp;
	ychar++;
	return(*scanp++);
  }
yyerror(s)
char *s;
  {
	char *str;

	ychar -= 5;
	str = strbeg;
	fprintf(stderr,"\n%s\n:  ",s);
	fprintf(stderr,"error near underbar\n");
	while(str - strbeg < ychar)
		putc(*str++,stderr);
	fprintf(stderr,"_%s\n",scanp);
  }
