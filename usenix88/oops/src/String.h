#ifndef	STRING_H
#define	STRING_H

/* String.h -- declarations for character strings

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Authors:
	C. J. Eppich and K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	December, 1987

$Log:	String.h,v $
 * Revision 1.4  88/02/04  12:52:18  keith
 * Make Class class_CLASSNAME const.
 * Make destructor non-inline.
 * 
 * Revision 1.3  88/01/29  21:56:48  keith
 * Change DEFAULT_STRING_EXTRA from 16 to 15.
 * 
 * Revision 1.2  88/01/16  23:41:32  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"
#include <string.h>
#include <malloc.h>

#define DEFAULT_STRING_EXTRA 	15

typedef const char* constCharPtTy;	// cfront bug
class String;
class Range;
extern const Class class_String;
overload String_reader;

class SubString {
	char* sp;		// substring pointer 
        unsigned sl;		// substring length 
	String* st;		// String this is a SubString of
	SubString(const String&, unsigned, unsigned);
	int compare(const char*);
	int compare(const String&);
	int compare(const SubString&);
	void replace(const char* st, unsigned ln);
	void checkSubStr();
	friend String;
public:
	const char* ptr()	{ return sp; }
	unsigned position();
	unsigned length()	{ return sl; }

	void operator=(const String&);
	void operator=(const SubString&);
	void operator=(const char*);

	bool operator<(const String&);	 
	bool operator>(const String&);	 
	bool operator<=(const String&); 
	bool operator>=(const String&); 
	bool operator==(const String&); 
	bool operator!=(const String&); 

	bool operator<(const SubString& ss) { return compare(ss)<0; } 
	bool operator>(const SubString& ss) { return compare(ss)>0; }
	bool operator<=(const SubString& ss){ return compare(ss)<=0; }
	bool operator>=(const SubString& ss){ return compare(ss)>=0; }
	bool operator==(const SubString& ss){ return compare(ss)==0; }
	bool operator!=(const SubString& ss){ return compare(ss)!=0; }

	bool operator<(const char* cs)	{ return compare(cs) < 0; }   
	bool operator>(const char* cs)	{ return compare(cs) > 0; }   
	bool operator<=(const char* cs) { return compare(cs) <= 0; }
	bool operator>=(const char* cs) { return compare(cs) >= 0; }
	bool operator==(const char* cs) { return compare(cs) == 0; }
	bool operator!=(const char* cs) { return compare(cs) != 0; }

	friend bool operator<(const char* cs, const SubString& ss) {
	    return ss.compare(cs) > 0;
	}
	friend bool operator>(const char* cs, const SubString& ss) {	 
	    return ss.compare(cs) < 0;
	}
	friend bool operator<=(const char* cs, const SubString& ss) { 
	    return ss.compare(cs) >= 0; 
	}
	friend bool operator>=(const char* cs, const SubString& ss) { 
	    return ss.compare(cs) <= 0;
	}
	friend bool operator==(const char* cs, const SubString& ss) {
	    return ss.compare(cs) == 0; 
	}
	friend bool operator!=(const char* cs, const SubString& ss) {
	    return ss.compare(cs) != 0; 
	}

	String operator&(const String&);	
	String operator&(const SubString&);
	String operator&(const char*);
	friend String operator&(const char*, const SubString&);

	void printOn(ostream& strm);
};

class String : public Object {
	char* p;		// character string
	unsigned len;		// length of string, excluding null character
	unsigned alloc;		// amount of storage allocated
	void indexRangeErr();
	friend SubString;
protected:
	String(fileDescTy&,String&);
	String(istream&,String&);
	friend	void String_reader(fileDescTy& fd, Object& where);
	friend	void String_reader(istream& strm, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);		
public:
	String(char& c, unsigned l=1, unsigned extra=DEFAULT_STRING_EXTRA);
//	String(unsigned extra=DEFAULT_STRING_EXTRA);	// cfront bug
	String();
	String(unsigned extra);
//	String(const char*, unsigned extra=DEFAULT_STRING_EXTRA); // cfront bug
	String(const char*);
	String(const char*, unsigned extra);
//	String(const String&, unsigned extra=DEFAULT_STRING_EXTRA); // cfront bug
	String(const String&);
	String(const String&, unsigned extra);
//	String(const SubString&, unsigned extra=DEFAULT_STRING_EXTRA); // cfront bug
	String(const SubString&);
	String(const SubString&, unsigned extra);
	~String();
//	operator const char*()		{ return p; }	// cfront bug
	operator constCharPtTy()	{ return p; }
	SubString operator()(unsigned pos, unsigned lgt);
	SubString operator()(const Range& r);
	char& operator[](unsigned i) {
		if (i >= len) indexRangeErr();
		return p[i];
    	}

	void operator=(const String&);
	void operator=(const SubString&);
	void operator=(const char*);

	bool operator<(const String& s)	 { return strcmp(p, s.p) < 0; }
	bool operator>(const String& s)	 { return strcmp(p, s.p) > 0; }
	bool operator<=(const String& s) { return strcmp(p, s.p) <= 0; }
	bool operator>=(const String& s) { return strcmp(p, s.p) >= 0; }
	bool operator==(const String& s);
	bool operator!=(const String& s) { return !(*this==s); }

	bool operator<(const SubString& ss) { return ss.compare(*this) > 0; }
	bool operator>(const SubString& ss) { return ss.compare(*this) < 0; }
	bool operator<=(const SubString& ss) { return ss.compare(*this) >= 0; }
	bool operator>=(const SubString& ss) { return ss.compare(*this) <= 0; }
	bool operator==(const SubString& ss) { return ss.compare(*this) == 0; }
	bool operator!=(const SubString& ss) { return ss.compare(*this) != 0; }

	bool operator<(const char* cs)	{ return strcmp(p,cs) < 0; }   
	bool operator>(const char* cs)	{ return strcmp(p,cs) > 0; }   
	bool operator<=(const char* cs) { return strcmp(p,cs) <= 0; }
	bool operator>=(const char* cs) { return strcmp(p,cs) >= 0; }
	bool operator==(const char* cs) { return strcmp(p,cs) == 0; }
	bool operator!=(const char* cs) { return strcmp(p,cs) != 0; }

	friend bool operator<(const char* cs, const String& s) {
	    return strcmp(cs, s.p) < 0;
	}
	friend bool operator>(const char* cs, const String& s) {	 
	    return strcmp(cs, s.p) > 0;
	}
	friend bool operator<=(const char* cs, const String& s) { 
	    return strcmp(cs, s.p) <= 0; 
	}
	friend bool operator>=(const char* cs, const String& s) { 
	    return strcmp(cs, s.p) >= 0;
	}
	friend bool operator==(const char* cs, const String& s) {
	    return strcmp(cs, s.p) == 0; 
	}
	friend bool operator!=(const char* cs, const String& s) {
	    return strcmp(cs, s.p) != 0; 
	}

	String operator&(const String& s);
	String operator&(const SubString& ss);
	String operator&(const char* cs);
	friend String operator&(const char*, const SubString&);
	friend String operator&(const char* cs, const String& s);

	String& operator&=(const String&);
	String& operator&=(const SubString&);
	String& operator&=(const char* cs);

	char& at(unsigned i)	{ return (*this)[i]; }
	unsigned length()	{ return len; } 
	unsigned reSize(unsigned new_capacity);

	virtual void toAscii();
	virtual void toLower();
	virtual void toUpper();
	virtual unsigned capacity() { return alloc - 1; }
	virtual int compare(const Object& ob);
	virtual void deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();	
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual void scanFrom(istream& strm);
	virtual unsigned size();
	virtual const Class* species();
};

inline ostream& operator<<(ostream& strm, const SubString& ss)
{
	ss.printOn(strm);
	return(strm);
}

inline SubString::SubString(const String& s, unsigned pos, unsigned lgt) { 
	sp = &s.p[pos];
	sl = lgt;
	st = &s;
	checkSubStr();
}
	
inline unsigned SubString::position()	{ return sp - st->p; }

/*inline*/ static SubString String::operator()(unsigned pos, unsigned lgt)
{	
	return SubString(*this, pos, lgt);
}

inline bool SubString::operator<(const String& s) { return compare(s) < 0; }
inline bool SubString::operator>(const String& s) { return compare(s) > 0; }
inline bool SubString::operator<=(const String& s) { return compare(s) <= 0; }
inline bool SubString::operator>=(const String& s) { return compare(s) >= 0; }
inline bool SubString::operator==(const String& s) { return compare(s) == 0; }
inline bool SubString::operator!=(const String& s) { return compare(s) != 0; }

#endif

