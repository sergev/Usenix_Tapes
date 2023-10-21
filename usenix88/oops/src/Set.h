#ifndef	SET_H
#define	SET_H

/* Set.h -- declarations for hash tables

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

$Log:	Set.h,v $
 * Revision 1.3  88/02/04  13:00:15  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:14  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Collection.h"
#include "ArrayOb.h"

extern const Class class_Set;
overload Set_reader;

class Dictionary;
class IdentDict;

class Set: public Collection {
	unsigned count;		// number of objects in set 
	unsigned nbits;		// log base 2 of contents.capacity() 
protected:
	unsigned mask;		// contents.capacity()-1 
	ArrayOb contents;	// array of set objects 
	unsigned setCapacity(unsigned);	// compute set allocation size 
	int h(unsigned long);	// convert hash key into contents index 
	Object* findObjectWithKey(const Object&);
	virtual int findIndexOf(const Object&);
protected:
	Set(fileDescTy&,Set&);
	Set(istream&,Set&);
	friend	void Set_reader(istream& strm, Object& where);
	friend	void Set_reader(fileDescTy& fd, Object& where);
public:
	Set(unsigned size =CLTN_DEFAULT_CAPACITY);
	Set(const Set&);
	void operator=(const Set&);
	bool operator==(const Set&);
	bool operator!=(const Set& a)		{ return !(*this==a); }
	Set operator&(const Set&);		// intersection 
	Set operator|(const Set&);		// union 
	Set operator-(const Set&);		// difference 
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
