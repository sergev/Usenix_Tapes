#ifndef	IDENTDICT_H
#define	IDENTDICT_H

/* IdentDict.h -- declarations for Identity Dictionary

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

$Log:	IdentDict.h,v $
 * Revision 1.3  88/02/04  12:59:20  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:16  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Dictionary.h"

extern const Class class_IdentDict;
overload IdentDict_reader;

class LookupKey;
class OrderedCltn;

class IdentDict: public Dictionary {
	virtual int findIndexOf(const Object&);
protected:
	IdentDict(fileDescTy&,IdentDict&);
	IdentDict(istream&,IdentDict&);
	friend	void IdentDict_reader(istream& strm, Object& where);
	friend	void IdentDict_reader(fileDescTy& fd, Object& where);
public:
	IdentDict(unsigned size =CLTN_DEFAULT_CAPACITY);
	IdentDict(const IdentDict&);
	void operator=(const IdentDict&);
	virtual LookupKey& assocAt(const Object& key);
	virtual Object* atKey(const Object& key);
	virtual Object* atKey(const Object& key, const Object& newValue);
	virtual bool includesKey(const Object& key);
	virtual const Class* isA();
};

#endif
