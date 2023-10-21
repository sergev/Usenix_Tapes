#ifndef	ITERATOR_H
#define	ITERATOR_H

/* Iterator.h -- Declarations for Collection Iterators

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
	December, 1987

$Log:	Iterator.h,v $
 * Revision 1.3  88/02/04  12:59:26  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:24  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

class Collection;
class Link;

extern const Class class_Iterator;
overload Iterator_reader;

class Iterator: public Object {
	Collection* cltn;	// Collection being iterated over
public:
	int	index;		// index of next Object
	Object*	ptr;		// pointer to current Object or NULL
	unsigned num;		// object number, used by Bags
protected:
	Iterator(fileDescTy&,Iterator&);
	Iterator(istream&,Iterator&);
	friend	void Iterator_reader(istream& strm, Object& where);
	friend	void Iterator_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Iterator(const Collection&);
	void reset();		// reset to beginning of Collection
	Object* operator++();	// advance to next object in Collection
	Collection* collection()		{ return cltn; }
	bool operator==(const Iterator&);
	bool operator!=(const Iterator& a)	{ return !(*this==a); }
	virtual Object*	copy();				// shallowCopy()
	virtual void	deepenShallowCopy();		// copy with cltn->deepCopy()
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool	isEqual(const Object&);
	virtual void	printOn(ostream& strm);
	virtual Object* shallowCopy();			// copy with cltn->copy()
	virtual const Class* species();
};

#define DO(cltn,cls,arg)\
{ cls arg; Iterator DO_pos(cltn); while (arg = (cls)DO_pos++) {
#define OD }}

#endif
