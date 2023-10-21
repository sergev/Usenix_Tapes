#ifndef ASSOCIDINTH
#define ASSOCIDINTH 

#include "LookupId.h"
#include "Integer.h"

extern Class class_AssocIdInt;

class AssocIdInt: public LookupId {
	Integer avalue;
public:
	AssocIdInt(Object& newKey/*=*nil*/, int newValue/*=0*/);
	AssocIdInt(Object& newKey/*=*nil*/);
	AssocIdInt();
	AssocIdInt(istream&,AssocIdInt&);
	virtual void deepenShallowCopy();
	virtual Class* isA();
	virtual void printOn(ostream& strm);
	virtual void storer(ostream&);
	virtual obid value();
	virtual obid value(Object& newValue);
};

#endif
