/* Class.c -- implementation of class descriptor objects

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
	
Functions pertaining to class Class.  Most of these are defined inline
in object.h.

Modification History:

$Log:	Class.c,v $
 * Revision 1.3  88/02/04  12:55:05  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  22:40:31  keith
 * Correct Class::Class() to initialize class_ident
 * Remove pre-RCS modification history
 * 
 * Revision 1.1.1.1  88/01/16  22:33:26  keith
 * Corrected Class::Class() to initialize class_ident
 * Removed pre-RCS modification history
 * 
 * Revision 1.1  88/01/14  23:37:18  keith
 * Modified for RCS maintenance
 * 

*/

#include "Object.h"
#include "String.h"
#include "Dictionary.h"
#include "Assoc.h"

#define	THIS	Class
#define	BASE	Object
/*DEFINE_CLASS(Class,Object,1,"$Header: Class.c,v 1.3 88/02/04 12:55:05 keith Exp $",NULL,NULL);	// cfront 1.2.1 bug */
extern const int OOPS_RDUNKCLASS;
overload Class_reader;
static void Class_reader(istream& /*strm*/, Object& /*where*/)
{
	setOOPSerror(OOPS_RDUNKCLASS,DEFAULT,"Class");
}
static void Class_reader(fileDescTy& /*fd*/, Object& /*where*/)
{
	setOOPSerror(OOPS_RDUNKCLASS,DEFAULT,"Class");
}
const Class class_Class = Class( class_Object, "Class", 1, "$Header: Class.c,v 1.3 88/02/04 12:55:05 keith Exp $", sizeof(Class), Class_reader, Class_reader, NULL, NULL);

const Class* Class::isA()	{ return &class_Class; }

// head of the list of all Classes, built by the static constructor of class Class 
Class* allClasses =0;

Dictionary classDictionary;	// Dictionary of all Classes 

Class::Class(const Class& super, const char* name, unsigned version, const char* ident, unsigned size, void (*reader)(istream&,Object&), void (*binreader)(fileDescTy&,Object&), initorTy initor1, initorTy initor2)
{
	superClass = &super;
	nextClass = allClasses; allClasses = this;
	class_name = name;
	class_ident = ident;
	def_version = version;
	class_version = 0;
	inst_size = size;
	inst_reader = reader;
        inst_binreader = binreader;
	class_number = 0;
	class_initor2 = initor2;
//	if (initor1 != 0) initor1(*this);	MASSCOMP cc bug
	register initorTy init = initor1;
	if (init != 0) init(*this);
}

int Class::compare(const Object& ob)	// compare Class names 
{
	assertArgSpecies(ob,class_Class,"compare");
	return strcmp(className(),((Class*)&ob)->className());
}

Object* Class::deepCopy()
{
	shouldNotImplement("deepCopy");
	return 0;
}

unsigned Class::hash()	{ return (unsigned)this; }

bool Class::isEqual(const Object& ob)	{ return this == (Class*)&ob; }

void Class::printOn(ostream& strm)	{ strm << class_name; }

Object* Class::shallowCopy()
{
	shouldNotImplement("shallowCopy");
	return 0;
}

unsigned Class::size()	{ return inst_size; }
	
void OOPS__classInit()	// class Initialization
{
	register Class* clp = allClasses;	// calculate class version numbers
	while (clp != NULL) {
		register const Class* clq = clp;
		while (clq != NULL) {
			clp->class_version = (clp->class_version << 4)
				^ (clp->class_version >> (8*sizeof(int)-4) & 0xF)
				^ clq->def_version;
			clq = clq->baseClass();
		}
		clp = clp->nextClass;
	}
	clp = allClasses;	// add classes to classDictionary
	while (clp != NULL) {
		classDictionary.add(*new Assoc(*new String(clp->className()), *clp));
		clp = clp->nextClass;
	}
	clp = allClasses;	// call the initor2 functions
	while (clp != NULL) {
		if (clp->class_initor2 != 0) (*clp->class_initor2)(*clp);
		clp = clp->nextClass;
	}
}

void Class::storer(ostream&) { shouldNotImplement("storer"); }

void Class::storer(fileDescTy&) { shouldNotImplement("storer"); }
