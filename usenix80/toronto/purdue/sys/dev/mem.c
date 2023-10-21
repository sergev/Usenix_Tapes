#
/*
 */

/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/RATHOLE
 *      minor device 3 is errorlog
 *	minor device 4 is statlog
 */

#include "../param.h"
#include "../user.h"
#include "../conf.h"
#include "../seg.h"
#include "../errlog.h"

mmread(dev)
{
	register c, bn, on;
	int a, d;

	if(dev.d_minor == 2)
		return;
	if(dev.d_minor == 3) {
		while(!errlog.cc)
			sleep(&errlog, ERLPRI);
		while(errlog.cc && passc(c=getc(&errlog)) >= 0)
			if(c == '\n')
				return;
		return;
	}
	if(dev.d_minor == 4) {
		while(!statlog.cc)
			sleep(&statlog, ERLPRI);
		while(statlog.cc && passc(c=getc(&statlog)) >= 0)
			if(c == '\n')
				return;
		return;
	}
	do {
		bn = lshift(u.u_offset, -6);
		on = u.u_offset[1] & 077;
		a = UISA->r[0];
		d = UISD->r[0];
		spl7();
		UISA->r[0] = bn;
		UISD->r[0] = 077406;
		if(dev.d_minor == 1)
			UISA->r[0] = (ka6-6)->r[(bn>>7)&07] + (bn & 0177);
		c = fuibyte(on);
		UISA->r[0] = a;
		UISD->r[0] = d;
		spl0();
	} while(u.u_error==0 && passc(c)>=0);
}

mmwrite(dev)
{
	register c, bn, on;
	int a, d;

	if (dev.d_minor == 3 || dev.d_minor == 4) {
		spl6();
		errlg++;
		if (dev.d_minor == 4)
			errlg = -1;
		while((c = cpass()) >= 0)
			putchar(c);
		errlg = 0;
		spl0();
		return;
	}
	if(dev.d_minor == 2) {
		c = u.u_count;
		u.u_count = 0;
		u.u_base =+ c;
		dpadd(u.u_offset, c);
		return;
	}
	for(;;) {
		bn = lshift(u.u_offset, -6);
		on = u.u_offset[1] & 077;
		if ((c=cpass())<0 || u.u_error!=0)
			break;
		a = UISA->r[0];
		d = UISD->r[0];
		spl7();
		UISA->r[0] = bn;
		UISD->r[0] = 077406;
		if(dev.d_minor == 1)
			UISA->r[0] = (ka6-6)->r[(bn>>7)&07] + (bn & 0177);
		suibyte(on, c);
		UISA->r[0] = a;
		UISD->r[0] = d;
		spl0();
	}
}
mmopen(dev)
{
	if (dev.d_minor == 3)
		errlog.EOpen++;
}

mmclose(dev)
{
	if (dev.d_minor == 3)
		errlog.EOpen = 0;
}
