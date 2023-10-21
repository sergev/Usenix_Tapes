/* Stack.c -- implementation of class Stack

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
	October, 1985

Function:
	
Member function definitions for class Stack.

$Log:	Stack.c,v $
 * Revision 1.2  88/01/16  23:41:25  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Stack.h"
#include "oopsIO.h"

#define	THIS	Stack
#define	BASE	SeqCltn
DEFINE_CLASS(Stack,SeqCltn,1,"$Header: Stack.c,v 1.2 88/01/16 23:41:25 keith Exp $",NULL,NULL);

Object* Stack::add(const Object& ob)	{ return contents.add(ob); }

Collection& Stack::addContentsTo(Collection& cltn)
{
	return contents.addContentsTo(cltn);
}

Object*& Stack::at(int i)		{ return contents.at(size()-i-1); }

unsigned Stack::capacity()	{ return contents.capacity(); }

void Stack::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	contents.deepenShallowCopy();
}

unsigned Stack::hash()		{ return contents.hash(); }

bool Stack::isEmpty()		{ return contents.size()==0; }

bool Stack::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Stack) && contents.isEqual(((Stack*)&ob)->contents);
}

const Class* Stack::species()	{ return &class_Stack; }

Object* Stack::last()		{ return contents.last(); }

void Stack::printOn(ostream& strm)
{
	strm << className() << "[";
	for (register unsigned i=size(); i>0; i--) {
		if(i<size()) strm << "\n";
		contents.at(i-1)->printOn(strm);
	}
	strm << "]\n";
}

void Stack::reSize(unsigned newSize) { contents.reSize(newSize); }

Object* Stack::removeLast()	{ return contents.removeLast(); }

unsigned Stack::size()		{ return contents.size(); }

static unsigned stack_capacity;

Stack::Stack(istream& strm, Stack& where) : contents((strm >> stack_capacity, stack_capacity))
{
	this = &where;
	unsigned n;
	strm >> n;		// read Stack size 
	while (n--) add(*readFrom(strm));
}

void Stack::storer(ostream& strm)
{
	Object::storer(strm);
	strm << contents.capacity() << " " << contents.size();
	DO(contents,Object*,ob) ob->storeOn(strm); OD
}

Stack::Stack(fileDescTy& fd, Stack& where) : contents((readBin(fd,stack_capacity), stack_capacity))
{
	this = &where;
	unsigned n;
	readBin(fd,n);
	while (n--) add(*readFrom(fd));
}

void Stack::storer(fileDescTy& fd) 
{
	Object::storer(fd);
	storeBin(fd,contents.capacity());
	storeBin(fd,contents.size());
	DO(contents,Object*,ob) ob->storeOn(fd); OD
}
