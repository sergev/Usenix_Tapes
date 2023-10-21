#ifndef	LOOKUPKEY_H
#define	LOOKUPKEY_H

/* LookupKey.h -- declarations for Dictionary LookupKey

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

$Log:	LookupKey.h,v $
 * Revision 1.3  88/02/04  12:59:36  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:43  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_LookupKey;
overload LookupKey_reader;

class LookupKey: public Object {
	Object* akey;
protected:
	LookupKey(fileDescTy&,LookupKey&);
	LookupKey(istream&,LookupKey&);
	friend	void LookupKey_reader(istream& strm, Object& where);
	friend	void LookupKey_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	LookupKey(const Object& newKey =*nil);
	virtual int compare(const Object&);
	virtual void deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual Object* key();
	virtual Object* key(const Object& newkey);
	virtual void printOn(ostream& strm);
	virtual Object* value();
	virtual Object* value(const Object& newvalue);
};

#endif
