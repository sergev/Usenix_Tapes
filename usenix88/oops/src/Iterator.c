/* Iterator.c -- Implementation of Collection Iterators

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
	December, 1987

Function:

Instances of class Iterator are used to iterate over (i.e. sequence
through) the objects contained in a Collection.  For example:

	OrderedCltn c;
	c.add(*new Point(0,0));
	c.add(*new Point(1,1));
//	...
	Iterator i(c);
	Object* p;
	while (p = i++) cout << *p;

will print all the Point objects in the OrderedCltn c in the same
order in which they were added.
	
Iterators may be used on any derived class of Collection, and several
Iterators may be active on the same Collection at the same time
without interference.

Iterator is an OOPS class and implements the usual Object
functionality with the following restriction: deepCopy() and
storeOn() work only for Iterators bound to classes derived from
SeqCltn.

$Log:	Iterator.c,v $
 * Revision 1.2  88/01/16  23:39:22  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Iterator.h"
#include "Collection.h"
#include "SeqCltn.h"
#include "oopsIO.h"

#define	THIS	Iterator
#define	BASE	Object
DEFINE_CLASS(Iterator,Object,1,"$Header: Iterator.c,v 1.2 88/01/16 23:39:22 keith Exp $",NULL,NULL);

Iterator::Iterator(const Collection& c)
{
	cltn = &c;
	cltn->doReset(*this);
}

void Iterator::reset()
{
	cltn->doReset(*this);
}

Object* Iterator::operator++()
{
	return ptr = cltn->doNext(*this);
}

bool Iterator::operator==(const Iterator& a)
{
	return cltn->isSame(*a.cltn) && index == a.index && num == a.num;
}

const Class* Iterator::species()
{
	return &class_Iterator;
}

bool Iterator::isEqual(const Object& p)
{
	return p.isSpecies(class_Iterator) && *this==*(Iterator*)&p;
}

unsigned Iterator::hash()
{
	return (unsigned)cltn ^ index ^ num;
}

Object* Iterator::copy()		{ return shallowCopy(); }

void Iterator::deepenShallowCopy()
{
// Can only deepCopy() Iterators over SeqCltns
// -- not Sets, Bags, Dictionaries, etc.
	assertClass(*cltn,class_SeqCltn);
	cltn = (Collection*)cltn->deepCopy();
	if (index != 0) ptr = cltn->at(index-1);
}

Object* Iterator::shallowCopy()
{
// Can only shallowCopy() Iterators over SeqCltns
// -- not Sets, Bags, Dictionaries, etc.
	assertClass(*cltn,class_SeqCltn);
	Iterator* temp = (Iterator*)Object::shallowCopy();
	temp->cltn = (Collection*)temp->cltn->copy();
	if (index != 0) temp->ptr = temp->cltn->at(index-1);
	return temp;
}

void Iterator::printOn(ostream& strm)
{
	strm << className() << "(" << cltn->className()
		<< "[" << index << "]#" << num << ")";
}

Iterator::Iterator(istream& strm, Iterator& where)
{
	this = &where;
	strm >> index >> num;
	cltn = (Collection*)readFrom(strm);
	assertClass(*cltn,class_SeqCltn);
	if (index != 0) ptr = cltn->at(index-1);
}

void Iterator::storer(ostream& strm)
{
// Can only storeOn() Iterators over SeqCltns
// -- not Sets, Bags, Dictionaries, etc.
	assertClass(*cltn,class_SeqCltn);
	BASE::storer(strm);
	strm << index << " " << num;
	cltn->storeOn(strm);
}

Iterator::Iterator(fileDescTy& fd, Iterator& where)
{
	this = &where;
	readBin(fd,index);
	readBin(fd,num);
	cltn = (Collection*)readFrom(fd);
	assertClass(*cltn,class_Collection);
	if (index != 0) ptr = cltn->at(index-1);
}

void Iterator::storer(fileDescTy& fd) 
{
// Can only storeOn() Iterators over SeqCltns
// -- not Sets, Bags, Dictionaries, etc.
	assertClass(*cltn,class_SeqCltn);
	BASE::storer(fd);
	storeBin(fd,index);
	storeBin(fd,num);
	cltn->storeOn(fd);
}
