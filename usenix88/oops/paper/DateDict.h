#ifndef DateDictH
#define DateDictH

#include "Dictionary.h"
// #include .h files for other classes used

// insert class declarations for classes accessed by pointer and reference ONLY
class String;
class Date;
class SortedCltn;

extern Class class_DateDict;
overload DateDict_reader;

class DateDict: public Dictionary {
protected:
	DateDict(fileDescTy&,DateDict&);
	DateDict(istream&,DateDict&);
	friend	void DateDict_reader(istream& strm, Object& where);
	friend	void DateDict_reader(fileDescTy& fd, Object& where);
//	virtual void storer(fileDescTy&);
//	virtual void storer(ostream&);
public:
	DateDict(int size =CLTN_DEFAULT_CAPACITY) : (size) {}
	virtual const Class* isA();
	void addName(const Date& birthdate, const String& name);
	SortedCltn& lookup(const Date& birthdate);
};

#endif
