#ifndef	ASSOCINT_H
#define	ASSOCINT_H

/* AssocInt.h -- declarations for key-Integer association

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

$Log:	AssocInt.h,v $
 * Revision 1.3  88/02/04  12:54:50  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:37:34  keith
 * Remove pre-RCS modification history
 * 
*/

#include "LookupKey.h"
#include "Integer.h"

extern const Class class_AssocInt;
overload AssocInt_reader;

class AssocInt: public LookupKey {
	Integer avalue;
protected:
	AssocInt(fileDescTy&,AssocInt&);
	AssocInt(istream&,AssocInt&);
	friend	void AssocInt_reader(istream& strm, Object& where);
	friend	void AssocInt_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	AssocInt(const Object& newKey =*nil, int newValue =0);
	virtual void deepenShallowCopy();
	virtual const Class* isA();
	virtual void printOn(ostream& strm);
	virtual Object* value();
	virtual Object* value(const Object& newValue);
};

#endif
