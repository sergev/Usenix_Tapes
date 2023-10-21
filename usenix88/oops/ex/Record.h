#ifndef RECORDH
#define RECORDH 

#include "Object.h"
#include "String.h"

extern const Class class_Record;
overload Record_reader;

class Record: public Object {
	String* v_name;
	String v_address;
	String v_phone;
	unsigned v_zip;
protected:
	Record(fileDescTy&,Record&);
	Record(istream&,Record&);
	friend	void Record_reader(istream& strm, Object& where);
	friend	void Record_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Record(const String& aName, const char* anAddress, unsigned aZip, const char* aPhone);
	Record(const Record&);
	void operator=(const Record&);
	bool operator==(const Record&);
	bool operator!=(const Record& r)	{ return !(*this == r); }
	String&	name()		{ return *v_name; }
	String	address()	{ return v_address; }
	String	phone()		{ return v_phone; }
	int	zip()		{ return v_zip; }
	virtual int	compare(const Object&);
	virtual void	deepenShallowCopy();
	virtual unsigned hash();
	virtual Class*	isA();
	virtual bool	isEqual(const Object&);
	virtual void	printOn(ostream&);
};

#endif
