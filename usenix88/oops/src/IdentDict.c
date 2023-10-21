/* IdentDict.c -- implementation of Identifier Dictionary

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
	
An IdentDict is like a Dictionary, except keys are compared using
isSame() rather than isEqual().

$Log:	IdentDict.c,v $
 * Revision 1.2  88/01/16  23:39:13  keith
 * Remove pre-RCS modification history
 * 
*/

#include "IdentDict.h"
#include "LookupKey.h"

#define	THIS	IdentDict
#define	BASE	Dictionary
DEFINE_CLASS(IdentDict,Dictionary,1,"$Header: IdentDict.c,v 1.2 88/01/16 23:39:13 keith Exp $",NULL,NULL);

IdentDict::IdentDict(unsigned size) : (size) {}

IdentDict::IdentDict(const IdentDict& d) : (d) {}

void IdentDict::operator=(const IdentDict& d)
{
	this->Dictionary::operator=(d);
}

int IdentDict::findIndexOf(const Object& ob)
/*
Search this IdentDict for a LookupKey with the same key object as the
argument.

Enter:
	ob = pointer to LookupKey to search for

Returns:
	index of object if found or of nil slot if not found
	
Algorithm L, Knuth Vol. 3, p. 519
*/
{
	register int i;
	Object* keyob = ((LookupKey*)&ob)->key();
	for (i = h((int)keyob); contents[i]!=nil; i = (i-1)&mask) {
		if (((LookupKey*)contents[i])->key()->isSame(*keyob)) return i;
	}
	return i;
}

Object* IdentDict::atKey(const Object& key)
{
	return Dictionary::atKey(LookupKey(key));
}

Object* IdentDict::atKey(const Object& key, const Object& newValue)
{
	return Dictionary::atKey(LookupKey(key), newValue);
}

LookupKey& IdentDict::assocAt(const Object& key)
{
	return Dictionary::assocAt(LookupKey(key));
}

bool IdentDict::includesKey(const Object& key)
{
	return Dictionary::includesKey(LookupKey(key));
}

IdentDict::IdentDict(istream& strm, IdentDict& where) : (strm,where) {}

IdentDict::IdentDict(fileDescTy& fd, IdentDict& where) : (fd,where)  {}
