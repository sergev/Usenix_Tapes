/* Collection.c -- implementation of abstract Collection class

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
	
Collection is an abstract class that actually implements only the
addAll, removeAll, includes, isEmpty, and Collection.conversion
functions.  Note that the functions Collection::asBag, asOrderedCltn,
asSet, and asSortedCltn are defined in the file that implements the
respective target Collection so that all of these classes are not loaded
whenever any one Collection is used.

$Log:	Collection.c,v $
 * Revision 1.2  88/01/16  23:38:06  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Collection.h"
#include "ArrayOb.h"
#include "oopsIO.h"

#define	THIS	Collection
#define	BASE	Object
DEFINE_CLASS(Collection,Object,1,"$Header: Collection.c,v 1.2 88/01/16 23:38:06 keith Exp $",NULL,NULL);

Object* Collection::add(const Object& ob) { derivedClassResponsibility("add"); return &ob; }

const Collection& Collection::addAll(const Collection& c)
{
	c.addContentsTo(*this);
	return c;
}

Collection& Collection::addContentsTo(Collection& ob) { derivedClassResponsibility("addContentsTo"); return ob; }

Object* Collection::remove(const Object& ob) { derivedClassResponsibility("remove"); return &ob; }

const Collection& Collection::removeAll(const Collection& c)
{
	DO(c,Object*,o) remove(*o); OD
	return c;
}

void Collection::deepenShallowCopy() {}

Object*& Collection::at(int /*index*/)
{
	static Object* dummy;
	derivedClassResponsibility("at");
	return dummy;
}
	
bool Collection::includes(const Object& ob) { return occurrencesOf(ob) != 0; }

bool Collection::isEmpty() { return size() == 0; }

unsigned Collection::occurrencesOf(const Object&) { derivedClassResponsibility("occurrencesOf"); return 0; }

ArrayOb Collection::asArrayOb()
{
	ArrayOb a(size());
	register Object** q = &(a.elem(0));
	DO(*this,Object*,o) *q++ = o; OD
	return a;
}
 
Object* Collection::doNext(Iterator& /*pos*/)
	{ derivedClassResponsibility("doNext"); return 0; }

void Collection::doReset(Iterator& pos)
{
	pos.index = 0;
	pos.ptr = (Object*)NULL;
	pos.num = 0;
}

Object* Collection::shallowCopy()	{ shouldNotImplement("shallowCopy"); return 0; }
	
unsigned Collection::size() { derivedClassResponsibility("size"); return 0; }

void Collection::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << capacity() << " " << size();
	DO(*this,Object*,ob) ob->storeOn(strm); OD
}

void Collection::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	storeBin(fd,capacity());
	storeBin(fd,size());
	DO(*this,Object*,ob) ob->storeOn(fd); OD
}
