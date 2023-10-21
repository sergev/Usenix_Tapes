#ifndef	DICTIONARY_H
#define	DICTIONARY_H

/* Dictionary.h -- declarations for Set of Associations

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

$Log:	Dictionary.h,v $
 * Revision 1.3  88/02/04  12:58:53  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:38:48  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Set.h"

extern const Class class_Dictionary;
overload Dictionary_reader;

class LookupKey;
class Assoc;
class OrderedCltn;

class Dictionary: public Set {
protected:
	Dictionary(fileDescTy&,Dictionary&);
	Dictionary(istream&,Dictionary&);
	friend	void Dictionary_reader(istream& strm, Object& where);
	friend	void Dictionary_reader(fileDescTy& fd, Object& where);
public:
	Dictionary(unsigned size =CLTN_DEFAULT_CAPACITY);
	Dictionary(const Dictionary&);
	void operator=(const Dictionary&);
	bool operator==(const Dictionary&);
	bool operator!=(const Dictionary& d)	{ return !(*this == d); }
	virtual	Object* add(const Object&);
	virtual Assoc* addAssoc(const Object& key, const Object& value);
	virtual Collection& addContentsTo(Collection&);
	virtual Collection& addKeysTo(Collection&);
	virtual Collection& addValuesTo(Collection&);
	virtual LookupKey& assocAt(const Object& key);
	virtual Object* atKey(const Object& key);
	virtual Object* atKey(const Object& key, const Object& newValue);
	virtual bool includesAssoc(const LookupKey& asc);
	virtual bool includesKey(const Object& key);
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual Object* keyAtValue(const Object& val);
	virtual unsigned occurrencesOf(const Object& val);
	virtual Object* remove(const Object& asc);
	virtual LookupKey& removeAssoc(const LookupKey& asc);
	virtual LookupKey& removeKey(const Object& key);
	virtual const Class* species();
};

#endif
