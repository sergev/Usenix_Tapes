
/*
 * Gprolog 1.4/1.5
 *
 * Barry Brachman
 * Dept. of Computer Science
 * Univ. of British Columbia
 * Vancouver, B.C. V6T 1W5
 *
 * .. {ihnp4!alberta, uw-beaver}!ubc-vision!ubc-cs!brachman
 * brachman@cs.ubc.cdn
 * brachman%ubc.csnet@csnet-relay.arpa
 * brachman@ubc.csnet
 */

#include "pl.h"
#include "gr.h"

#include <signal.h>

extern struct Core_info Core_info[];
extern int ncorefuncs;

/*
 * Binary search
 */
struct Core_info
*lookup_core(s)
char *s;
{
	register int low,high,mid,cond;
	register struct Core_info *cp;

	cp = Core_info;
	low = 0;
	high = ncorefuncs;
	while (low <= high) {
		mid = (low + high) / 2;
		if ((cond = strcmp(s,(cp+mid)->Core_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return(cp+mid);
	}
	return(0);
}

getenv_mapper(s1, s2)
char *s1, *s2;
{
	char *p;
	char *getenv();

	if ((p = getenv(s1)) == 0)
		return(1);
	strcpy(s2, p);
	return(0);
}

resetsigs()
{
	signal(SIGBUS,oldbussignal);
	signal(SIGSEGV,oldsegvsignal);
}

err()
{
	sprintf(OutBuf,"plgraphics: "); PutString(OutBuf);
}

err1(s)
char *s;
{
	sprintf(OutBuf,"plgraphics: %s\n",s); PutString(OutBuf);
}

err2(s1,s2)
char *s1,*s2;
{
	sprintf(OutBuf,"plgraphics: "); PutString(OutBuf);
	sprintf(OutBuf,s1,s2); PutString(OutBuf);
	sprintf(OutBuf,"\n"); PutString(OutBuf);
}

err3(i,s1,s2)
int i;
char *s1,*s2;
{
	sprintf(OutBuf,"plgraphics: "); PutString(OutBuf);
	sprintf(OutBuf,"Argument %d of %s must be %s\n",i,s1,s2);
	PutString(OutBuf);
}

test1(i,f)
int i;
float f;
{
	sprintf(OutBuf,"test1: %d,%f\n",i,f); PutString(OutBuf);
	return(0);
}

test2(n,f)
int n;
float *f;
{
	register int i;

	sprintf(OutBuf,"test2:\n"); PutString(OutBuf);
	for (i = 0; i < n; i++) {
		sprintf(OutBuf,"%f\n",f[i]);
		PutString(OutBuf);
	}
	return(0);
}

test3(s)
char *s;
{
	sprintf(OutBuf,"test3: %s\n",s); PutString(OutBuf);
	return(0);
}

test4(n,v)
int n,*v;
{
	register int i;

	sprintf(OutBuf,"test4:\n"); PutString(OutBuf);
	for (i = 0; i < n; i++) {
		sprintf(OutBuf,"%d\n",v[i]);
		PutString(OutBuf);
	}
	return(0);
}

test5(ch)
char ch;
{
	sprintf(OutBuf,"test5: %d\n",ch); PutString(OutBuf);
	return(0);
}

test6(s)
char *s;
{
	sprintf(OutBuf,"test6:\n"); PutString(OutBuf);
	strcpy(s,"test6 string");
	return(0);
}

test7(s, f1, f2)
char *s;
float *f1, *f2;
{

	sprintf(OutBuf, "test7: s=%s\n", s); PutString(OutBuf);
	*f1 = 1.23;
	*f2 = 2.34;
	return(0);
}

/*
 * Print a list of the implemented functions
 * in a nice table
 *
 * NOTE: Ideally, this routine should check on the size of the
 * window it is running in through termcap and adjust the number
 * of columns dynamically.
 * Problem is that the window system does not seem to distribute
 * SIGWINCH as advertised.
 */
list()
{
	int pentry(), len();

	sprintf(OutBuf,"%d functions:\n",ncorefuncs);
	PutString(OutBuf);
	prtable(Core_info, ncorefuncs, 0, 0, pentry, len);
	return(0);
}

static int
pentry(base, index)
struct Core_info *base;
int index;
{
	register int l;
	register struct Core_info *p;

	p = base + index;
	sprintf(OutBuf, "%s(%d)", p->Core_name, p->Core_arity);
	l = strlen(OutBuf);
	PutString(OutBuf);
	return(l);
}

static int
len(base, index)
struct Core_info *base;
int index;
{
	register struct Core_info *p;

	p = base + index;
	sprintf(OutBuf, "%s(%d)", p->Core_name, p->Core_arity);
	return(strlen(OutBuf));
}

