/* LinkOb.c -- implementation of singly-linked list Object pointer element

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
	October, 1985

Function:

Objects of class LinkOb are used to link Objects into a LinkedList.

$Log:	LinkOb.c,v $
 * Revision 1.2  88/01/16  23:39:31  keith
 * Remove pre-RCS modification history
 * 
*/

#include "LinkOb.h"
#include "oopsIO.h"

#define	THIS	LinkOb
#define	BASE	Link
DEFINE_CLASS(LinkOb,Link,1,"$Header: LinkOb.c,v 1.2 88/01/16 23:39:31 keith Exp $",NULL,NULL);

LinkOb::LinkOb(const Object& newval)
{
	val = &newval;
}

unsigned LinkOb::capacity()		{ return val->capacity(); }

int LinkOb::compare(const Object& ob)	{ return ob.compare(*val); }

void LinkOb::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	val = val->deepCopy();
}

unsigned LinkOb::hash()			{ return val->hash(); }

bool LinkOb::isEqual(const Object& ob)	{ return ob.isEqual(*val); }

void LinkOb::printOn(ostream& strm)
{
	strm << className() << "("; val->printOn(strm); strm << ")";
}

unsigned LinkOb::size()			{ return val->size(); }

Object* LinkOb::value()	{ return val; }

Object* LinkOb::value(const Object& newval)
{
	Object* temp = val;
	val = &newval;
	return temp;
}

LinkOb::LinkOb(istream& strm, LinkOb& where)
{
	this = &where;
	val = readFrom(strm);
}

void LinkOb::storer(ostream& strm)
{
	BASE::storer(strm);
	val->storeOn(strm);
}

LinkOb::LinkOb(fileDescTy& fd, LinkOb& where)
{
	this = &where;
	val = readFrom(fd);
}

void LinkOb::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	val->storeOn(fd);
}
