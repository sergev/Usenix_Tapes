#ifndef	OBJECT_H
#define	OBJECT_H

/* Object.h -- declarations for class Object and class Class

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
	
Define class Object, the root of the class tree, and class Class, the
class that describes class objects.

$Log:	Object.h,v $
 * Revision 1.3  88/02/04  12:59:46  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:02:28  keith
 * Remove pre-RCS modification history
 * 
*/

#define bool int
#include <stdio.h>
#include <stream.h>
#include <errors.h>
#undef bool

typedef int bool;	// int for compatibility with X V11
const bool YES = 1;
const bool NO = 0;
typedef int fileDescTy;

overload ABS;
inline int	ABS(int x)	{ return x >= 0 ? x : -x; }
inline long	ABS(long x)	{ return x >= 0 ? x : -x; }
//inline float	ABS(float x)	{ return x >= 0 ? x : -x; }
inline double	ABS(double x)	{ return x >= 0 ? x : -x; }
overload MAX;
inline int	MAX(int a,int b)	{ return a >= b ? a : b; }
inline long	MAX(long a,long b)	{ return a >= b ? a : b; }
//inline float	MAX(float a,float b)	{ return a >= b ? a : b; }
inline double	MAX(double a,double b)	{ return a >= b ? a : b; }
inline unsigned int	MAX(unsigned int a, unsigned int b)	{ return a >= b ? a : b; }
inline unsigned long	MAX(unsigned long a, unsigned long b)	{ return a >= b ? a : b; }
overload MIN;
inline int	MIN(int a,int b)	{ return a <= b ? a : b; }
inline long	MIN(long a,long b)	{ return a <= b ? a : b; }
//inline float	MIN(float a,float b)	{ return a <= b ? a : b; }
inline double	MIN(double a,double b)	{ return a <= b ? a : b; }
inline unsigned int	MIN(unsigned int a, unsigned int b)	{ return a <= b ? a : b; }
inline unsigned long	MIN(unsigned long a, unsigned long b)	{ return a <= b ? a : b; }

/*
The DEFINE_CLASS preprocessor macro composes names by concatenating
the "classname" argument with other character strings.  This is done
by separating the two with an empty comment.  If this doesn't work on
your C preprocessor, try using a \<newline> as a separator (see the
file DEFCLASS.h).
*/

#define DEFINE_CLASS(classname,basename,version,identification,initor1,initor2)	\
overload classname/**/_reader;						\
static void classname/**/_reader(istream& strm, Object& where) { new classname(strm,*(classname*)&where); }\
static void classname/**/_reader(fileDescTy& fd, Object& where) { new classname(fd,*(classname*)&where); }\
const Class class_/**/classname = Class( class_/**/basename, "classname",	\
 version, identification, sizeof(classname), classname/**/_reader, classname/**/_reader,\
 initor1, initor2 );							\
const Class* classname::isA()	{ return &class_/**/classname; }

class Object;
class Class;
class IdentDict;
class OrderedCltn;

class Object {
protected:
	Object() {}
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	void	assertArgClass(const Object& ob, const Class& expect, const char* fname);	// validate argument class
	void	assertArgSpecies(const Object& ob, const Class& expect, const char* fname);	// validate argument species
	const	Class* baseClass();				// return pointer to base class descriptor
	const	char* classIdent();				// return class RCS identification
	const	char* className();				// return class name 
	unsigned classVersion();				// return class version 
	void	derivedClassResponsibility(const char*);	// unimplemented virtual function 
	void	invalidArgClass(const Object& ob, const Class& expect, const char* fname);	// invalid object class error
	void	invalidArgSpecies(const Object& ob, const Class& expect, const char* fname);	// invalid object species error
	bool	isKindOf(const Class&);			// YES if MemberOf class or a superclass 
	bool	isMemberOf(const Class& clid)		{ return isA()==&clid; }
	bool	isSame(const Object& ob)		{ return this==&ob; }
	bool	isSpecies(const Class& clid)		{ return species()==&clid; }
	void	shouldNotImplement(const char*);	// class cannot implement this function 
	void	storeOn(fileDescTy&);			// store object in binary on file
	void	storeOn(ostream&);			// store object on stream 
	virtual Object* addDependent(const Object&);	// add dependent object 
	virtual unsigned capacity();			// subclass capacity 
	virtual void changed();				// notify dependents of change 
	virtual void changed(const Object&);		// notify dependents of change 
	virtual int compare(const Object&);		// compare objects 
	virtual Object* copy();				// copy defaulted as deepCopy 
	virtual Object* deepCopy();			// copy with distinct instance variables 
	virtual void deepenShallowCopy();		// convert shallow copy to deep copy 
	virtual OrderedCltn& dependents();		// return list of dependent objects 
	virtual void destroyer();			// destroy object
	virtual unsigned hash();			// calculate object hash 
	virtual const Class* isA();			// return class descriptor address 
	virtual bool isEqual(const Object&);		// equality test 
	virtual void scanFrom(istream& strm);		// parse object from stream 
	virtual void printOn(ostream& strm);		// print object on stream 
	virtual void release();				// remove all dependent objects 
	virtual Object* removeDependent(const Object&);	// remove dependent object 
	virtual Object* shallowCopy();			// copy with shared instance variables 
	virtual unsigned size();			// # of objects in array/container subclass
	virtual const Class* species();			// return species class descriptor address 
	virtual void update(const Object&, const Object&);	// object change notification 
};

typedef void (*initorTy)(const Class&);

class Class : public Object {	// class descriptor object 
	const Class* superClass;	// pointer to this class's base class (superclass) object 
	Class* nextClass;		// link for list of all Class objects 
	const char* class_name;		// class name 
	const char* class_ident;	// class RCS identification $Header: Object.h,v 1.3 88/02/04 12:59:46 keith Locked $
	unsigned class_version;		// class version number 
	unsigned def_version;		/* DEFINE_CLASS version parameter */
	unsigned inst_size;		// sizeof instance variables 
	void (*inst_reader)(istream&,Object&);		// object reader function 
	void (*inst_binreader)(fileDescTy&,Object&); 	// binary reader function
	initorTy class_initor2;		// phase 2 class initor
	unsigned class_number;		// class number, used by storeOn()
	unsigned classNumber(unsigned n)	{ return class_number = n; }
	friend void Object::storeOn(fileDescTy&);	
	friend void Object::storeOn(ostream&);
	friend void Object::storer(fileDescTy&);
	friend void Object::storer(ostream&);
	friend void OOPS__classInit();
public:
	Class(const Class& super, const char* name,
		unsigned version, const char* ident, unsigned size,
		void (*reader)(istream&,Object&), 
		void (*binreader)(fileDescTy&,Object&), 
		initorTy initor1 =0, initorTy initor2 =0);
//	Class(fileDescTy&,Class&) {}	// cfront 1.2.1 bug
//	Class(istream&,Class&) {}	// cfront 1.2.1 bug
	const Class* baseClass()		{ return superClass; }
	const char* classIdent()		{ return class_ident; }
	const char* className()			{ return class_name; }
	unsigned classNumber()			{ return class_number; }
	unsigned classVersion()			{ return class_version; }
	void reader(istream& strm, Object& where)	{ (*inst_reader)(strm,where); }
	void reader(fileDescTy& fd, Object& where)	{ (*inst_binreader)(fd,where); }
	virtual int compare(const Object&);	// compare class names 
	virtual Object* deepCopy();		// shouldNotImplement 
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object& ob);
	virtual void printOn(ostream& strm);
	virtual Object* shallowCopy();		// shouldNotImplement 
	virtual unsigned size();
	virtual void storer(ostream&);
	virtual void storer(fileDescTy&);
};

extern Class* allClasses;	// header of list of all Classes 
extern const Class class_Class;	// Class of Classes 
extern const Class class_Object;
extern const Object* const nil;	// pointer to sole instance of nil object 

inline const Class* Object::baseClass()	{ return isA()->baseClass(); }

/*inline*/ static const char* Object::classIdent()	{ return isA()->classIdent(); }	// static due to CC bug

/*inline*/ static const char* Object::className()	{ return isA()->className(); }	// static due to CC bug

inline unsigned Object::classVersion()	{ return isA()->classVersion(); }
		
inline istream& operator>>(istream& strm, Object& ob)
{
	ob.scanFrom(strm);
	return strm;
}

inline ostream& operator<<(ostream& strm, const Object& ob)
{
	ob.printOn(strm);
	return strm;
}

overload readFrom;
extern Object* readFrom(istream&, const char* expectedClass ="", Object& where =*(Object*)0);	// read object from stream 
extern Object* readFrom(fileDescTy&, const char* expectedClass ="", Object& where =*(Object*)0);	// read object in binary from file

extern bool OOPS__Initialized;
inline bool OOPSInitialized() {	return OOPS__Initialized; }

extern void setOOPSerror(int error, int sev ...);			// declare an OOPS error condition

extern void invalidArgClass(const Object& ob, const Class& expect, const char* fname);	// invalid argument class error
extern void invalidArgSpecies(const Object& ob, const Class& expect, const char* fname);// invalid argument species error
extern void invalidClass(const Object& ob, const Class& expect);			// invalid object class error
extern void invalidSpecies(const Object& ob, const Class& expect);			// invalid object species error

inline void assertArgClass(const Object& ob, const Class& expect, const char* fname)
{
	if (!((ob).isKindOf(expect))) ::invalidArgClass(ob,expect,fname);
}

inline void assertArgSpecies(const Object& ob, const Class& expect, const char* fname)
{
	if (!((ob).isSpecies(expect))) ::invalidArgSpecies(ob,expect,fname);
}

inline void Object::assertArgClass(const Object& ob, const Class& expect, const char* fname)
{
	if (!((ob).isKindOf(expect))) this->invalidArgClass(ob,expect,fname);
}

inline void Object::assertArgSpecies(const Object& ob, const Class& expect, const char* fname)
{
	if (!((ob).isSpecies(expect))) this->invalidArgSpecies(ob,expect,fname);
}

inline void assertClass(const Object& ob, const Class& expect)
{
	if (!((ob).isKindOf(expect))) ::invalidClass(ob,expect);
}

inline void assertSpecies(const Object& ob, const Class& expect)
{
	if (!((ob).isSpecies(expect))) ::invalidSpecies(ob,expect);
}

#endif
