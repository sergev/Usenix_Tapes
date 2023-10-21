#include "LongVec.h"
#include "Vectorimp.h"
#include "oopsconfig.h"
#include <libc.h>
#define	THIS	LongVec
#define	BASE	Object
DEFINE_CLASS(LongVec,Object,1,NULL,NULL);
Vectorimplement(LongVec,long)

static int typeCmp(long& a, long& b)
{
	register long t = a-b;
	if (t < 0) return -1;
	if (t > 0) return 1;
	return 0;
}

int THIS::hash()
{
	register int h = sz;
#ifdef LOG2_INT
	register int i = sz*sizeof(long) >> LOG2_INT;
#else
	register int i = sz*sizeof(long)/sizeof(int);
#endif
	register int* vv = (int*)v;
	while (i--) h ^= *vv++;
	return h;
}

void THIS::printOn(ostream& strm)
{
	for (register int i=0; i<sz; i++) {
		if (i>0 && (i%8 == 0)) strm << "\n";
		strm << dec(v[i],10);
	}
}

THIS::THIS(istream& strm, THIS& where)
{
	this = &where;
	strm >> sz;
	v = new long[sz];
	for (register int i =0; i<sz; i++) strm >> v[i];
}

void THIS::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz << " ";
	for (register int i=0; i<sz; i++) strm << v[i] << " ";
}
