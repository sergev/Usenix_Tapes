#ifndef	STACK_H
#define	STACK_H

/* Stack.h -- declarations for class Stack

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

$Log:	Stack.h,v $
 * Revision 1.3  88/02/04  13:00:24  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:27  keith
 * Remove pre-RCS modification history
 * 
*/

#include "OrderedCltn.h"

extern const Class class_Stack;
overload Stack_reader;

class Stack : public SeqCltn {
	OrderedCltn contents;
protected:
	Stack(fileDescTy&,Stack&);
	Stack(istream&,Stack&);
	friend	void Stack_reader(istream& strm, Object& where);
	friend	void Stack_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Stack(unsigned size =CLTN_DEFAULT_CAPACITY) : contents(size) {}
	Stack(const Stack& s) : contents(*((OrderedCltn*)&s)) {}
	bool operator==(const Stack& s)
		{ return contents == ((Stack*)&s)->contents; }
	bool operator!=(const Stack& s)	{ return !(*this==s); }
	void operator=(const Stack& s)	{ contents = ((Stack*)&s)->contents; }
	Object*& operator[](int i)		{ return contents.at(size()-i-1); }
	void push(const Object& ob)	{ contents.addLast(ob); }
	Object* pop()			{ return contents.removeLast(); }
	Object* top()			{ return contents.last(); }
	virtual Object* add(const Object& ob);
	virtual Collection& addContentsTo(Collection& cltn);
	virtual Object*& at(int i);
	virtual unsigned capacity();
	virtual void deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEmpty();
	virtual bool isEqual(const Object& ob);
	virtual Object* last();
	virtual void printOn(ostream& strm);
	virtual void reSize(unsigned newSize);
	virtual Object* removeLast();
	virtual unsigned size();
	virtual const Class* species();
};

#endif
