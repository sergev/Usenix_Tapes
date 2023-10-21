/* IdentDict.c -- member functions of class IdentDict

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
	
An IdentDict is a Dictionary which associates the ID of the key with an
object.

Modification History:
	
7-Jan-86	K. E. Gorlen

1.  Delete IdentDict::isEqual.

*/

#include "IdentDict.h"
#include "LookupKey.h"
#include "LookupId.h"
#include "ObjectId.h"

#define	THIS	IdentDict
#define	BASE	Dictionary
DEFINE_CLASS(IdentDict,Dictionary,1,NULL);

IdentDict::IdentDict(int size) : (size) {}

IdentDict::IdentDict(IdentDict& d) : (d) {}

void IdentDict::operator=(IdentDict& d)
{
	this->Dictionary::operator=(d);
}

obid IdentDict::add(Object& ob)
{
	ASSERT_CLASS(ob,class_LookupId,"add");
	return Dictionary::add(ob);
}

obid IdentDict::remove(Object& ob)
{
	ASSERT_CLASS(ob,class_LookupId,"remove");
	return Dictionary::remove(ob);
}

obid IdentDict::atKey(Object& key)
{
	return Dictionary::atKey(ObjectId(key));
}

obid IdentDict::atKey(Object& key, Object& newValue)
{
	return Dictionary::atKey(ObjectId(key), newValue);
}

LookupKey& IdentDict::assocAt(Object& key)
{
	return Dictionary::assocAt(ObjectId(key));
}

Collection& IdentDict::addKeysTo(Collection& cltn)
{
	DO(*this,LookupId*,a) cltn.add(*((ObjectId*)a->key())->value()); OD
	return cltn;
}

obid IdentDict::keyAtValue(Object& val)
{
	obid key = Dictionary::keyAtValue(val);
	if (key != nil) key = ((ObjectId*)key)->value();
	return key;
}

bool IdentDict::includesAssoc(LookupKey& asc)
{
	ASSERT_CLASS(asc,class_LookupId,"includesAssoc");
	return Dictionary::includesAssoc(asc);
}

bool IdentDict::includesKey(Object& key)
{
	return Dictionary::includesKey(ObjectId(key));
}

LookupKey& IdentDict::removeAssoc(LookupKey& asc)
{
	ASSERT_CLASS(asc,class_LookupId,"removeAssoc");
	return Dictionary::removeAssoc(asc);
}

static void IdentDict_reader(istream& strm, Object& where)
{
	IdentDict* temp = new IdentDict(strm,*(IdentDict*)&where);
}

IdentDict::IdentDict(istream& strm, IdentDict& where) : (strm,where) {}
