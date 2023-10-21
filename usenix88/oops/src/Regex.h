#ifndef	REGEX_H
#define	REGEX_H

/* Regex.h -- header file for class Regex

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	December, 1987

$Log:	Regex.h,v $
 * Revision 1.3  88/02/04  13:00:05  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:40:56  keith
 * Remove pre-RCS modification history
 * 
*/

#include "String.h"
#include "Range.h"
#include "regex.h"

const unsigned DEFAULT_REGEX_BUFSIZE = 64;

extern const Class class_Regex;
overload Regex_reader;

class Regex: public String {
	struct re_pattern_buffer pattern;
	unsigned ngroups;	// 1 + number of \( \) groups in pattern
	struct re_registers regs;
	void re_compile_pattern();
	int re_match(const String&, int pos=0);
	void init(int bufsize);
	void fixCopy();
	void setGroups(int);
	void errRegex(const char*);
protected:
	Regex(fileDescTy&,Regex&);
	Regex(istream&,Regex&);
	friend void Regex_reader(istream& strm, Object& where);
	friend void Regex_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
//	Regex(unsigned bufsize =DEFAULT_REGEX_BUFSIZE);  // cfront bug
	Regex();
	Regex(unsigned bufsize);
	Regex(const char*, unsigned bufsize =DEFAULT_REGEX_BUFSIZE);
	Regex(const Regex&);
	~Regex();
//	operator const char*()	{ return *(String*)this; }	// cfront bug
#ifndef __cplusplus
	operator constCharPtTy()	{ return *(String*)this; }
#endif
	Range operator[](unsigned);
	unsigned groups()	{ return ngroups; }
	bool match(const String&, int pos=0);
	int search(const String&, int startpos=0);
	int search(const String&, int startpos, int range);
	void operator=(const char*);
	void operator=(const String&);
	void operator=(const SubString&);
	void operator=(const Regex&);
	String& operator&=(const String&)	{ shouldNotImplement("operator&="); return *this; }
	String& operator&=(const SubString&)	{ shouldNotImplement("operator&="); return *this; }
	String& operator&=(const char*)		{ shouldNotImplement("operator&="); return *this; }
	void dumpOn(ostream& strm =cerr);
	virtual void deepenShallowCopy();
	virtual const Class* isA();
	virtual void scanFrom(istream& strm);
	virtual void toAscii();
	virtual void toLower();
	virtual void toUpper();
};


#endif
