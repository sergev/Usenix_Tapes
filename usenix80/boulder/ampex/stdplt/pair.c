/*	Make them into pairs of data and exchange npts and count em too */

# include "graph.h"
# include <stdplt.h>
pair()
  {
	int step;

	if(_xdata[0] == 0)
		_feror("Missing or bad X aray:");
	if(_ydata[0] == 0)
		_feror("Missing or bad Y aray:");
	for(step = ngraphs = 0;;)
	  {
		if(_xdata[step]->npts <= 0)
			_xdata[step]->npts = _ydata[step]->npts;
		if(_ydata[step]->npts <= 0)
			_ydata[step]->npts = _xdata[step]->npts;
		step++;
		if((step >= nary[X]) && (step >= nary[Y]))
		  {
			ngraphs = step;
			break;
		  }
		if(step >= nary[X])
			_xdata[step] = _xdata[step - 1];
		if(step >= nary[Y])
			_ydata[step] = _ydata[step - 1];
	  }
  }
