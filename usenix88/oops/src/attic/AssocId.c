/* AssocId.c -- member functions of class AssocId

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: {decvax!}seismo!elsie!cecil!keith
	October, 1985

Function:

Objects of class AssocId associate the ID of a key object with a value object.

Modification History:
	
*/

#include "AssocId.h"

#define	THIS	AssocId
#define	BASE	LookupId
DEFINE_CLASS(AssocId,LookupId,1,NULL);

AssocId::AssocId(Object& newKey, Object& newValue) : (newKey)
{
	avalue = &newValue;
}

AssocId::AssocId(Object& newKey) : (newKey)
{
	avalue = nil;
}

AssocId::AssocId() : (*nil)
{
	avalue = nil;
}

obid AssocId::value()	{ return avalue; }

obid AssocId::value(Object& newvalue)
{
	obid temp = avalue;
	avalue = &newvalue;
	return temp;
}

void AssocId::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	avalue = avalue->deepCopy();
}

void AssocId::printOn(ostream& strm)
{
	strm << className() << "("; LookupKey::printOn(strm);
	strm << "="; avalue->printOn(strm); strm << ")";
}

static void AssocId_reader(istream& strm, Object& where)
{
	AssocId* temp = new AssocId(strm,*(AssocId*)&where);
}

AssocId::AssocId(istream& strm, AssocId& where) : (strm)
{
	this = &where;
	avalue = readFrom(strm);
}

void AssocId::storer(ostream& strm)
{
	BASE::storer(strm);
	avalue->storeOn(strm);
}
