#include "Field.h"
#include "oopsIO.h"
#define THIS	Field
#define	BASE	Object
DEFINE_CLASS(Field,Object,1,"$Header$",NULL,NULL);

Field::Field(istream& strm, Field& where)
{
	this = &where;
	namePtr = (String*)readFrom(strm,"String");
	readFrom(strm,"Rectangle",location);
}

void Field::storer(ostream& strm)
{
	BASE::storer(strm);
	namePtr->storeOn(strm);
	location.storeOn(strm);
}

Field::Field(const char* fname, const Rectangle& floc)
{
	namePtr = new String(fname);
	location = floc;
}

void Field::display() {}

void Field::fillIn() {}
