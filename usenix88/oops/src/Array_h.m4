/* Array.h.m4 -- template for declarations for generic array objects

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
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	September, 1985

Function:

m4 macro template for the .h files for arrays of the specified
fundamental type: char, int, short, long, unsigned, float, and double.
For example, to generate the declarations for an array of chars:

	echo "ARRAYDECLARE(Arraychar,char,friend SubString;)" | m4 Array.h.m4 - >Arraychar.h

$Log:	Array_h.m4,v $
Revision 1.3  88/02/04  12:52:03  keith
Make Class class_CLASSNAME const.
Make destructor non-inline.

Revision 1.2  88/01/16  23:37:17  keith
Remove pre-RCS modification history

*/

define(ARRAYDECLARE,
``#ifndef'' $1_H
``#define'' $1_H

``#include'' "Collection.h"
``#include'' <malloc.h>

overload REALLOC;
overload DELETE;

inline $2* REALLOC($2* ptr, unsigned size)
{
	return ($2*)realloc((char*)ptr,sizeof($2)*size);
}

inline void DELETE($2* ptr) { free((char*)ptr); }

extern const Class class_$1;
overload $1_reader;

class $1: public Collection {
protected:
	$2* v;
	unsigned sz;
	void AllocSizeErr();
	void IndexRangeErr();
// friends go here
	$3
protected:
	$1(fileDescTy&,$1&);
	$1(istream&,$1&);
	friend	void $1_reader(istream& strm, Object& where);
	friend	void $1_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	$1(unsigned size =CLTN_DEFAULT_CAPACITY);
	$1(const $1&);
	~$1();
	$2& elem(int i)	{ return v[i]; }
	bool operator!=(const $1& a) { return !(*this==a); }
	void operator=(const $1&);
	bool operator==(const $1&);
	$2& operator[](int i)
	  { if ((unsigned)i >= sz) IndexRangeErr(); return v[i]; }
	virtual unsigned capacity();
	virtual void deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual void reSize(unsigned);
	virtual unsigned size();
	virtual void sort();
	virtual const Class* species();
};

``#endif''
)
