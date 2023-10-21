#ifndef	ARRAYOB_H
#define	ARRAYOB_H

/* ArrayOb.h -- declarations for array of object pointers (object IDs)

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

$Log:	ArrayOb.h,v $
 * Revision 1.3  88/02/04  12:51:51  keith
 * Make Class class_CLASSNAME const.
 * Make destructor non-inline.
 * 
 * Revision 1.2  88/01/16  23:37:05  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Collection.h"
#include <malloc.h>

overload REALLOC;
overload DELETE;

inline Object** REALLOC(Object** ptr, unsigned size)
{
	return (Object**)realloc((char*)ptr,sizeof(Object*)*size);
}

inline void DELETE(Object** ptr) { free((char*)ptr); }

extern const Class class_ArrayOb;
overload ArrayOb_reader;

class ArrayOb: public Collection {
	Object** v;
	unsigned sz;
	void allocSizeErr();
	void indexRangeErr();
protected:
	ArrayOb(fileDescTy&,ArrayOb&);
	ArrayOb(istream&,ArrayOb&);
	friend	void ArrayOb_reader(istream& strm, Object& where);
	friend	void ArrayOb_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	ArrayOb(unsigned size =CLTN_DEFAULT_CAPACITY);
	ArrayOb(const ArrayOb&);
	~ArrayOb();
	Object*& elem(int i)	{ return v[i]; }
	bool operator!=(const ArrayOb& a)	{ return !(*this==a); }
	void operator=(const ArrayOb&);
	bool operator==(const ArrayOb&);
	Object*& operator[](int i) {
		if ((unsigned)i >= sz) indexRangeErr();
		return v[i];
	}
	virtual Collection& addContentsTo(Collection&);
	virtual Object*& at(int i);
	virtual unsigned capacity();
	virtual void deepenShallowCopy();
	virtual Object* doNext(Iterator&);
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual void reSize(unsigned);
	virtual unsigned size();
	virtual void sort();
	virtual const Class* species();
};

#endif
