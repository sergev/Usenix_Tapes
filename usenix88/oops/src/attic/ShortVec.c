#include "ShortVec.h"
#include "Vectorimp.h"
#include "oopsconfig.h"
#include <libc.h>
#define	THIS	ShortVec
#define	BASE	Object
DEFINE_CLASS(ShortVec,Object,1,NULL,NULL);
Vectorimplement(ShortVec,short)

static int typeCmp(short& a, short& b)
{
	return a-b;
}

static union hash_short_mask {
	int in[sizeof(int)/sizeof(short)];
	short sh[sizeof(int)/sizeof(short)*sizeof(int)/sizeof(short)];
	hash_short_mask();
} mask;

hash_short_mask::hash_short_mask()
{
	for (register int i=0; i<sizeof(int)/sizeof(short); i++) {
		for (register int j=0; j<sizeof(int)/sizeof(short); j++) sh[sizeof(int)*i+j] = j<i ? -1 : 0;
	}
}

int THIS::hash()
{
	register int h = sz;
#ifdef LOG2_INT
	register int i = sz >> (LOG2_INT-LOG2_SHORT);
#else
	register int i = sz/(sizeof(int)/sizeof(short));
#endif
	register int* vv = (int*)v;
	while (i--) h ^= *vv++;
#ifdef LOG2_INT
	if ((i = sz&((sizeof(int)/sizeof(short))-1)) != 0)
#else
	if ((i = sz%(sizeof(int)/sizeof(short))) != 0)
#endif
		h ^= *vv & mask.in[i];
	return h;
}

void THIS::printOn(ostream& strm)
{
	for (register int i=0; i<sz; i++) {
		if (i>0 && (i%10 == 0)) strm << "\n";
		strm << dec(v[i],8);
	}
}

THIS::THIS(istream& strm, THIS& where)
{
	this = &where;
	strm >> sz;
	v = new short[sz];
	for (register int i =0; i<sz; i++) strm >> v[i];
}

void THIS::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz << " ";
	for (register int i=0; i<sz; i++) strm << v[i] << " ";
}
