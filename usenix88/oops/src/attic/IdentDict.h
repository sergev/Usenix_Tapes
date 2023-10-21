#ifndef IDENTDICTH
#define IDENTDICTH 

#include "Dictionary.h"

extern Class class_IdentDict;

class LookupKey;
class OrderedCltn;

class IdentDict: public Dictionary {
public:
	IdentDict(int size =CLTN_DEFAULT_CAPACITY);
	IdentDict(IdentDict&);
	IdentDict(istream&,IdentDict&);
	void operator=(IdentDict&);
	virtual	obid add(Object&);
	virtual Collection& addKeysTo(Collection&);
	virtual LookupKey& assocAt(Object& key);
	virtual obid atKey(Object& key);
	virtual obid atKey(Object& key, Object& newValue);
	virtual bool includesAssoc(LookupKey& asc);
	virtual bool includesKey(Object& key);
	virtual Class* isA();
	virtual obid keyAtValue(Object& val);
	virtual obid remove(Object& asc);
	virtual LookupKey& removeAssoc(LookupKey& asc);
};

#endif
