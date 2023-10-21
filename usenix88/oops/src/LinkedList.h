#ifndef	LINKEDLIST_H
#define	LINKEDLIST_H

/* LinkedList.h -- declarations for singly-linked list

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

$Log:	LinkedList.h,v $
 * Revision 1.3  88/02/04  12:59:33  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:38  keith
 * Remove pre-RCS modification history
 * 
*/

#include "SeqCltn.h"
#include "Link.h"

extern const Class class_LinkedList;
overload LinkedList_reader;

class LinkedList : public SeqCltn {
	Link* firstLink;	// pointer to first Link of list 
	Link* lastLink;		// pointer to last Link of list 
	unsigned count;		// count of items on list 
	void errDblLnk(const char* fn, const Link& lnk);
	void errEmpty(const char* fn);
	void errNotFound(const char* fn, const Object& ob);
protected:
	LinkedList(fileDescTy&,LinkedList&);
	LinkedList(istream&,LinkedList&);
	friend	void LinkedList_reader(istream& strm, Object& where);
	friend	void LinkedList_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	LinkedList();
	bool operator!=(const LinkedList& a)	{ return !(*this==a); }
	bool operator==(const LinkedList&);
	Object*& operator[](int i);
	virtual Object* add(const Object&);
	virtual Collection& addContentsTo(Collection& cltn);
	virtual Object* addFirst(const Object& ob);
	virtual Object* addLast(const Object& ob);
	virtual Object*& at(int i);
	virtual void atAllPut(const Object& ob);	// should not implement 
	virtual void deepenShallowCopy();
	virtual Object* doNext(Iterator&);
	virtual Object* first();
	virtual unsigned hash();
 	virtual int indexOf(const Object& ob);
	virtual int indexOfSubCollection(const SeqCltn& cltn, int start=0);	// shouldNotImplement 
	virtual const Class* isA();
	virtual bool isEmpty();
	virtual bool isEqual(const Object&);
	virtual Object* last();
	virtual unsigned occurrencesOf(const Object&);
	virtual void printOn(ostream& strm);
	virtual Object* remove(const Object&);
	virtual Object* removeFirst();
	virtual Object* removeLast();
	virtual void replaceFrom(int start, int stop, const SeqCltn& replacement, int startAt =0);	// shouldNotImplement 
	virtual void reSize(unsigned newSize);
	virtual unsigned size();
	virtual const Class* species();
};

#endif
