#include "DoubleVec.h"
#include "Vectorimp.h"
#include "oopsconfig.h"
#include <libc.h>
#define	THIS	DoubleVec
#define	BASE	Object
DEFINE_CLASS(DoubleVec,Object,1,NULL,NULL);
Vectorimplement(DoubleVec,double);

static int typeCmp(double& a, double& b)
{
	register double t = a-b;
	if (t < 0) return -1;
	if (t > 0) return 1;
	return 0;
}

int THIS::hash()
{
	register int h = sz;
#ifdef LOG2_INT
	register int i = sz*sizeof(double) >> LOG2_INT;
#else
	register int i = sz*sizeof(double)/sizeof(int);
#endif
	register int* vv = (int*)v;
	while (i--) h ^= *vv++;
	return h;
}

void THIS::printOn(ostream& strm)
{
	for (register int i=0; i<sz; i++) {
		if (i>0 && (i%6 == 0)) strm << "\n";
		strm << form("%13.4g",v[i]);
	}
}

THIS::THIS(istream& strm, THIS& where)
{
	this = &where;
	strm >> sz;
	v = new double[sz];
	for (register int i =0; i<sz; i++) strm >> v[i];
}

void THIS::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz << " ";
	for (register int i=0; i<sz; i++) strm << form("%.14g",v[i]) << " ";
}
