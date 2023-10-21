#ifndef ALPHAFIELDH
#define ALPHAFIELDH

#include "Field.h"

extern Class class_AlphaField;

class AlphaField: public Field {
	String value;
public:
	AlphaField(const char*, const Rectangle&, const char*);
	AlphaField(istream&, AlphaField&);
	AlphaField(fileDescTy& fd, AlphaField& where) : (fd,where) {}
	virtual const Class* isA();
	virtual void display();
	virtual void fillIn();
	virtual void storer(fileDescTy&) {}
	virtual void storer(ostream&);
};

#endif
