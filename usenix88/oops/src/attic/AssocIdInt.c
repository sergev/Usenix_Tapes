/* AssocIdInt.c -- member functions of class AssocIdInt

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

Objects of class AssocIdInt associate a key ObjectId with an Integer value
object.  This class is used by the storeOn function to convert object ID's
to integers.

Modification History:
	
*/

#include "AssocIdInt.h"

#define	THIS	AssocIdInt
#define	BASE	LookupId
DEFINE_CLASS(AssocIdInt,LookupId,1,NULL);

AssocIdInt::AssocIdInt(Object& newKey, int newValue) : (newKey), avalue(newValue) {}

AssocIdInt::AssocIdInt(Object& newKey) : (newKey), avalue(0) {}

AssocIdInt::AssocIdInt() : (*nil), avalue(0) {}

obid AssocIdInt::value()	{ return (obid)&avalue; }

obid AssocIdInt::value(Object& newValue)
{
	ASSERT_CLASS(newValue,class_Integer,"value");
	avalue = *(Integer*)&newValue;
	return &avalue;
}

void AssocIdInt::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	avalue.deepenShallowCopy();
}

void AssocIdInt::printOn(ostream& strm)
{
	strm << className() << "("; LookupKey::printOn(strm);
	strm << "="; avalue.printOn(strm); strm << ")";
}

static void AssocIdInt_reader(istream& strm, Object& where)
{
	AssocIdInt* temp = new AssocIdInt(strm,*(AssocIdInt*)&where);
}

static int intval;

AssocIdInt::AssocIdInt(istream& strm, AssocIdInt& where) : (strm), avalue((strm >> intval, intval))
{
	this = &where;
}

void AssocIdInt::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << avalue.value();
}
