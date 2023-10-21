/* Arraychar.p -- type-specific functions for class Arraychar

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	September, 1985

Function:
	
Type-specific functions for class Arraychar, such as hash() and
printOn().  Generic functions are generated from the m4 template file
Array.c.m4.

WARNING -- Make changes to the .p or .m4 files, not to the .c file
they generate.

$Log:	Arraychar.p,v $
 * Revision 1.2  88/01/16  23:37:24  keith
 * Remove pre-RCS modification history
 * 
*/

`#include' "Arraychar.h"
`#include' "oopsconfig.h"
`#include' <libc.h>
`#include' "oopsIO.h"

`#define'	THIS	Arraychar
`#define'	BASE	Collection
DEFINE_CLASS(Arraychar,Collection,1,"$Header: Arraychar.p,v 1.2 88/01/16 23:37:24 keith Exp $",NULL,NULL);

static int charCmp(char* a, char* b)
{
	return *(unsigned char*)a - *(unsigned char*)b;
}

static union hash_char_mask {
	unsigned in[sizeof(int)];
	char ch[sizeof(int)*sizeof(int)];
	hash_char_mask();
} mask;

static hash_char_mask::hash_char_mask()
{
	for (register unsigned i=0; i<sizeof(int); i++) {
		for (register unsigned j=0; j<sizeof(int); j++) ch[sizeof(int)*i+j] = j<i ? 0xff : 0;
	}
}

unsigned THIS::hash()
{
	register unsigned h = sz;
	register unsigned i = div_sizeof_int(sz);
	register unsigned* vv = (unsigned*)v;
	while (i--) h ^= *vv++;
	if ((i = mod_sizeof_int(sz)) != 0)
		h ^= *vv & mask.in[i];
	return h;
}

void THIS::printOn(ostream& strm)
{
	strm << Arraychar() << "[";
	for (register unsigned i=0; i<sz; i++) {
		strm << "\t" << form("0x%.2x",(unsigned int)(unsigned char)v[i]);
	}
	strm << "]\n";
}

THIS::THIS(istream& strm, THIS& where)
{
	this = &where;
	strm >> sz;
	v = NEW(char,sz);
	for (register unsigned i =0; i<sz; i++) { int _i; strm >> _i; v[i] = _i; }
}

void THIS::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz << " ";
	for (register unsigned i=0; i<sz; i++) strm << (unsigned int)(unsigned char)v[i] << " ";
}

Arraychar::Arraychar(fileDescTy& fd, Arraychar& where)
{
	this = &where;
	readBin(fd,sz);
	v = NEW(char,sz);
	readBin(fd,v,sz);
}

void Arraychar::storer(fileDescTy& fd) 
{
	Object::storer(fd);
	storeBin(fd,sz);
	storeBin(fd,v,sz);
}

ARRAYIMPLEMENT(Arraychar,char)
