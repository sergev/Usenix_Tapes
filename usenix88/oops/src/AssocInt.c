/* AssocInt.c -- implementation of key-Integer association

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

Objects of class AssocInt associate a key object with an Integer value
object.  They are used to implement Bags, which use a Dictionary to
associate objects with their occurrence counts.

$Log:	AssocInt.c,v $
 * Revision 1.2  88/01/16  23:37:32  keith
 * Remove pre-RCS modification history
 * 
*/

#include "AssocInt.h"
#include "oopsIO.h"

#define	THIS	AssocInt
#define	BASE	LookupKey
DEFINE_CLASS(AssocInt,LookupKey,1,"$Header: AssocInt.c,v 1.2 88/01/16 23:37:32 keith Exp $",NULL,NULL);

AssocInt::AssocInt(const Object& newKey, int newValue) : (newKey), avalue(newValue) {}

Object* AssocInt::value()	{ return (Object*)&avalue; }

Object* AssocInt::value(const Object& newValue)
{
	assertArgClass(newValue,class_Integer,"value");
	avalue = *(Integer*)&newValue;
	return &avalue;
}

void AssocInt::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	avalue.deepenShallowCopy();
}

void AssocInt::printOn(ostream& strm)
{
	strm << className() << "("; BASE::printOn(strm);
	strm << "="; avalue.printOn(strm); strm << ")";
}

static int intval;

AssocInt::AssocInt(istream& strm, AssocInt& where) : (strm,where), avalue((strm >> intval, intval))
{
	this = &where;
}

void AssocInt::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << avalue.value() << " ";
}

AssocInt::AssocInt(fileDescTy& fd, AssocInt& where) : (fd,where) , avalue((readBin(fd,intval),intval))
{
	this = &where;
}

void AssocInt::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	storeBin(fd,avalue.value());
}
