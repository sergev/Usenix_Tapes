#ifndef OBJECTIDH
#define OBJECTIDH 

#include "Object.h"

extern Class class_ObjectId;

class ObjectId: public Object {
	obid id;
public:
	ObjectId(Object& newid /*=*nil*/)	{ id = &newid; }
	ObjectId()			{ id = nil; }
	ObjectId(istream&,ObjectId&);
	obid value()			{ return id; }
	obid value(obid newid)		{ return id = newid; }
	virtual int compare(Object&);
	virtual int hash();
	virtual Class* isA();
	virtual bool isEqual(Object&);
	virtual void printOn(ostream& strm);
	virtual Class* species();
	virtual void storer(ostream&);
};

#endif
