#include "ByteVec.h"
#include "Vectorimp.h"
#include "oopsconfig.h"
#include <libc.h>
#define	THIS	ByteVec
#define	BASE	Object
DEFINE_CLASS(ByteVec,Object,1,NULL,NULL);
Vectorimplement(ByteVec,char);

static int typeCmp(char& a, char& b)
{
	return a-b;
}

static union hash_byte_mask {
	int in[sizeof(int)];
	char ch[sizeof(int)*sizeof(int)];
	hash_byte_mask();
} mask;

static hash_byte_mask::hash_byte_mask()
{
	for (register int i=0; i<sizeof(int); i++) {
		for (register int j=0; j<sizeof(int); j++) ch[sizeof(int)*i+j] = j<i ? 0xff : 0;
	}
}

int THIS::hash()
{
	register int h = sz;
#ifdef LOG2_INT
	register int i = sz >> LOG2_INT;
#else
	register int i = sz/sizeof(int);
#endif
	register int* vv = (int*)v;
	while (i--) h ^= *vv++;
#ifdef LOG2_INT
	if ((i = sz&(sizeof(int)-1)) != 0)
#else
	if ((i = sz%sizeof(int)) != 0)
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
	v = new char[sz];
	for (register int i =0; i<sz; i++) { int _i; strm >> _i; v[i] = _i; }
}

void THIS::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz << " ";
	for (register int i=0; i<sz; i++) strm << (unsigned int)(unsigned char)v[i] << " ";
}
