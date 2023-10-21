/* String.c -- implementation of character strings

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

Function:
	
Class String implements character string objects.  Operations provided
include & (concatenation) and () (substring extraction).  Type
conversions between String and char* are provided, permitting the two
to be used interchangeably in many contexts.  Note also that
SubStrings are not derived classes from Object.

$Log:	String.c,v $
 * Revision 1.4  88/02/04  12:52:14  keith
 * Make Class class_CLASSNAME const.
 * Make destructor non-inline.
 * 
 * Revision 1.3  88/01/17  00:03:29  keith
 * Change call to Range::found() to Range::valid()
 * 
 * Revision 1.2  88/01/16  23:41:29  keith
 * Remove pre-RCS modification history
 * 
*/

#include "String.h"
#include "Range.h"
#include "oopsconfig.h"
#include "oopsIO.h"
#include <ctype.h> 

#define	THIS	String
#define	BASE	Object

DEFINE_CLASS(String,Object,1,"$Header: String.c,v 1.4 88/02/04 12:52:14 keith Exp $",NULL,NULL);

extern const int OOPS_BADCLASS, OOPS_BADRANGE, OOPS_INDEXRANGE, OOPS_SUBSTRERR;

/* System-independent versions of toupper and tolower */

inline char to_upper(unsigned char c)	{ return (islower(c) ? (c-'a'+'A') : c); }
inline char to_lower(unsigned char c)	{ return (isupper(c) ? (c-'A'+'a') : c); }


//==== SubString functions:

/*
The following compare functions were implemented because strncmp is
not adequate for comparing character strings of unequal length.  For
example, strncmp("abc","abcd",3) will return 0.
*/

int SubString::compare(const char* cs)
/*
Return integer greater than, equal to, or less than 0, according as
this SubString is lexicographically greater than, equal to, or less
than cs.
*/
{
	int cl = strlen(cs);
	int result = strncmp(sp,cs,sl);
	if (result != 0 || sl == cl) return result;
	return (sl>cl ? 1 : -1);
}

int SubString::compare(const String& s)
/*
Return integer greater than, equal to, or less than 0, according as
this SubString is lexicographically greater than, equal to, or less
than s.
*/
{
	int result = strncmp(sp,s.p,sl);
	if (result != 0 || sl == s.len) return result;
	return (sl>s.len ? 1 : -1);
}

int SubString::compare(const SubString& ss)
/*
Return integer greater than, equal to, or less than 0, according as
this SubString is lexicographically greater than, equal to, or less
than SubString ss.
*/
{
	int result = strncmp(sp,ss.sp,MIN(sl,ss.sl));
	if (result != 0 || sl == ss.sl) return result;
	return (sl>ss.sl ? 1 : -1);
}

void SubString::printOn(ostream& strm)
// Print this SubString on output stream strm.
{
	strm << String(*this);
#ifdef DEBUG
	strm << "[" << st->p << "(" << position() << "," << sl << ")]";
#endif
}

void SubString::operator=(const String& s)
{
	if (sl == s.length()) strncpy(sp,s.p,sl);
	else replace(s.p,s.len);
}

void SubString::operator=(const SubString& ss)
{
	if (sl == ss.sl) strncpy(sp,ss.sp,sl);
	else replace(ss.sp,ss.sl);
}

void SubString::operator=(const char* cs)
{
	int cslen = strlen(cs);
	if (sl == cslen) strncpy(sp,cs,sl);
	else replace(cs,cslen);
}

void SubString::replace(const char* src, unsigned srclen)
// Replace this SubString with the argument string
// Terminology:
//	head: portion of destination string before this SubString
//	tail: portion of destination string after this SubString
{
#ifdef DEBUG
cerr << "replacing " << *this << " by " << form("%.*s ...\n",srclen,src);
#endif
	bool overlap = NO;	// src overlaps destination String
	int tailDelta = 0;	// amount to adjust for tail movement
	char* srcbuf = 0;	// buffer to hold src if it overlaps SubString
// src overlap destination String?
	if (src >= st->p && src <= st->p+st->len) {
		overlap = YES;	// src overlaps destination String
// src overlap only tail of destination String?
		if (src >= sp+sl) tailDelta = srclen-sl;
		else {		
// src overlap this SubString?
			if (src+srclen > sp) {	// move src to buffer
				srcbuf = new char[srclen];
				strncpy(srcbuf,src,srclen);
				src = srcbuf;
				overlap = NO;	// no overlap now
			}
		}
	}
#ifdef DEBUG
cerr << "overlap=" << overlap << "  tailDelta=" << tailDelta << "  srcbuf=" << (int)srcbuf << "\n";
#endif
	if (srclen+st->len >= sl+st->alloc) {	// need to make String bigger
		char* p = st->p;
		st->alloc = st->len+srclen-sl+DEFAULT_STRING_EXTRA+1;
		st->p = realloc(st->p, st->alloc);
		sp += st->p-p;
		if (overlap) src += st->p-p;
#ifdef DEBUG
cerr << "realloc(" << st->alloc << ")  " << *this << "\n";
#endif
	}
	if (sl > srclen) {	// shift tail down
		register const char* p = sp+sl;
		register char* q = sp+srclen;
		while (*q++ = *p++);
	}
	else {			// shift tail up
		register const char* p = st->p+st->len;
		register char* q = (char*)p+srclen-sl;
		register unsigned n = p-(sp+sl)+1;
		while (n--) *q-- = *p--;
	}
	src += tailDelta;
	st->len += srclen-sl;
#ifdef DEBUG
cerr << "target " << *this << " source " << form("%.*s\n",srclen,src);
#endif
	strncpy(sp,src,srclen);		// insert src into destination
	if (srcbuf) free(srcbuf);
#ifdef DEBUG
cerr << "... result: " << *this << "\n";
#endif
}

String SubString::operator&(const SubString& ss)  
{
	String t(sl + ss.sl);
	strncpy (t.p, sp, sl);
	strncpy (&(t.p[sl]), ss.sp, ss.sl);
	t.len = sl + ss.sl;
	t.p[t.len] = '\0';
	return t;
}

String SubString::operator&(const String& s)
{
	String t(sl + s.alloc -1);
	strncpy(t.p, sp, sl);
	strcpy(&(t.p[sl]), s.p);
	t.len = sl + s.len;
	return t;
}

String SubString::operator&(const char* cs)
{
	int cslen = strlen(cs);
	String t(sl + cslen);
	strncpy(t.p,sp,sl);
	strcpy(&(t.p[sl]),cs);
	t.len = sl + cslen;
	return t;
}

String operator&(const char* cs, const SubString& ss)
{
	unsigned cslen = strlen(cs);
	String t(cslen + ss.sl);
	strcpy(t.p,cs);
	strncpy(&(t.p[cslen]),ss.sp,ss.sl);
	t.len = cslen + ss.sl;
	t.p[t.len] = '\0';
	return t;
}

void SubString::checkSubStr()
// check for legal SubString
{
	unsigned pos = position();
	unsigned len = st->len;
	if (pos+sl <= len) return;
	if (sl == 0 && pos == len) return;
	setOOPSerror(OOPS_SUBSTRERR,DEFAULT,st,st->className(),pos,sl);
}

//==== String Private functions:

void String::indexRangeErr()
{
	setOOPSerror(OOPS_INDEXRANGE,DEFAULT,this,className());
}

//==== String Constructors:

String::String(char& c, unsigned l, unsigned extra)
{
	len = l;
	alloc = len + extra + 1;
	p = malloc(alloc);
	register unsigned i=len;
	p[i] = '\0';
	while (i > 0) p[--i] = c;
}

String::String()
{
	len = 0;
	alloc = DEFAULT_STRING_EXTRA + 1;
	p = malloc(alloc);
	*p = '\0';
}

String::String(unsigned extra)
{
	len = 0;
	alloc = extra + 1;
	p = malloc(alloc);
	*p = '\0';
}

String::String(const char* cs)
{ 
	len = strlen(cs);
	alloc = len + DEFAULT_STRING_EXTRA + 1;
	p = malloc(alloc);
  	strcpy(p,cs); 
}

String::String(const char* cs, unsigned extra)
{ 
	len = strlen(cs);
	alloc = len + extra + 1;
	p = malloc(alloc);
  	strcpy(p,cs); 
}

String::String(const String& s)
{
	len = s.len;
	alloc = len + DEFAULT_STRING_EXTRA + 1;
	p = malloc(alloc);
	strcpy (p,s.p);
}
 
String::String(const String& s, unsigned extra)
{
	len = s.len;
	alloc = len + extra + 1;
	p = malloc(alloc);
	strcpy(p,s.p);
}

String::String(const SubString& ss)
{
	len = ss.sl;
	alloc = len + DEFAULT_STRING_EXTRA + 1;
	p = malloc(alloc);
	strncpy(p,ss.sp,ss.sl);
	p[len] = '\0';
}

String::String(const SubString& ss, unsigned extra)
{
	len = ss.sl;
	alloc = len + extra + 1;
	p = malloc(alloc);
	strncpy(p,ss.sp,ss.sl);
	p[len] = '\0';
}

String::String(fileDescTy& fd, String& where)
{
	this = &where;
	readBin(fd,len);
	readBin(fd,alloc);
	p = malloc(alloc);
	readBin (fd,p,len+1);
}

String::String(istream& strm, String& where)
{
	this = &where;
	strm >> len;
	strm >> alloc;
	p = malloc(alloc);
	read_Cstring(strm,p,len+1);
	p[len+1] = '\0';
}

String::~String()	{ free(p); }

//==== Operators:

SubString String::operator()(const Range& r)
{
	if (r.valid()) return SubString(*this,r.firstIndex(),r.length());
	else  setOOPSerror(OOPS_BADRANGE,DEFAULT,this,className(),"operator()",r.firstIndex(),r.length());
}

void String::operator=(const String& s)
{
	if (p == s.p) return;
	len = s.len;
	if (len >= alloc) {
		free(p);
		p = malloc(alloc = s.alloc);
	}
	strcpy(p,s.p);
}

void String::operator=(const SubString& ss)
{
	len = ss.sl;
	if (this == ss.st) {		// s = s(pos,len)
		if (ss.sp == p) {	// s = s(0,len)
			p[len] = '\0';
			return;
		}
		register const char* src = ss.sp;
		register char* dst = p;
		register unsigned n = len;
		while (n--) *dst++ = *src++;
		*dst = '\0';
		return;
	}
	else if (len >= alloc) {
		alloc = ss.sl + DEFAULT_STRING_EXTRA + 1;
		free(p);
		p = malloc(alloc);
	}
	strncpy(p, ss.sp, ss.sl);
	p[len] = '\0';
}

void String::operator=(const char* cs)
{
	len = strlen(cs);
	if (len >= alloc) {
		alloc = len + DEFAULT_STRING_EXTRA + 1;
		free(p);
		p = malloc(alloc);
	}	
	strcpy(p,cs);
}

String String::operator&(const String& s)
{
	String t(len+s.len);
	strcpy(t.p,p);
	strcpy(&(t.p[len]), s.p);
	t.len = len+s.len;
	return t;
}

String String::operator&(const SubString& ss)
{
	String t(len+ss.sl);
	strcpy(t.p,p);
	strncpy(&(t.p[len]), ss.sp, ss.sl);
	t.len = len+ss.sl;
	t.p[t.len] = '\0';
	return t;
}

String String::operator&(const char* cs)
{
	unsigned cslen = strlen(cs);
	String t(len+cslen);
	strcpy (t.p,p);
	strcpy (&(t.p[len]), cs);
	t.len = len+cslen;
	return t;
}

String operator&(const char* cs, const String& s)
{
	int cslen=strlen(cs);
	String t(cslen + s.len);
	strcpy(t.p,cs);
	strcpy(&(t.p[cslen]),s.p);
	t.len = cslen + s.len;
	return t;
}

String& String::operator&=(const String& s)
// Concatenate a String with another
{
	if (alloc <= len + s.len) {
		alloc += s.len;
		p = realloc(p, alloc);
	}
	strcpy(&p[len],s.p);
	len += s.len;
	return *this;
}	

String& String::operator&=(const SubString& ss)
{
	if (alloc <= len + ss.sl) {
		alloc += ss.sl;
		if (this == ss.st) {	// s &= s(pos,len)
			char* t = p;
			p = malloc(alloc);
			strcpy(p,t);
			strncpy(&(p[len]),ss.sp,ss.sl);
			free(t);
			len += ss.sl;
			p[len] = '\0';
			return *this;
		}
		else p = realloc(p,alloc);
	}		
	strncpy(&(p[len]),ss.sp,ss.sl);
	len += ss.sl;
	p[len] = '\0';
	return *this;
}

String& String::operator&=(const char* cs)
{
	int cslen = strlen(cs);
	if (alloc <= len + cslen) {
		alloc += cslen;
		p = realloc(p,alloc);
	}
	strcpy(&(p[len]),cs);
	len += cslen;
	return *this;
}

void String::toAscii()
{
	register unsigned i = len;
	register char* q = p;
	while (i--) { *q = toascii(*q); q++; }
}

void String::toLower()
{
	register unsigned i = len;
	register char* q = p;
	while (i--) { *q = to_lower(*q); q++; }
}

void String::toUpper()
{
	register unsigned i = len;
	register char* q = p;
	while (i--) { *q = to_upper(*q); q++; }
}

int String::compare(const Object& ob)
{
	assertArgClass(ob,class_String,"compare");
	return strcmp(p, ((String*)&ob)->p);
}

void String::deepenShallowCopy()
{
	char* q = p;
	p = malloc(alloc);
	strcpy(p,q);
}

static union char_mask {
	unsigned in[sizeof(int)];
	char ch[sizeof(int)*sizeof(int)];
	char_mask();
} mask;

static char_mask::char_mask()
{
	for (register unsigned i=0; i<sizeof(int); i++) {
		for (register unsigned j=0; j<sizeof(int); j++) ch[sizeof(int)*i+j] = j<i ? 0xff : 0;
	}
}

bool String::operator==(const String& s)
{
	if (len != s.len) return NO;
	register unsigned i = div_sizeof_int(len);
	register const unsigned* q = (unsigned*)p;
	register const unsigned* r = (unsigned*)s.p;
	while (i--) if (*q++ != *r++) return NO;
	if ((i = mod_sizeof_int(len)) != 0)
		if ((*q & mask.in[i]) != (*r & mask.in[i])) return NO;
	return YES;
}

unsigned String::hash()
{
	register unsigned h = len;
	register unsigned i = div_sizeof_int(len);
	register unsigned* q = (unsigned*)p;
	while (i--) h ^= *q++;
	if ((i = mod_sizeof_int(len)) != 0)
		h ^= *q & mask.in[i];
	return h;
}

bool String::isEqual(const Object& a)
{
	return a.isSpecies(class_String) && *this==*(String*)&a;
}

void String::printOn(ostream& strm)	{ strm << p; }

void String::scanFrom(istream& strm)
//	Read next line of input from strm into this String.
{
	ostream* os = strm.tie((ostream*)0);
	if (os != 0) {
		os->flush();
		strm.tie(os);
	}
	char c;
	strm.get(c);
	if (c != '\n') strm.putback(c);
	char temp[513];
	strm.get(temp,513);
	*this = String(temp);
}

unsigned String::reSize(unsigned new_capacity)
{
	if (new_capacity < len) new_capacity = len;
	if (alloc != new_capacity+1) {
		p = realloc(p,alloc = new_capacity+1);
	}
	return alloc - 1;
}

unsigned String::size()			{ return len; }

const Class* String::species()
{
	return &class_String;
}

void String::storer(fileDescTy& fd) 
{
	Object::storer(fd);
	storeBin(fd,len);
	storeBin(fd,alloc);
	storeBin(fd,p,len+1);         //Store concluding null character.
}

void String::storer(ostream& strm)
{
	Object::storer(strm);
	strm << len << " " << alloc << " ";
	store_Cstring(strm,p);
}


