# include "graph.h"
# include <stdplt.h>
Do_disp()
  {
	int i;
	struct _data *lbel;

	for(i = 0;i<ngraphs;i++)
	  {
		if(_xdata[i]->labl)
			lbel = _xdata[i];
		else 
			lbel = _ydata[i];
		if(lbel->labl)
		  {
			move(_ax[X].x,(max[Y] - min[Y])/2.);
			cmove((-20. + (i * 1.5)),1.);
			_vertextf(lbel->labl);
			rmove(0.,_ax[Y].len/20.);
			dash(i);
			rdraw(0.,_ax[Y].len/10.);
			cmove(-.25,0.);
			if(_ydata[i]->symble)
				textf("%c",_ydata[i]->symble);
			else
				textf("%c",_xdata[i]->symble);
			dash(0);
		  }
		_display(_xdata[i],_ydata[i],i);
	  }
  }
