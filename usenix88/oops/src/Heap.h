#ifndef	HEAP_H
#define	HEAP_H

/* Heap.h -- declarations for abstract heap          

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

$Log:	Heap.h,v $
 * Revision 1.3  88/02/04  12:59:17  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:11  keith
 * Remove pre-RCS modification history
 * 
*/
#include "Collection.h"
#include "OrderedCltn.h"
#include "ArrayOb.h"

extern const Class class_Heap;          
overload Heap_reader;

class Heap: public SeqCltn {
	int endIndex;
	ArrayOb contents;

	void bubbleUp(int);
	void bubbleUpMax(int);
	void bubbleUpMin(int);
	int child(int i)	{ return( (i << 1) + 1 ); }
	int descendents(int);
	void errEmpty(const char* fn);
	int grandchild(int i)	{ return( (child(i) << 1) + 1 ); }
	int largest(int,int);
	int level(int);
	int parent(int i)		{return( (i - 1) >> 1 );}
	Object* removeAtIndex(int i);
	int smallest(int,int);
	void swap(int a,int b)    	{Object* temp = contents[a];
					 contents[a] = contents[b]; 
					 contents[b] = temp;}
	void trickleDown(int);
	void trickleDownMax(int);
	void trickleDownMin(int);

protected:
	Heap(fileDescTy&,Heap&);
	Heap(istream&,Heap&);
	friend	void Heap_reader(istream& strm, Object& where);
	friend	void Heap_reader(fileDescTy& fd, Object& where);

public:
	Heap(int size=CLTN_DEFAULT_CAPACITY);
	Heap(const Heap&);
	Heap(const ArrayOb&);
	void operator=(const Heap&);
	bool operator==(const Heap&);
	bool operator!=(const Heap& a)    {  return !(*this==a);  }
	virtual Object* add(const Object&);
	virtual Collection& addContentsTo(Collection&);
	virtual Object*& at(int index);
	virtual void atAllPut(const Object& ob);  // should not implement
	unsigned capacity();
	virtual void deepenShallowCopy();
	virtual Object* doNext(Iterator&);
	virtual Object* first();
	virtual unsigned hash();
	virtual int indexOf(const Object& ob);   // should not implement
	virtual int indexOfSubCollection(const SeqCltn& cltn, int start=0);
		// should not implement
	virtual const Class* isA();
	virtual bool isEmpty();
	virtual bool isEqual(const Object&);
	virtual Object* last();
	virtual unsigned occurrencesOf(const Object&);
	virtual void printOn(ostream& strm);
	virtual Object* remove(const Object&);
	virtual Object* removeFirst();
	virtual Object* removeLast();
	virtual void reSize(int newSize);
	virtual unsigned size();
	OrderedCltn sort();
	virtual const Class* species();
};

#endif
