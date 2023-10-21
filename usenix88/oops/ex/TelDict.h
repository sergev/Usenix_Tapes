#ifndef TELDICTH
#define TELDICTH 

#include "Dictionary.h"

extern const Class class_TelDict;
overload TelDict_reader;

class TelDict: public Dictionary {
protected:
	TelDict(fileDescTy&,TelDict&);
	TelDict(istream&,TelDict&);
	friend	void TelDict_reader(istream& strm, Object& where);
	friend	void TelDict_reader(fileDescTy& fd, Object& where);
public:
	TelDict(int size =CLTN_DEFAULT_CAPACITY);
	virtual const Class* isA();
	virtual void printOn(ostream&);
};

#endif
