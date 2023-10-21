#ifndef	FLOAT_H
#define	FLOAT_H

/* Float.h -- declarations for Float object

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

$Log:	Float.h,v $
 * Revision 1.3  88/02/04  12:59:11  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:00  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_Float;
overload Float_reader;

class Float: public Object {
	double val;
	void parseFloat(istream& strm)	{ strm >> val; }
protected:
	Float(fileDescTy&,Float&);
	Float(istream&,Float&);
	friend	void Float_reader(istream& strm, Object& where);
	friend	void Float_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Float(double v =0)		{ val = v; }
	Float(istream&);
	double value()			{ return val; }
	double value(double newval)	{ return val = newval; }
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
