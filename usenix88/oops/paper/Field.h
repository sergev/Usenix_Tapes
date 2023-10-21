#ifndef FIELDH
#define FIELDH

#include "Object.h"
#include "String.h"
#include "Rectangle.h"

extern Class class_Field;

class Field: public Object {
	String* namePtr;
	Rectangle location;
public:
	Field(const char*, const Rectangle&);
	Field(istream&, Field&);
	Field(fileDescTy&, Field&) {}
	const String& name()		{ return *namePtr; }
	virtual const Class* isA();
	virtual void display();
	virtual void fillIn();
	virtual void storer(fileDescTy&) {}
	virtual void storer(ostream&);
};

#endif
