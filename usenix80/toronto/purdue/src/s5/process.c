#
#include	"/u/sys/param.h"
process(xpid)
int xpid;
{
	register who, i;
	int pdv;
	int *npa, *npb;
#include   "/u/sys/proc.h"
	static struct proc buff;
	pdv=open("/dev/proc",0);
	read(pdv,proc,sizeof proc);
	close(pdv);
	for(who=0;who < NPROC;who++)
		if(proc[who].p_pid == xpid)
			{
			npa= &proc[who];
			npb= &buff;
			for(i=0;i < sizeof buff/2;i++)
				*npb++= *npa++;
			return(&buff);
		}
	return(0);
}
