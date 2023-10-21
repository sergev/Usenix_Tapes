#ifndef	ORDEREDCLTN_H
#define	ORDEREDCLTN_H

/* OrderedCltn.h -- declarations for abstract ordered collections

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

$Log:	OrderedCltn.h,v $
 * Revision 1.3  88/02/04  12:59:49  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:40:19  keith
 * Remove pre-RCS modification history
 * 
*/

#include "SeqCltn.h"
#include "ArrayOb.h"

extern const Class class_OrderedCltn;
overload OrderedCltn_reader;

class SortedCltn;

class OrderedCltn : public SeqCltn {
	int endIndex;
	ArrayOb contents;
	Object* addAtIndex(int i, const Object& ob);
	void errEmpty(const char* fn);
	void errNotFound(const char* fn, const Object& ob);
	Object* removeAtIndex(int i);
	friend SortedCltn;
protected:
	OrderedCltn(fileDescTy&,OrderedCltn&);
	OrderedCltn(istream&,OrderedCltn&);
	friend	void OrderedCltn_reader(istream& strm, Object& where);
	friend	void OrderedCltn_reader(fileDescTy& fd, Object& where);
public:
	OrderedCltn(unsigned size =CLTN_DEFAULT_CAPACITY);
	OrderedCltn(const OrderedCltn&);
	bool operator!=(const OrderedCltn& a)	{ return !(*this==a); }
	void operator=(const OrderedCltn&);
	bool operator==(const OrderedCltn&);
	Object*& operator[](int i) {
		if ((unsigned)i > endIndex) indexRangeErr();
		return contents[i];
	}
	OrderedCltn operator&(const SeqCltn& cltn);	// concatenation operator 
	void operator&=(const SeqCltn& cltn);
	virtual Object* add(const Object&);
	virtual Object* addAfter(const Object& ob, const Object& newob);
	virtual Object* addAllLast(const OrderedCltn&);
	virtual Object* addBefore(const Object& ob, const Object& newob);
	virtual Collection& addContentsTo(Collection& cltn);
	virtual Object* addLast(const Object& ob);
	virtual Object* after(const Object&);
	virtual Object*& at(int i);
	virtual void atAllPut(const Object& ob);
	virtual Object* before(const Object&);
	virtual unsigned capacity();
	virtual void deepenShallowCopy();
	virtual Object* first();
	virtual unsigned hash();
 	virtual int indexOf(const Object& ob);
	virtual int indexOfSubCollection(const SeqCltn& cltn, int start=0);
	virtual const Class* isA();
	virtual bool isEmpty();
	virtual bool isEqual(const Object&);
	virtual Object* last();
	virtual unsigned occurrencesOf(const Object&);
	virtual void printOn(ostream& strm);
	virtual Object* remove(const Object&);
	virtual Object* removeId(const Object&);
	virtual Object* removeLast();
	virtual void replaceFrom(int start, int stop, const SeqCltn& replacement, int startAt =0);
	virtual void reSize(unsigned newSize);
	virtual unsigned size();
	virtual void sort();
	virtual const Class* species();
};

#endif
