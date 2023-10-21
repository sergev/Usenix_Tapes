/* Dictionary.c -- implementation of Set of Associations

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
	
A Dictionary is a Set of Associations.  A Dictionary returns the value
of an association given its key.

$Log:	Dictionary.c,v $
 * Revision 1.2  88/01/16  23:38:45  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Dictionary.h"
#include "LookupKey.h"
#include "Assoc.h"

#define	THIS	Dictionary
#define	BASE	Set
DEFINE_CLASS(Dictionary,Set,1,"$Header: Dictionary.c,v 1.2 88/01/16 23:38:45 keith Exp $",NULL,NULL);

extern const int OOPS_DUPKEY,OOPS_KEYNOTFOUND;

Dictionary::Dictionary(unsigned size) : (size) {}

Dictionary::Dictionary(const Dictionary& d) : (d) {}

void Dictionary::operator=(const Dictionary& d)
{
	this->Set::operator=(d);
}

bool Dictionary::operator==(const Dictionary& d)
{
	if (size() != d.size()) return NO;
	DO(*this,LookupKey*,a)
		if (!d.includesAssoc(*a)) return NO;
	OD
	return YES;
}

Object* Dictionary::add(const Object& ob)
{
	assertArgClass(ob,class_LookupKey,"add");
	return Set::add(ob);
}

Assoc* Dictionary::addAssoc(const Object& key, const Object& value)
{
	Assoc* a = new Assoc(key,value);
	Assoc* b = (Assoc*)Set::add(*a);
	if (a != b) {
		delete a;
		setOOPSerror(OOPS_DUPKEY,DEFAULT,this,className(),"addAssoc",key.className(),&key);
	}
	return b;
}

Collection& Dictionary::addContentsTo(Collection& cltn)
{
	DO(*this,LookupKey*,a) cltn.add(*(a->value())); OD
	return cltn;
}

Object* Dictionary::remove(const Object& ob)
{
	assertArgClass(ob,class_LookupKey,"remove");
	return Set::remove(ob);
}

Object* Dictionary::atKey(const Object& key)
{
	register Object* p = findObjectWithKey(key);
	if (p==nil) setOOPSerror(OOPS_KEYNOTFOUND,DEFAULT,this,className(),key.className(),&key);
	else return ((LookupKey*)p)->value();
}

Object* Dictionary::atKey(const Object& key, const Object& newValue)
{
	register Object* p = findObjectWithKey(key);
	if (p==nil) setOOPSerror(OOPS_KEYNOTFOUND,DEFAULT,this,className(),key.className(),&key);
	else return ((LookupKey*)p)->value(newValue);
}

LookupKey& Dictionary::assocAt(const Object& key)
{
	return *(LookupKey*)findObjectWithKey(key);
}

Collection& Dictionary::addKeysTo(Collection& cltn)
{
	DO(*this,LookupKey*,a) cltn.add(*a->key()); OD
	return cltn;
}

Collection& Dictionary::addValuesTo(Collection& cltn)
{
	return addContentsTo(cltn);
}

Object* Dictionary::keyAtValue(const Object& val)
{
	DO(*this,LookupKey*,a)
		if (val.isEqual(*a->value())) return a->key();
	OD
	return nil;
}

unsigned Dictionary::occurrencesOf(const Object& val)
{
	register unsigned n =0;
	DO(*this,LookupKey*,a) if (val.isEqual(*a->value())) n++; OD
	return n;
}

bool Dictionary::includesAssoc(const LookupKey& asc)
{
	register Object* p = findObjectWithKey(asc);
	if (p==nil) return NO;
	return (asc.value())->isEqual(*((LookupKey*)p)->value());
}

bool Dictionary::includesKey(const Object& key)
{
	if (findObjectWithKey(key) == nil) return NO;
	else return YES;
}

bool Dictionary::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Dictionary) && *this==*(Dictionary*)&ob;
}

const Class* Dictionary::species()	{ return &class_Dictionary; }

LookupKey& Dictionary::removeAssoc(const LookupKey& asc)
{
	return *(LookupKey*)remove(asc);
}

LookupKey& Dictionary::removeKey(const Object& key)
{
	return removeAssoc(assocAt(key));
}

Dictionary::Dictionary(fileDescTy& fd, Dictionary& where) : (fd,where) {}

Dictionary::Dictionary(istream& strm, Dictionary& where) : (strm,where) {}
