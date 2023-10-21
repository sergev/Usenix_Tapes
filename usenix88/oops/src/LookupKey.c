/* LookupKey.c -- implementation of Dictionary LookupKey

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
	
LookupKey is an abstract class for managing the key object of an Assoc.
It is used to implement class Dictionary.

$Log:	LookupKey.c,v $
 * Revision 1.2  88/01/16  23:39:40  keith
 * Remove pre-RCS modification history
 * 
*/

#include "LookupKey.h"

#define	THIS	LookupKey
#define	BASE	Object
DEFINE_CLASS(LookupKey,Object,1,"$Header: LookupKey.c,v 1.2 88/01/16 23:39:40 keith Exp $",NULL,NULL);

LookupKey::LookupKey(const Object& newKey)
{
	akey = &newKey;
}

Object* LookupKey::key()	{ return akey; }

Object* LookupKey::key(const Object& newkey)
{
	Object* temp = akey;
	akey = &newkey;
	return temp;
}

bool LookupKey::isEqual(const Object& ob) { return ob.isEqual(*akey); }

unsigned LookupKey::hash() { return akey->hash(); }

int LookupKey::compare(const Object& ob) { return ob.compare(*akey); }

void LookupKey::deepenShallowCopy()
{
	akey = akey->deepCopy();
}

void LookupKey::printOn(ostream& strm)
{
	akey->printOn(strm);
}

Object* LookupKey::value()
{
	derivedClassResponsibility("value"); return (Object*)0;
}

Object* LookupKey::value(const Object& newvalue)
{
	derivedClassResponsibility("value"); return (Object*)&newvalue;
}

LookupKey::LookupKey(istream& strm, LookupKey& /*where*/)
{
	akey = readFrom(strm);
}

void LookupKey::storer(ostream& strm)
{
	BASE::storer(strm);
	akey->storeOn(strm);
}

LookupKey::LookupKey(fileDescTy& fd, LookupKey& /*where*/)
{
	akey = readFrom(fd);
}

void LookupKey::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	akey->storeOn(fd);
}
