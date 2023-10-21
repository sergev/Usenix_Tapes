#ifndef FORMH
#define FORMH

#include "Object.h"
#include "Dictionary.h"
#include "OrderedCltn.h"
#include "Field.h"

extern Class class_Form;

class Form : public Object {
	Dictionary fieldNames;
	OrderedCltn fields;
public:
	Form() {};
	Form(istream&, Form&);
	Form(fileDescTy&, Form&) {}
	virtual const Class* isA();
	virtual void addField(const Field&);
	virtual void display();
	virtual void fillIn();
	virtual void storer(fileDescTy&) {};
	virtual void storer(ostream&);
};

#endif
