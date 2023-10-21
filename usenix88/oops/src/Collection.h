#ifndef	COLLECTION_H
#define	COLLECTION_H

/* Collection.h -- declarations for abstract Collection class

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

$Log:	Collection.h,v $
 * Revision 1.3  88/02/04  12:55:08  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:38:18  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

const unsigned CLTN_DEFAULT_CAPACITY	= 16;	// default initial collection capacity 
const unsigned CLTN_EXPANSION_INCREMENT = 32;	// collection (OrderedCltn) expansion increment 
const unsigned CLTN_EXPANSION_FACTOR = 2;	// collection (Set,Bag,Dictionary) expansion factor 

class ArrayOb;
class Bag;
class Iterator;
class Heap;
class OrderedCltn;
class Set;
class SortedCltn;

extern const Class class_Collection;
overload Collection_reader;

class Collection: public Object {	// abstract class 
protected:
	Collection() {}
	Collection(fileDescTy&,Collection&) {}
	Collection(istream&,Collection&) {}
	friend	void Collection_reader(istream& strm, Object& where);
	friend	void Collection_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	ArrayOb	asArrayOb();
	Bag		asBag();
	Heap		asHeap();
	OrderedCltn	asOrderedCltn();
	Set		asSet();
	SortedCltn	asSortedCltn();
	virtual	Object*		add(const Object&);
	virtual const Collection& addAll(const Collection&);
	virtual Collection&	addContentsTo(Collection&);
	virtual Object*&	at(int);
	virtual void		deepenShallowCopy();
	virtual	Object*		doNext(Iterator& pos);
	virtual	void		doReset(Iterator& pos);
	virtual bool		includes(const Object&);
	virtual const Class*	isA();
	virtual bool		isEmpty();
	virtual unsigned	occurrencesOf(const Object&);
	virtual Object*		remove(const Object&);
	virtual const Collection& removeAll(const Collection&);
	virtual Object*		shallowCopy();		// shouldNotImplement
	virtual unsigned	size();
};

#include "Iterator.h"

#endif
