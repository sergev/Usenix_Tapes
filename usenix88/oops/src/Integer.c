/* Integer.c -- implementation of Integer object

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
	
Provides an object that contains an int.

$Log:	Integer.c,v $
 * Revision 1.2  88/01/16  23:39:18  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Integer.h"
#include "oopsIO.h"

#define	THIS	Integer
#define	BASE	Object
DEFINE_CLASS(Integer,Object,1,"$Header: Integer.c,v 1.2 88/01/16 23:39:18 keith Exp $",NULL,NULL);

Integer::Integer(istream& strm)		{ parseInteger(strm); }

unsigned Integer::hash()	{ return val; }

bool Integer::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Integer) && val==((Integer*)&ob)->val;
}

const Class* Integer::species()	{ return &class_Integer; }

int Integer::compare(const Object& ob)
{
	assertArgSpecies(ob,class_Integer,"compare");
	return val-((Integer*)&ob)->val;
}

Object* Integer::copy()		{ return shallowCopy(); }

void Integer::deepenShallowCopy()	{}

void Integer::printOn(ostream& strm)
{
	strm << val;
}

void Integer::scanFrom(istream& strm)	{ parseInteger(strm); }

Integer::Integer(istream& strm, Integer& where)
{
	this = &where;
	strm >> val;
}

void Integer::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << val << " ";
}

Integer::Integer(fileDescTy& fd, Integer& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Integer::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}
