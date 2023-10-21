#ifndef	INTEGER_H
#define	INTEGER_H

/* Integer.h -- declarations for Integer object

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

$Log:	Integer.h,v $
 * Revision 1.3  88/02/04  12:59:23  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:20  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_Integer;
overload Integer_reader;

class Integer: public Object {
	int val;
	void parseInteger(istream& strm)	{ strm >> val; }
protected:
	Integer(fileDescTy&,Integer&);
	Integer(istream&,Integer&);
	friend	void Integer_reader(istream& strm, Object& where);
	friend	void Integer_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Integer(int v =0)		{ val = v; }
	Integer(istream&);
	int value()			{ return val; }
	int value(int newval)		{ return val = newval; }
	virtual int compare(const Object&);
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual void scanFrom(istream& strm);
	virtual const Class* species();
};

#endif
