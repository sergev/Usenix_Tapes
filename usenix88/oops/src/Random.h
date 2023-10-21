#ifndef	RANDOM_H
#define	RANDOM_H

/* Random.h -- declarations for pseudo-random number generator

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

$Log:	Random.h,v $
 * Revision 1.3  88/02/04  12:59:56  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:40:39  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_Random;
overload Random_reader;

class Random: public Object {
	long	randx;
protected:
	Random(fileDescTy&,Random&);
	Random(istream&,Random&);
	friend	void Random_reader(istream& strm, Object& where);
	friend	void Random_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Random();
	Random(long seed)		{ randx = seed; }
	float next();
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual const Class* isA();
	virtual void printOn(ostream& strm);
};

#endif
