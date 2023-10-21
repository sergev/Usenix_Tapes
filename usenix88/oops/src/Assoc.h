#ifndef	ASSOC_H
#define	ASSOC_H

/* Assoc.h -- declarations for key-value association

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

$Log:	Assoc.h,v $
 * Revision 1.3  88/02/04  12:54:11  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:37:29  keith
 * Remove pre-RCS modification history
 * 
*/

#include "LookupKey.h"

extern const Class class_Assoc;
overload Assoc_reader;

class Assoc: public LookupKey {
	Object* avalue;
protected:
	Assoc(fileDescTy&,Assoc&);
	Assoc(istream&,Assoc&);
	friend	void Assoc_reader(istream& strm, Object& where);
	friend	void Assoc_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Assoc(const Object& newKey =*nil, const Object& newValue =*nil);
	virtual void deepenShallowCopy();
	virtual const Class* isA();
	virtual void printOn(ostream& strm);
	virtual Object* value();
	virtual Object* value(const Object& newvalue);
};

#endif
