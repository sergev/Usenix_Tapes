#

/*
 *      priority interrupt request handler.
 */

#include "../param.h"
#define PIRADDR 0177772
#define PQLEN   10

struct  pq
{
	int     (*p_fun)();
	int     p_lev;
}pq[PQLEN];

pireq(fun,lev)
int (*fun)();
{
	register struct pq *p;
	register i,s;
	s = PS->integ;
	spl7();
	p = &pq[0];
	while(p->p_lev != 0)
		p++;
	p->p_fun = fun;
	p->p_lev = lev;
	i = 0400;
	i =<< lev;
	PIRADDR->integ =| i;
	PS->integ = s;
}

pirqint()
{
	register struct pq *p;
	register i,j;
	j = (PIRADDR->integ >> 1) & 07;
	for(;;) {
		p = &pq[0];
		while(p->p_lev != 0 && p->p_lev != j)
			p++;
		if(p->p_lev == 0) goto done;
		PS->lobyte = PIRADDR->lobyte;
		(*p->p_fun)();
		spl7();
		do {
			p->p_fun = (p+1)->p_fun;
			p->p_lev = (p+1)->p_lev;
		}while((p++)->p_lev != 0);
	}
done:
	i = 0400;
	i =<< j;
	PIRADDR->integ =& ~i;
}
