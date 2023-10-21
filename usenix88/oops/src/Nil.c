/* Nil.c -- implementation of the nil object

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	September, 1985

Function:
	
Declarations and member functions for the nil object.

$Log:	Nil.c,v $
 * Revision 1.3  88/02/04  12:59:38  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:55  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_Nil;

class Nil : public Object {
public:	
	Nil() {}
	Nil(fileDescTy&,Nil&) {}
	Nil(istream&,Nil&) {}
	virtual int compare(const Object&);	// compare objects 
	virtual Object* copy();			// copy returns nil 
	virtual Object* deepCopy();		// copy returns nil 
	virtual unsigned hash();		// calculate object hash 
	virtual const Class* isA();
	virtual bool isEqual(const Object&);	// equality test 
	virtual void printOn(ostream& strm);
	virtual Object* shallowCopy();		// copy returns nil 
	virtual void storer(ostream&);		// shouldNotImplement 
	virtual void storer(fileDescTy&);	// shouldNotImplement 
};

#define	THIS	Nil
#define	BASE	Object
DEFINE_CLASS(Nil,Object,1,"$Header: Nil.c,v 1.3 88/02/04 12:59:38 keith Exp $",NULL,NULL);
static const Nil nil_object;				// the nil object 
extern const Object* const nil = (Object*)&nil_object; // pointer to the nil object 

bool Nil::isEqual(const Object& ob) {	return (&ob==nil); }

unsigned Nil::hash() { return 0; }

int Nil::compare(const Object& ob)
{
	assertArgSpecies(ob,class_Nil,"compare");
	return 0;
}

Object* Nil::copy() { return nil; }

Object* Nil::shallowCopy() { return nil; }

Object* Nil::deepCopy() { return nil; }

void Nil::printOn(ostream& strm) { strm << "NIL"; }

void Nil::storer(ostream&)
/*
The Nil object is always implicitly stored as object number zero; all
references to nil are stored as @0.
*/
{
	shouldNotImplement("storer");
}

void Nil::storer(fileDescTy&)
{
	shouldNotImplement("storer");
}
