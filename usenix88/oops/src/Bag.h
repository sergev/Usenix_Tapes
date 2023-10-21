#ifndef	BAG_H
#define	BAG_H

/* Bag.h -- declarations for Set of Objects with possible duplicates

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

$Log:	Bag.h,v $
 * Revision 1.3  88/02/04  12:54:56  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:37:47  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Collection.h"
#include "Dictionary.h"

extern const Class class_Bag;
overload Bag_reader;

class Bag: public Collection {
	unsigned count;
	Dictionary contents;
protected:
	Bag(fileDescTy&,Bag&);
	Bag(istream&,Bag&);
	friend	void Bag_reader(istream& strm, Object& where);
	friend	void Bag_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Bag(unsigned size =CLTN_DEFAULT_CAPACITY);
	Bag(const Bag&);
	Object* addWithOccurrences(const Object&, unsigned);
	bool operator!=(const Bag& a)			{ return !(*this==a); }
	void operator=(const Bag&);
	bool operator==(const Bag&);
	virtual	Object*	add(const Object&);
	virtual Collection& addContentsTo(Collection&);
	virtual Object*& at(int);
	virtual unsigned capacity();
	virtual void	deepenShallowCopy();
	virtual Object*	doNext(Iterator&);
	virtual unsigned hash();
	virtual const Class*	isA();
	virtual bool	isEqual(const Object&);
	virtual unsigned occurrencesOf(const Object&);
	virtual void	printOn(ostream& strm);
	virtual void	reSize(unsigned);
	virtual Object*	remove(const Object&);
	virtual unsigned size();
	virtual const Class*	species();
};

#endif
