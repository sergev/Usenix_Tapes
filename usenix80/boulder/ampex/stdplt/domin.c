# include "graph.h"
# include <stdplt.h>
Do_min(data,xy)
struct _data **data[];
char xy;
  {
	int n;
	int flag;
	int step;
	double *x;

	step = 0;
	if(nary[xy] == 1)
		data[step]->flgs =| MINMAX;
	while(1)
	  {
		flag = (data[step]->flgs)|(_ax[xy].flags);
		n = data[step]->npts;
		if((n <= 0) && ((flag&DATATYPE) != NODATA))
		  {
			_error("Bad or missing number of points");
		  }
		else
		  {
			x = data[step]->dptr;
			if(flag&MINMAX)
				if(!((flag&DATATYPE) == NODATA))
					_minmax(&min[xy],&max[xy],x,n,flag);
		  }
		step++;
		if(step >= nary[xy]) 
			break;
	  }
	if(min[xy] >= max[xy])
	  {
		printf("Minmax not defined or bad: %c",xy + 'X');
		exit();
	  }
  }
