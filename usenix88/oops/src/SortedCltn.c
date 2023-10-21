/* SortedCltn.c -- implementation of sorted collection

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
	
A SortedCltn is a Collection of objects ordered as defined by the
virtual function "compare", which the objects must implement.  The "add"
function locates the position at which to insert the object by
performing a binary search, then invokes the private function
"OrderedCltn::addAtIndex" to insert the object after shifting up all the
objects after it in the array; therefore, a SortedCltn is not efficient
for a large number of objects.

$Log:	SortedCltn.c,v $
 * Revision 1.2  88/01/16  23:41:21  keith
 * Remove pre-RCS modification history
 * 
*/

#include "SortedCltn.h"
#include "oopsIO.h"

#define	THIS	SortedCltn
#define	BASE	OrderedCltn
DEFINE_CLASS(SortedCltn,OrderedCltn,1,"$Header: SortedCltn.c,v 1.2 88/01/16 23:41:21 keith Exp $",NULL,NULL);

SortedCltn::SortedCltn(unsigned size) : (size) {}

SortedCltn::SortedCltn(const SortedCltn& s) : (s) {}

void SortedCltn::operator=(const SortedCltn& s)
{
	this->OrderedCltn::operator=(s);
}

bool SortedCltn::operator==(const SortedCltn& s)
{
	return this->OrderedCltn::operator==(s);
}
	
Object* SortedCltn::add(const Object& ob)
{
	if (size()==0) {		// add first object to collection 
		this->OrderedCltn::add(ob);
		return &ob;
	}
/*
Do a binary search to determine where to insert the object.  This binary
search algorithm was adapted from Knuth, "The Art of Computer
Programming", Vol. 3, Section 6.2.1, Algorithm U.
*/
	register int i =(size()+1)>>1;
	register int m =size()>>1;
	while (YES) {
		if (i>0 && ob.compare(*contents[i-1])<0) {
			if (m==0) {
				this->OrderedCltn::addAtIndex(i-1,ob);
				return &ob;
			}
			else i -= (m+1)>>1;
		}
		else {
			if (m==0) {
				this->OrderedCltn::addAtIndex(i,ob);
				return &ob;
			}
			else i += (m+1)>>1;
		m >>= 1;
		}
	}
}
	
Object* SortedCltn::addAfter(const Object& ob, const Object& /*newob*/)
{
	shouldNotImplement("addAfter"); return &ob;
}

Object* SortedCltn::addAllLast(const OrderedCltn&)
{
	shouldNotImplement("addAllLast"); return (Object*)0;
}

Object* SortedCltn::addBefore(const Object& ob, const Object& /*newob*/)
{
	shouldNotImplement("addBefore"); return &ob;
}

Object* SortedCltn::addLast(const Object& ob)
{
	shouldNotImplement("addLast"); return &ob;
}

void SortedCltn::atAllPut(const Object&)
{
	shouldNotImplement("atAllPut");
}

int SortedCltn::indexOfSubCollection(const SeqCltn& /*cltn*/, int /*start*/)
{
	shouldNotImplement("indexOfSubCollection"); return 0;
}

SeqCltn SortedCltn::operator&(const SeqCltn& cltn)
{
	shouldNotImplement("operator&"); return cltn;
}

void SortedCltn::replaceFrom(int /*start*/, int /*stop*/, const SeqCltn& /*replacement*/, int /*startAt*/)
{
	shouldNotImplement("replaceFrom");
}

void SortedCltn::sort()
{
	shouldNotImplement("sort");
}
 
SortedCltn Collection::asSortedCltn()
{
	SortedCltn cltn(MAX(size(),CLTN_DEFAULT_CAPACITY));
	addContentsTo(cltn);
	return cltn;
}

SortedCltn::SortedCltn(istream& strm, SortedCltn& where) : (strm,where) {}

SortedCltn::SortedCltn(fileDescTy& fd, SortedCltn& where) : (fd,where) {}
