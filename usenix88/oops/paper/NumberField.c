#include "NumberField.h"
#include "oopsIO.h"
#define THIS	NumberField
#define	BASE	Field
DEFINE_CLASS(NumberField,Field,1,"$Header$",NULL,NULL);

NumberField::NumberField(istream& strm, NumberField& where) : (strm,where)
{
	this = &where;
	strm >> value;
}

void NumberField::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << value;
}

NumberField::NumberField(const char* fname, const Rectangle& floc, int fval) : (fname,floc)
{
	value = fval;
}

void NumberField::display() {}

void NumberField::fillIn() {}
