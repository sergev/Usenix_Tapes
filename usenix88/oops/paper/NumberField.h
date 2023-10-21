#ifndef NUMBERFIELDH
#define NUMBERFIELDH

#include "Field.h"

extern Class class_NumberField;

class NumberField: public Field {
	int value;
public:
	NumberField(const char*, const Rectangle&, int);
	NumberField(istream&, NumberField&);
	NumberField(fileDescTy& fd, NumberField& where) : (fd,where) {}
	virtual const Class* isA();
	virtual void display();
	virtual void fillIn();
	virtual void storer(fileDescTy&) {}
	virtual void storer(ostream&);
};

#endif
