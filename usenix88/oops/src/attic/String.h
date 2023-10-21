#ifndef STRINGH
#define STRINGH

/* String.h -- declarations for character strings

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

Modification History:

8-Oct-87	K. E. Gorlen

1.  Change String(unsigned,char c =" ") to String(char,unsigned l =1) so that
    expressions like string &= '0' behave well.

25-Aug-87	K. E. Gorlen

1.  Protect readFrom constructors and storer().

*/

#include "Arraychar.h"
#include <string.h>

typedef const char* constCharPtTy;
class String;
extern Class class_String;
overload String_reader;

class SubString {
	char* sp;	// substring pointer 
	unsigned sl;		// substring length 
	SubString(const String& s, unsigned pos, unsigned lgt);
	friend String;
public:
	void operator=(const SubString& from)	{ strncpy(sp,from.sp,sl); }
	void operator=(const String& from);
	bool operator<(const String& s);
	bool operator>(const String& s);
	bool operator<=(const String& s);
	bool operator>=(const String& s);
	bool operator==(const String& s);
	bool operator!=(const String& s);
	String operator&(const String& s);
};

class String : public Arraychar {
	String(const char* cs, unsigned lgt) : (lgt+1) {
		strncpy(v,cs,lgt);
	}
	void errSubStr(unsigned pos, unsigned lgt);
	friend SubString;
protected:
	String(fileDescTy&,String&);
	String(istream&,String&);
	friend	void String_reader(istream& strm, Object& where);
	friend	void String_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	String() : (1) { *v = '\0'; }
	String(const char* cs) : (strlen(cs)+1) {
		strcpy(v,cs);
	}
	String(char c, unsigned l =1);
	String(const String& s) : (s)	{}
	String(const SubString& ss) : (ss.sl+1) {
		strncpy(v,ss.sp,ss.sl);
		v[ss.sl] = 0;
	}
	operator constCharPtTy()	{ return v; }
	SubString operator()(unsigned pos, unsigned lgt);
	char& operator[](unsigned pos) {
		if (pos >= sz-1) IndexRangeErr();
		return v[pos];
    	}
	void operator=(const String& s)	{ this->Arraychar::operator=(s); }
	void operator=(const SubString& ss)	{
		String s(ss);
		this->Arraychar::operator=(s);
	}
	bool operator<(const String& s)		{ return strcmp(v, s.v) < 0; }
	bool operator>(const String& s)		{ return strcmp(v, s.v) > 0; }
	bool operator<=(const String& s)	{ return strcmp(v, s.v) <= 0; }
	bool operator>=(const String& s)	{ return strcmp(v, s.v) >= 0; }
	bool operator==(const String& s)	{ return strcmp(v, s.v) == 0; }
	bool operator!=(const String& s)	{ return strcmp(v, s.v) != 0; }
	String operator&(const String& s) {
		String t(v, sz+s.sz-2);
		strcpy(&(t.v[sz-1]), s.v);
		return t;
	}
	void operator&=(const String& s) {
		unsigned oldSize = sz-1;
		reSize(s.sz-1+oldSize+1);
		strcpy(&v[oldSize],s.v);
	}
	unsigned length() { return sz-1; }
	void toAscii();
	void toLower();
	void toUpper();
	virtual int compare(const Object& ob);
	virtual const Class* isA();
	virtual void printOn(ostream& strm);
	virtual void scanFrom(istream& strm);
	virtual unsigned size();
};

inline SubString::SubString(const String& s, unsigned pos, unsigned lgt) { 
	sp = &s[pos];
	if (pos+lgt >= s.sz) s.errSubStr(pos,lgt);
	sl = lgt;
}
	
inline void SubString::operator=(const String& from)	{ strncpy(sp,from.v,sl); }

inline bool SubString::operator<(const String& s)	{ return strncmp(sp, s.v, sl) < 0; }
inline bool SubString::operator>(const String& s)	{ return strncmp(sp, s.v, sl) > 0; }
inline bool SubString::operator<=(const String& s)	{ return strncmp(sp, s.v, sl) <= 0; }
inline bool SubString::operator>=(const String& s)	{ return strncmp(sp, s.v, sl) >= 0; }
inline bool SubString::operator==(const String& s)	{ return strncmp(sp, s.v, sl) == 0; }
inline bool SubString::operator!=(const String& s)	{ return strncmp(sp, s.v, sl) != 0; }
	
/*inline*/ static SubString String::operator()(unsigned pos, unsigned lgt)
{
	return SubString(*this, pos, lgt);
}
	
inline String SubString::operator&(const String& s) {
	String t(sp, sl+s.sz-1);
	strcpy(&(t.v[sl]), s.v);
	return t;
}
	
#endif
