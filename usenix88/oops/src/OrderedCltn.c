/* OrderedCltn.c -- implementation of abstract ordered collections

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
	
OrderedCltns are ordered by the sequence in which objects are added and removed
from them.  Object elements are accessible by index.

$Log:	OrderedCltn.c,v $
 * Revision 1.2  88/01/16  23:40:13  keith
 * Remove pre-RCS modification history
 * 
*/

#include <libc.h>
#include "OrderedCltn.h"
#include "oopsIO.h"

#define	THIS	OrderedCltn
#define	BASE	SeqCltn
DEFINE_CLASS(OrderedCltn,SeqCltn,1,"$Header: OrderedCltn.c,v 1.2 88/01/16 23:40:13 keith Exp $",NULL,NULL);

extern const int OOPS_CLTNEMPTY,OOPS_OBNOTFOUND;

OrderedCltn::OrderedCltn(unsigned size) : contents(size)
{
	endIndex = 0;
}

OrderedCltn::OrderedCltn(const OrderedCltn& c) : contents(c.contents)
{
	endIndex = c.endIndex;
}

void OrderedCltn::operator=(const OrderedCltn& c)
{
	endIndex = c.endIndex;
	contents = c.contents;
}

bool OrderedCltn::operator==(const OrderedCltn& a)
{
	if (endIndex != a.endIndex) return NO;
	else {
		register int i = endIndex;
		register Object** vv = &contents.elem(0);
		register Object** av = &a.at(0);
		while (i--) if (!((*vv++)->isEqual(**av++))) return NO;
	}
	return YES;
}

OrderedCltn OrderedCltn::operator&(const SeqCltn& cltn)
{
	OrderedCltn c(size()+cltn.size());
	addContentsTo(c);
	cltn.addContentsTo(c);
	return c;
}

void OrderedCltn::operator&=(const SeqCltn& cltn)
{
	cltn.addContentsTo(*this);
}

Object* OrderedCltn::addAtIndex(int i, const Object& ob)
{
	if (endIndex == capacity()) contents.reSize(capacity()+CLTN_EXPANSION_INCREMENT);
	for (register int j=endIndex; j>i; j--) contents[j] = contents[j-1];
	contents[i] = &ob;
	endIndex++;
	return &ob;
}

Object* OrderedCltn::removeAtIndex(int i)
{
	register Object* obrem = contents[i];
	for (register int j=i+1; j<endIndex; j++) contents[j-1] = contents[j];
	if (endIndex < capacity()) contents[endIndex--] = nil;
	return obrem;
}

Object* OrderedCltn::add(const Object& ob)
{
	addAtIndex(endIndex,ob);
	return &ob;
}

Object* OrderedCltn::addAfter(const Object& ob, const Object& newob)
{
	register int i = indexOf(ob);
	if (i < 0) errNotFound("addAfter",ob);
	return addAtIndex(i+1,newob);
}

Object* OrderedCltn::addAllLast(const OrderedCltn& cltn)
{
	if (endIndex+cltn.size() >= capacity())
		contents.reSize(endIndex+cltn.size()+CLTN_EXPANSION_INCREMENT);
	for (register int i=0; i<cltn.size(); i++) contents[endIndex++] = cltn.contents[i];
	return &cltn;
}

Object* OrderedCltn::addBefore(const Object& ob, const Object& newob)
{
	register int i = indexOf(ob);
	if (i < 0) errNotFound("addBefore",ob);
	return addAtIndex(i,newob);
}

Collection& OrderedCltn::addContentsTo(Collection& cltn)
{
	for (register int i=0; i<size(); i++) cltn.add(*contents[i]);
	return cltn;
}

Object* OrderedCltn::addLast(const Object& ob) { return add(ob); }

Object* OrderedCltn::after(const Object& ob)
{
	register int i=indexOf(ob);
	if (i<0) errNotFound("after",ob);
	if (++i == endIndex) return nil;
	return contents[i];
}

Object*& OrderedCltn::at(int i)	{ return (*this)[i]; }

void OrderedCltn::atAllPut(const Object& ob)
{
	for (register int i=0; i<endIndex; i++) contents[i] = &ob;
}

Object* OrderedCltn::before(const Object& ob)
{
	register int i = indexOf(ob);
	if (i < 0) errNotFound("before",ob);
	if (--i < 0) return nil;
	return contents[i];
}

unsigned OrderedCltn::capacity()	{ return contents.capacity(); }

void OrderedCltn::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	contents.deepenShallowCopy();
}

Object* OrderedCltn::first()
{
	if (endIndex==0) errEmpty("first");
	else return contents.elem(0);
}

unsigned OrderedCltn::hash()
{
	register unsigned h = endIndex;
	register int i = endIndex;
	register Object** vv = &contents.elem(0);
	while (i--) h^=(*vv++)->hash();
	return h;
}

int OrderedCltn::indexOf(const Object& ob)
{
	for (register int i=0; i<endIndex; i++)
		if (contents[i]->isEqual(ob)) return i;
	return -1;
}

int OrderedCltn::indexOfSubCollection(const SeqCltn& cltn, int start)
{
	int subsize = cltn.size();
	for (register int i=start; i<(endIndex-subsize); i++) {
		for (register int j=0; j<subsize; j++)
			if (!(contents[i+j]->isEqual(*cltn.at(j)))) goto next;
		return i;
next:;	}
	return -1;
}

bool OrderedCltn::isEmpty()	{ return endIndex==0; }
	
bool OrderedCltn::isEqual(const Object& a)
{
	return a.isSpecies(class_OrderedCltn) && *this==*(OrderedCltn*)&a;
}

const Class* OrderedCltn::species()	{ return &class_OrderedCltn; }

Object* OrderedCltn::last()
{
	if (endIndex==0) errEmpty("last");
	else return contents.elem(endIndex-1);
}

unsigned OrderedCltn::occurrencesOf(const Object& ob)
{
	register unsigned n=0;
	for (register int i=0; i<endIndex; i++)
		if (contents[i]->isEqual(ob)) n++;
	return n;
}

void OrderedCltn::printOn(ostream& strm)
{
	strm << className() << "[\n";
	for (register int i=0; i<endIndex; i++) {
		if (i>0) strm << "\n";
		contents[i]->printOn(strm);
	}
	strm << "]\n";
}

Object* OrderedCltn::remove(const Object& ob)
{
	for (register int i=0; i<endIndex; i++) {
		if (contents[i]->isEqual(ob)) {
			return removeAtIndex(i);
		}
	}
	return nil;
}

Object* OrderedCltn::removeId(const Object& ob)
{
	for (register int i=0; i<endIndex; i++) {
		if (contents[i] == &ob) {
			removeAtIndex(i);
			return &ob;
		}
	}
	return nil;
}

Object* OrderedCltn::removeLast()
{
	if (endIndex==0) errEmpty("removeLast");
	else return removeAtIndex(endIndex-1);
}

void OrderedCltn::reSize(unsigned newSize)
{
	if (newSize > size()) contents.reSize(newSize);
}

void OrderedCltn::replaceFrom(int start, int stop, const SeqCltn& replacement, int startAt)
{
	register int j=startAt;
	for (register int i=start; i<=stop; i++,j++)
		contents[i] = replacement.at(j);
}

static int compare_ob(Object** a, Object** b) { return (*a)->compare(**b); }

unsigned OrderedCltn::size()		{ return endIndex; }

void OrderedCltn::sort()
{
     qsort(&contents.elem(0),size(),sizeof(Object*),compare_ob);
}

void OrderedCltn::errEmpty(const char* fn)
{
	setOOPSerror(OOPS_CLTNEMPTY,DEFAULT,this,className(),fn);
}

void OrderedCltn::errNotFound(const char* fn, const Object& ob)
{
	setOOPSerror(OOPS_OBNOTFOUND,DEFAULT,this,className(),fn,ob.className(),&ob);
}

OrderedCltn Collection::asOrderedCltn()
{
	OrderedCltn cltn(MAX(size(),CLTN_DEFAULT_CAPACITY));
	addContentsTo(cltn);
	return cltn;
}

static unsigned orderedcltn_capacity;

OrderedCltn::OrderedCltn(istream& strm, OrderedCltn& where) :
	contents((strm >> orderedcltn_capacity, orderedcltn_capacity))
{
	this = &where;
	endIndex = 0;
	unsigned n;
	strm >> n;		// read collection capacity 
	while (n--) add(*readFrom(strm));
}

OrderedCltn::OrderedCltn(fileDescTy& fd, OrderedCltn& where) :
	contents((readBin(fd,orderedcltn_capacity),orderedcltn_capacity))
{
	this = &where;
	endIndex = 0;
	unsigned n;
	readBin(fd,n);
	while (n--) add(*readFrom(fd));
}
