#include "apl.h"

ex_dibm()
{
	register j, arg, *p;


	/* Dyadic i-beam functions.  I-beam 63 assumes that the
	 * "empty" system call (check whether pipe empty) has been
	 * implemented in the Unix kernel.
	 */

	arg = topfix();		/* Get left argument */

	switch(topfix()) {

	default:
		error("unknown i-beam");

	case 29: /* Set origin, return old one */
		datum = thread.iorg;
		thread.iorg = arg;
		break;

	case 30: /* Set width, return old one */
		datum = thread.width;
		thread.width = arg;
		break;

	case 31: /* Set number of digits, return old one */
		datum = thread.digits;
		thread.digits = arg;
		break;

	case 34: /* "Nice" system call */
		datum = nice(arg);
		break;

	case 35: /* "Sleep" system call */
		datum = sleep(arg);
		break;

	case 63: /* "Empty" system call */
		datum = empty(arg);
		break;

	}

	p = newdat(DA, 0, 1);
	p->datap[0] = datum;
	*sp++ = p;

}

int afnfree, afnused;


ex_mibm()
{
	struct si *gp;
	register i, *p;
	int t[6];

	switch(topfix()) {

	default:
		error("unknown i-beam");

	case 20: /* time of day */
		time(t);
		p = t;
		goto tod;

	case 21: /* CPU time */
		times(t);
		t[3] = t[0];
		t[0] = 0;
		t[2] = 0;
		datum = ltod(t) + ltod(t+2);
		break;

	case 22: /* ws bytes unused */
		datum = afnfree;
		break;

	case 24: /* starting time */
		p = stime;

	tod:
		p = localtime(p);
		datum = 60.*(p[0]+60.*(p[1]+60.*p[2]));
		break;

	case 25: /* date */
		time(t);
		p = t;
		goto dt;

	case 26:	/* current line */
		datum = (gsip ? gsip->funlc - 1 : 0);
		break;

	case 27: /* vector of line numbers of fn activations # */
		i = 0;
		gp = gsip;
		while(gp){
			if(gp->np)
				i++;
			gp = gp->sip;
		}
		p = newdat(DA, 1, i);
		gp = gsip;
		i = 0;
		while(gp){
			if(gp->np);
				p->datap[i++] = gp->funlc - 1;
			gp = gp->sip;
		}
		*sp++ = p;
		return;

	/*
	 * non standard I functions
	 */

	case 28: /* starting date */
		p = stime;

	dt:
		p = localtime(p);
		datum = p[5]+100.*(p[3]+100.*(p[4]+1));
		break;

	case 29: /* iorg */
		datum = thread.iorg;
		break;

	case 30: /* width */
		datum = thread.width;
		break;

	case 31: /* digits */
		datum = thread.digits;
		break;

	case 32: /* ws bytes in use */
		datum = afnused;
		break;

	case 36: /* 2nd element of ib27 */
		datum = ((gsip && gsip->sip) ? gsip->sip->funlc - 1 : 0);
		break;

	case 40: /* Total accumulated child's time */
		times(t);
		datum = ltod(t+2) + ltod(t+4);
		break;

	case 41: /* Total accumulated user time -- including all kids */
		times(t);
		t[4] = 0;
		t[5] = t[0];
		datum = ltod(t+2) + ltod(t+4);
		break;

	case 42: /* Total system time -- including all kids */
		times(t);
		t[0] = 0;
		datum = ltod(t) + ltod(t+4);
		break;

	case 43: /* User time -- parent only */
		times(t);
		datum = t[0];
		break;

	case 44: /* System time -- parent only */
		times(t);
		datum = t[1];
		break;

	case 96:
		dstack();


	case 97:
		datum = (sp - stack) / 2;
		break;

	case 98: /* turn off alloc/free trace */
		datum = aftrace;
		aftrace = 0;
		break;

	case 99: /* turn on alloc/free trace */
		datum = aftrace;
		aftrace = 1;
		break;
	}
	p = newdat(DA, 0, 1);
	p->datap[0] = datum;
	*sp++ = p;
}
