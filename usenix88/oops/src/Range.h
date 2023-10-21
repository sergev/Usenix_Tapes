#ifndef	RANGE_H
#define	RANGE_H

/* Range.h -- header file for class Range

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
		C. J. Eppich
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

$Log:	Range.h,v $
 * Revision 1.3  88/02/04  12:59:59  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:33:49  keith
 * Remove pre-RCS modification history
 * Replace operator()(int,int) with int length(int), int firstIndex(int),
 * 	and int lastIndex(int).
 * 
*/

#include "Object.h"

extern const Class class_Range;
overload Range_reader;

class Range: public Object {
	int first,len;
protected:
	Range(fileDescTy&,Range&);
	Range(istream&,Range&);
	friend	void Range_reader(istream& strm, Object& where);
	friend	void Range_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Range()			{ first = 0; len = -1; }
	Range(int f, int l)	{ first = f; len = l; }
	Range(const Range& r)	{ first = r.first;  len = r.len; }
	int length()		{ return len; }
	int length(int l)	{ return len = l; }
	int firstIndex()	{ return first; }
	int firstIndex(int f)	{ return first = f; }
	int lastIndex()		{ return (first + len - 1); }
	int lastIndex(int i)	{ len = i - first + 1;  return i; }
	bool valid()		{ return (len >= 0); }
	void operator=(const Range& r)  { first = r.first;  len = r.len; }
	bool operator==(const Range& r) { return ((first == r.first) && (len == r.len)); }
	bool operator!=(const Range& r)	{ return !(*this==r); }
	virtual Object* copy();			// return shallowCopy();
	virtual void deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual const Class* species();
};

#endif
