#include "IntVec.h"
#include "Vectorimp.h"
#include <libc.h>
#define	THIS	IntVec
#define	BASE	Object
DEFINE_CLASS(IntVec,Object,1,NULL,NULL);
Vectorimplement(IntVec,int)

static int typeCmp(int& a, int& b)
{
	return a-b;
}

int THIS::hash()
{
	register int h = sz;
	register int i = sz;
	register int* vv = v;
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
	v = new int[sz];
	for (register int i =0; i<sz; i++) strm >> v[i];
}

void THIS::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz << " ";
	for (register int i=0; i<sz; i++) strm << v[i] << " ";
}
