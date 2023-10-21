/* ArrayOb.c -- member functions of class ArrayOb

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

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
	
Member function definitions for class ArrayOb (Array of Object*.
Objects of class ArrayOb are used in the implementations of several
other Collection classes such as: Bag, Dictionary, Set, and
OrderedCltn.  Note that the ArrayOb constructor initializes the array
with pointers to the nil object.

$Log:	ArrayOb.c,v $
 * Revision 1.3  88/02/04  12:50:27  keith
 * Make Class class_CLASSNAME const.
 * Make destructor non-inline.
 * 
 * Revision 1.2  88/01/16  23:36:42  keith
 * Remove pre-RCS modification history
 * 
*/

#include <libc.h>
#include "ArrayOb.h"
#include "oopsIO.h"

#define	THIS	ArrayOb
#define	BASE	Collection
DEFINE_CLASS(ArrayOb,Collection,1,"$Header: ArrayOb.c,v 1.3 88/02/04 12:50:27 keith Exp $",NULL,NULL);

extern const int OOPS_ALLOCSIZE,OOPS_INDEXRANGE;

#define NEW(type,size)	((type*)malloc(sizeof(type)*(size)))

ArrayOb::ArrayOb(unsigned size)
{
	sz = size;
	if (sz==0) allocSizeErr();
	v = NEW(Object*,sz);
	register i = sz;
	register Object** vp = &v[0];
	while (i--) *vp++ = nil;
}
	
ArrayOb::ArrayOb(const ArrayOb& a)
{
	register i = a.sz;
	sz = i;
	v = NEW(Object*,i);
	register Object** vp = &v[0];
	register Object** av = &a.v[0];
	while (i--) *vp++ = *av++;
}

ArrayOb::~ArrayOb()	{ DELETE(v); }

void ArrayOb::operator=(const ArrayOb& a)
{
	if (v != a.v) {
		DELETE(v);
		v = NEW(Object*,sz=a.sz);
		register i = a.sz;
		register Object** vp = &v[0];
		register Object** av = &a.v[0];
		while (i--) *vp++ = *av++;
	}
}

bool ArrayOb::operator==(const ArrayOb& a)
{
	if (sz != a.sz) return NO;
	register unsigned i = sz;
	register Object** vp = &v[0];
	register Object** av = &a.v[0];
	while (i--) { if (!((*vp++)->isEqual(**av++))) return NO; }
	return YES;
}

Object*& ArrayOb::at(int i)	{ return (*this)[i]; }

unsigned ArrayOb::capacity()	{ return sz; }
	
bool ArrayOb::isEqual(const Object& a)
{
	return a.isSpecies(class_ArrayOb) && *this==*(ArrayOb*)&a;
}

const Class* ArrayOb::species()	{ return &class_ArrayOb; }

void ArrayOb::reSize(unsigned newsize)
{
	if (newsize<=0) allocSizeErr();
	v = REALLOC(v,newsize);
	sz = newsize;
}

Collection& ArrayOb::addContentsTo(Collection& cltn)
{
	register Object** vp = &v[0];
	register unsigned i = sz;
	while (i--) cltn.add(**vp++);
	return cltn;
}

Object* ArrayOb::doNext(Iterator& pos)
{
	if (pos.index < size()) return v[pos.index++];
	return 0;
}

void ArrayOb::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	register Object** av = &v[0];
	register i = sz;
	v = NEW(Object*,i);
	register Object** vp = &v[0];
	while (i--) *vp++ = (*av++)->deepCopy();
}

unsigned ArrayOb::hash()
{
	register unsigned h = sz;
	register unsigned i = sz;
	register Object** vp = &v[0];
	while (i--) h^=(*vp++)->hash();
	return h;
}

void ArrayOb::printOn(ostream& strm)
{
	strm << className() << "[\n";
	for (register unsigned i=0; i<sz; i++) {
		if (i>0) strm << "\n";
		v[i]->printOn(strm);
	}
	strm << "]\n";
}

ArrayOb::ArrayOb(istream& strm, ArrayOb& where)
{
	this = &where;
	strm >> sz;
	v = NEW(Object*,sz);
	for (register unsigned i=0; i<sz; i++) v[i] = readFrom(strm);
}

void ArrayOb::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz;
	for (register unsigned i=0; i<sz; i++) v[i]->storeOn(strm);
}

unsigned ArrayOb::size()	{ return sz; }

static int compare_ob(Object** a, Object** b) { return (*a)->compare(**b); }

void ArrayOb::sort()
{
	qsort(&v[0],sz,sizeof(Object*),compare_ob);
}

void ArrayOb::allocSizeErr()
{
	setOOPSerror(OOPS_ALLOCSIZE,DEFAULT,this,className());
}

void ArrayOb::indexRangeErr()
{
	setOOPSerror(OOPS_INDEXRANGE,DEFAULT,this,className());
}

ArrayOb::ArrayOb(fileDescTy& fd, ArrayOb& where)
{
	this = &where;
	readBin(fd,sz);
	v = NEW(Object*,sz);
	for (register unsigned i=0; i<sz; i++ )
	   v[i] = readFrom(fd);
}

void ArrayOb::storer(fileDescTy& fd) 
{
	Object::storer(fd);
	storeBin(fd,sz);
	for (register unsigned i=0; i<sz; i++) 
		v[i]->storeOn(fd);
}
