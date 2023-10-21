/* LookupId.c -- member functions of class LookupId

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

Objects of class LookupId allow an ObjectId to be the key of an
association.  They are used to implement class IdentDict.

Modification History:
	
*/

#include "LookupId.h"

#define	THIS	LookupId
#define	BASE	LookupKey
DEFINE_CLASS(LookupId,LookupKey,1,NULL);

void LookupId::deepenShallowCopy()
{
/* The key of a LookupId object is not deepCopied when a deepCopy is done */
	Object::deepenShallowCopy();
}

static void LookupId_reader(istream& strm, Object& where)
{
	(strm,where,seterror(OOPS_RDABSTCLASS,DEFAULT,"LookupId"));
}

LookupId::LookupId(istream& strm) : (id), id(*readFrom(strm))
{
}

void LookupId::storer(ostream& strm)
{
	Object::storer(strm);
	((ObjectId*)key())->value()->storeOn(strm);
}
