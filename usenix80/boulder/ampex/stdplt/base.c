# include "graph.h"
# include <stdplt.h>
base()
  {
	struct axtype *p;

	p = _ax;
	p->x = min[X];
	/* if shift has been declaired the value is stored in p->(y or x) */
	if(p->flags & SHIFT)
	  {
		p->y *= (max[Y] - min[Y]);
		p->y += min[Y];
	  }
	else if(!(p->flags & SHIFT1))
		p->y = min[Y];
	p++;
	p->y = min[Y];
	if(p->flags & SHIFT)
	  {
		p->x *= (max[X] - min[X]);
		p->x += min[X];
	  }
	else if(!(p->flags & SHIFT1))
		p->x = min[X];
  }
