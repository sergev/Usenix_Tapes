#include "AlphaField.h"
#include "oopsIO.h"
#define THIS	AlphaField
#define	BASE	Field
DEFINE_CLASS(AlphaField,Field,1,"$Header$",NULL,NULL);

AlphaField::AlphaField(istream& strm, AlphaField& where) : (strm,where)
{
	this = &where;
	readFrom(strm,"String",value);
}

void AlphaField::storer(ostream& strm)
{
	BASE::storer(strm);
	value.storeOn(strm);
}

AlphaField::AlphaField(const char* fname, const Rectangle& floc, const char* fval) : (fname,floc)
{
	value = fval;
}

void AlphaField::display() {}

void AlphaField::fillIn() {}
