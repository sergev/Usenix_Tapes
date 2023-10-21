#ifndef LOOKUPIDH
#define LOOKUPIDH 

#include "LookupKey.h"
#include "ObjectId.h"

extern Class class_LookupId;

class LookupId: public LookupKey {
	ObjectId id;
public:
	LookupId(Object& newId) : (id), id(newId) {}
	LookupId(istream&);
	virtual void deepenShallowCopy();
	virtual Class* isA();
	virtual void storer(ostream&);
};

#endif
