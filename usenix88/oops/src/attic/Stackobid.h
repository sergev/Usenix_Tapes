#ifndef STACKOBIDH
#define STACKOBIDH

#include "Arrayobid.h"

extern Class class_Stackobid;

class Stackobid : public Arrayobid {
	int t;
	void OverflowErr();
	void UnderflowErr();
	void EmptyErr();
public:
	Stackobid(int size=CLTN_DEFAULT_CAPACITY) : (size) { isA(class_Stackobid); t = 0; }
	Stackobid(Stackobid& a) : ((Arrayobid&)a)
		{ isA(class_Stackobid); t = a.t; }
	bool operator==(Stackobid&);
	bool operator!=(Stackobid& a)	{ return !(*this==a); }
	void push(obid& a)
		{ if (t==capacity()) OverflowErr(); elem(t++) = a; }
	obid pop()
		{ if (t==0) UnderflowErr(); return elem(--t); }
	obid& top()
		{ if (t==0) EmptyErr(); return elem(t-1); }
	virtual obid& at(int i)	{ return this->operator[](t-i-1); }
	virtual int hash();
	virtual bool isEqual(Object&);
	virtual void printOn(ostream& strm);
	virtual int size()	{ return t; }
};

#endif
