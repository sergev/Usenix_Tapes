/* String.c -- implementation of character strings

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
	uucp: {decvax!}seismo!elsie!cecil!keith
	September, 1985

Function:
	
Class String implements character string objects.  Operations provided
include & (concatenation) and () (substring extraction).  Type
conversions between String and char* are provided, permitting the two to
be used interchangeably in many contexts.  Note that a String should not
be used as a char* argument to a function that returns a result in that
argument since the String size will not be properly set and the result
may overflow the String capacity.  Note also that SubStrings are not
derived classes from Object.

Modification History:

8-Oct-87	K. E. Gorlen

1.  Change String(unsigned,char) to String(char,unsigned =1) so that
    expressions like string &= '0' behave well.

8-Apr-87	K. E. Gorlen

1.  Correct bug in String::String(unsigned l, char c) caused
by change to unsigned.

16-Dec-86	K. E. Gorlen

1.  Correct toUpper/toLower to call to_upper/to_lower.

06-Oct-86	S. M. Orlow

1.  Added binary I/O constructor, storer, and reader functions

26-Sep-86	K. E. Gorlen

1.  C++ Release 1.1: remove work-arounds for CC bugs.
2.  Add inline member function length().

16-Jul-86	K. E. Gorlen

1.  Implement system-independent versions of toupper and tolower.

26-Feb-86

1.  Implement operator& as out-of-line to work around CC compiler bug.

7-Feb-86	K. E. Gorlen

1.  Implement String::scanFrom(istream&).

7-Jan-86	K. E. Gorlen

1.  Deleted String::isEqual.  Arraychar::isEqual can be inherited due
to implementation of species.

2.  Deleted String::at.  Arraychar::at can be inherited.

*/

#include "String.h"
#include <ctype.h>
#include "oopsIO.h"

#define	THIS	String
#define	BASE	Arraychar
DEFINE_CLASS(String,Arraychar,1,NULL,NULL);

extern const int OOPS_SUBSTRERR;

/* System-independent versions of toupper and tolower */

inline char to_upper(char c)	{ return (islower(c) ? (c-'a'+'A') : c); }
inline char to_lower(char c)	{ return (isupper(c) ? (c-'A'+'a') : c); }

String::String(char c, unsigned l) : (l+1)
{
	register unsigned i=l;
	v[i] = '\0';
	while (i != 0) v[--i] = c;
}

void String::toAscii()
{
	register unsigned i = sz;
	register char* p = v;
	while (i--) *p++ = toascii(*p);
}

void String::toLower()
{
	register unsigned i = sz;
	register char* p = v;
	while (i--) *p++ = to_lower(*p);
}

void String::toUpper()
{
	register unsigned i = sz;
	register char* p = v;
	while (i--) *p++ = to_upper(*p);
}

int String::compare(const Object& ob)
{
	assertArgClass(ob,class_String,"compare");
	return strcmp(v, ((String*)&ob)->v);
}

void String::printOn(ostream& strm)	{ strm << v; }
	
void String::scanFrom(istream& strm)
/*
	Read next line of input from strm into this String.
*/
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

unsigned String::size()			{ return sz-1; }

void String::errSubStr(unsigned pos, unsigned lgt)
{
	setOOPSerror(OOPS_SUBSTRERR,DEFAULT,this,className(),pos,lgt);
}

String::String(istream& strm, String& where) : ((strm >> sz, sz))
{
	this = &where;
	read_Cstring(strm,v,sz);
}

void String::storer(ostream& strm)
{
	Object::storer(strm);
	strm << sz;
	store_Cstring(strm,v);
}

String::String(fileDescTy& fd, String& where) : (fd,where)
{
	this = &where;
}

void String::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
}
