#ifndef	SORTEDCLTN_H
#define	SORTEDCLTN_H

/* SortedCltn.h -- declarations for sorted collection

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

$Log:	SortedCltn.h,v $
 * Revision 1.3  88/02/04  13:00:20  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:23  keith
 * Remove pre-RCS modification history
 * 
*/

#include "OrderedCltn.h"

extern const Class class_SortedCltn;
overload SortedCltn_reader;

class SortedCltn : public OrderedCltn {
protected:
	SortedCltn(fileDescTy&,SortedCltn&);
	SortedCltn(istream&,SortedCltn&);
	friend	void SortedCltn_reader(istream& strm, Object& where);
	friend	void SortedCltn_reader(fileDescTy& fd, Object& where);
public:
	SortedCltn(unsigned size =CLTN_DEFAULT_CAPACITY);
	SortedCltn(const SortedCltn&);
	bool operator!=(const SortedCltn& a)	{ return !(*this==a); }
	void operator=(const SortedCltn&);
	bool operator==(const SortedCltn&);
	virtual Object* add(const Object&);
	virtual Object* addAfter(const Object& ob, const Object& newob);
	virtual Object* addAllLast(const OrderedCltn&);
	virtual Object* addBefore(const Object& ob, const Object& newob);
	virtual Object* addLast(const Object& ob);
	virtual void atAllPut(const Object& ob);
	virtual int indexOfSubCollection(const SeqCltn& cltn, int start=0);
	virtual const Class* isA();
	virtual SeqCltn operator&(const SeqCltn& cltn);
	virtual void replaceFrom(int start, int stop, const SeqCltn& replacement, int startAt =0);
	virtual void sort();
};

#endif
