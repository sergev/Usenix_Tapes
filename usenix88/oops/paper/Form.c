#include "Form.h"
#include "oopsIO.h"
#define THIS	Form
#define	BASE	Object
DEFINE_CLASS(Form,Object,1,"$Header$",NULL,NULL);

Form::Form(istream& strm, Form& where)
{
	this = &where;
	readFrom(strm,"Dictionary",fieldNames);
	readFrom(strm,"OrderedCltn",fields);
}

void Form::storer(ostream& strm)
{
	BASE::storer(strm);
	fieldNames.storeOn(strm);
	fields.storeOn(strm);
}

void Form::addField(const Field& f)
{
	fieldNames.addAssoc(f.name(),f);
	fields.add(f);
}

void Form::display()
{
	DO(fields,Field*,f) f->display(); OD
}

void Form::fillIn()
{
	DO(fields,Field*,f) f->fillIn(); OD
}
