# include "graph.h"
# include <stdplt.h>
_minmax(min,max,p,n,flag)
union ptr *p;
double *min,*max;
int n;
int flag;
  {
	int i;
	int type;

	type = flag & DATATYPE;
	switch(type)
	  {
		case DOUBLE:
			for(i = 0;i < n;i++)
			  {
				if(!(flag&MINSET))
				  {
					if(*p.dp < *min)
						*min = *p.dp;
				  }
				if(!(flag&MAXSET))
				  {
					if(*p.dp > *max)
						*max = *p.dp;
				  }
				p.dp++;
			  }
			break;
		case FLOAT:
			for(i = 0;i < n;i++)
			  {
				if(!(flag&MINSET))
				  {
					if(*p.fp < *min)
						*min = *p.fp;
				  }
				if(!(flag&MAXSET))
				  {
					if(*p.fp > *max)
						*max = *p.fp;
				  }
				p.fp++;
			  }
			break;
		case INTEGER:
			for(i = 0;i < n;i++)
			  {
				if(!(flag&MINSET))
				  {
					if(*p.ip < *min)
						*min = *p.ip;
				  }
				if(!(flag&MAXSET))
				  {
					if(*p.ip > *max)
						*max = *p.ip;
				  }
				p.ip++;
			  }
			break;
		case LONG:
			for(i = 0;i < n;i++)
			  {
				if(!(flag&MINSET))
				  {
					if(*p.lp < *min)
						*min = *p.lp;
				  }
				if(!(flag&MAXSET))
				  {
					if(*p.lp > *max)
						*max = *p.lp;
				  }
				p.lp++;
			  }
			break;
		case SHORT:
			for(i = 0;i < n;i++)
			  {
				if(!(flag&MINSET))
				  {
					if(*p.hp < *min)
						*min = *p.hp;
				  }
				if(!(flag&MAXSET))
				  {
					if(*p.hp > *max)
						*max = *p.hp;
				  }
				p.hp++;
			  }
			break;
		case UNSIGNED:
			for(i = 0;i < n;i++)
			  {
				if(!(flag&MINSET))
				  {
					if(*p.up < *min)
						*min = *p.up;
				  }
				if(!(flag&MAXSET))
				  {
					if(*p.up > *max)
						*max = *p.up;
				  }
				p.up++;
			  }
			break;
		  }
  }
