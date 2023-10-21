#include "Record.h"
#include "oopsIO.h"

#define	THIS	Record
#define	BASE	Object
DEFINE_CLASS(Record,Object,1,"$Header$",NULL,NULL);

Record::Record(const String& aName, const char* anAddress, unsigned aZip, const char* aPhone) :
	v_address(anAddress), v_phone(aPhone)
{
	v_name = &aName;
	v_zip = aZip;
}

Record::Record(const Record& r)
{
	v_name = r.v_name;
	v_address = r.v_address;
	v_phone = r.v_phone;
	v_zip = r.v_zip;
}

void Record::operator=(const Record& r)
{
	v_name = r.v_name;
	v_address = r.v_address;
	v_phone = r.v_phone;
	v_zip = r.v_zip;
}

bool Record::operator==(const Record& r)
{
	return *v_name == *r.v_name
		&& v_address == r.v_address
		&& v_phone == r.v_phone
		&& v_zip == r.v_zip;
}

bool Record::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Record) && *this==*(Record*)&ob;
}

unsigned Record::hash()
{
	return v_name->hash() ^ v_address.hash() ^ v_phone.hash() ^ v_zip;
}

int Record::compare(const Object& ob)
{
	assertArgSpecies(ob,class_Record,"compare");
	return v_name->compare(*((Record*)&ob)->v_name);
}

void Record::deepenShallowCopy()
// Called by deepCopy() to convert a shallow copy to a deep copy.
{
	v_name = (String*)v_name->deepCopy();
	v_address.deepenShallowCopy();
	v_phone.deepenShallowCopy();
}

void Record::printOn(ostream& strm)
{
	strm << *v_name << "\t" << v_address << " " << form("%05d",v_zip) << "\t" << v_phone;
}

Record::Record(istream& strm, Record& where)
{
	this = &where;
	v_name = (String*)readFrom(strm,"String");
	readFrom(strm,"String",v_address);
	readFrom(strm,"String",v_phone);
	strm >> v_zip;
}

void Record::storer(ostream& strm)
{
	BASE::storer(strm);
	v_name->storeOn(strm);
	v_address.storeOn(strm);
	v_phone.storeOn(strm);
	strm << v_zip << " ";
}

Record::Record(fileDescTy& fd, Record& where)
{
	this = &where;
	v_name = (String*)readFrom(fd,"String");
	readFrom(fd,"String",v_address);
	readFrom(fd,"String",v_phone);
	readBin(fd,v_zip);
}

void Record::storer(fileDescTy& fd)
{
	BASE::storer(fd);
	v_name->storeOn(fd);
	v_address.storeOn(fd);
	v_phone.storeOn(fd);
	storeBin(fd,v_zip);
}
