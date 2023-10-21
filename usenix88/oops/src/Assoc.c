/* Assoc.c -- implementation of key-value association

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
	Division of Computer Reearch and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	September, 1985

Function:

Objects of class Assoc associate a key object with a value object.  They
are used to implement Dictionaries, which are Sets of Associations.

$Log:	Assoc.c,v $
 * Revision 1.2  88/01/16  23:37:27  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Assoc.h"

#define	THIS	Assoc
#define	BASE	LookupKey
DEFINE_CLASS(Assoc,LookupKey,1,"$Header: Assoc.c,v 1.2 88/01/16 23:37:27 keith Exp $",NULL,NULL);

Assoc::Assoc(const Object& newKey, const Object& newValue) : (newKey)
{
	avalue = &newValue;
}

Object* Assoc::value()	{ return avalue; }

Object* Assoc::value(const Object& newvalue)
{
	Object* temp = avalue;
	avalue = &newvalue;
	return temp;
}

void Assoc::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	avalue = avalue->deepCopy();
}

void Assoc::printOn(ostream& strm)
{
	strm << className() << "("; BASE::printOn(strm);
	strm << "="; avalue->printOn(strm); strm << ")";
}

Assoc::Assoc(istream& strm, Assoc& where) : (strm,where)
{
	this = &where;
	avalue = readFrom(strm);
}

void Assoc::storer(ostream& strm)
{
	BASE::storer(strm);
	avalue->storeOn(strm);
}

Assoc::Assoc(fileDescTy& fd, Assoc& where) : (fd,where)
{
	this = &where;
	avalue = readFrom(fd);
}

void Assoc::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	avalue->storeOn(fd);
}
