#include "TelDict.h"
#include "LookupKey.h"
#include "SortedCltn.h"

#define	THIS	TelDict
#define	BASE	Dictionary
DEFINE_CLASS(TelDict,Dictionary,1,"$Header$",NULL,NULL);

TelDict::TelDict(int size) : (size) {}

void TelDict::printOn(ostream& strm)
{
	SortedCltn c;
	DO(*this,LookupKey*,a) c.add(*a); OD
	DO(c,LookupKey*,a)
		strm << *a->key() << "\t" << *a->value() << "\n";
	OD
}

TelDict::TelDict(istream& strm, TelDict& where) : (strm,where) {}

TelDict::TelDict(fileDescTy& fd, TelDict& where) : (fd,where) {}
