#include "DateDict.h"
#include "oopsIO.h"
// #include .h files for other classes used
#include "String.h"
#include "Date.h"
#include "SortedCltn.h"

#define	THIS	DateDict
#define	BASE	Dictionary
DEFINE_CLASS(DateDict,Dictionary,1,"$Header$",NULL,NULL);

DateDict::DateDict(istream& strm, DateDict& where) : (strm,where)
// Construct an object from istream "strm" at address "where".
{
	this = &where;
}

DateDict::DateDict(fileDescTy& fd, DateDict& where) : (fd,where)
// Construct an object from file descriptor "fd" at address "where".
{
	this = &where;
}

void DateDict::addName(const Date& birthdate, const String& name)
{
	SortedCltn* list;
	if (includesKey(birthdate)) list = &lookup(birthdate);
	else {
		list = new SortedCltn;
		addAssoc(birthdate,*list);
	}
	list->add(name);
	return;
}

SortedCltn& DateDict::lookup(const Date& birthdate) {
	return *(SortedCltn*)atKey(birthdate);
}
