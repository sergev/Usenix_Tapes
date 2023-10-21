#ifndef ASSOCIDH
#define ASSOCIDH 

#include "LookupId.h"

extern Class class_AssocId;

class AssocId: public LookupId {
	obid avalue;
public:
	AssocId(Object& newKey/*=*nil*/, Object& newValue/*=*nil*/);
	AssocId(Object& newKey);
	AssocId();
	AssocId(istream&,AssocId&);
	virtual void deepenShallowCopy();
	virtual Class* isA();
	virtual void printOn(ostream& strm);
	virtual void storer(ostream&);
	virtual obid value();
	virtual obid value(Object& newvalue);
};

#endif
