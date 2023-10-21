/* Object.c -- implementation of class Object

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

Operations applicable to all objects.
	
$Log:	Object.c,v $
 * Revision 1.3  88/02/04  12:59:41  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:59  keith
 * Remove pre-RCS modification history
 * 
*/

#include <ctype.h>
#include "oopsconfig.h"
#include "Object.h"
#include "Dictionary.h"
#include "LookupKey.h"
#include "IdentDict.h"
#include "String.h"
#include <string.h>
#include "Assoc.h"
#include "AssocInt.h"
#include "OrderedCltn.h"
#include "oopsIO.h"

extern const int OOPS_DRVDCLASSRSP,OOPS_ILLEGALMFCN,OOPS_BADARGCL,
	OOPS_BADARGSP,OOPS_BADARGCLM,OOPS_BADARGSPM,OOPS_BADCLASS,
	OOPS_BADSPEC,OOPS_NOISA,OOPS_RDEOF,OOPS_RDFAIL,OOPS_RDSYNERR,
	OOPS_RDREFERR,OOPS_RDWRONGCLASS,OOPS_RDUNKCLASS,OOPS_RDVERSERR,
	OOPS_RDBADTYP,OOPS_RDABSTCLASS;

#define	THIS	Object
#define	BASE
/* DEFINE_CLASS(); */
overload Object_reader;
static void Object_reader(istream&, Object&)
{
	setOOPSerror(OOPS_RDABSTCLASS,DEFAULT,"Object");
}
static void Object_reader(fileDescTy&,Object&)
{
	setOOPSerror(OOPS_RDABSTCLASS,DEFAULT,"Object");
}

const Class class_Object = Class( *(Class*)0, "Object", 1, "$Header: Object.c,v 1.3 88/02/04 12:59:41 keith Locked $", sizeof(Object), Object_reader, Object_reader, NULL, NULL);

extern Dictionary classDictionary;	// Dictionary of all classes 

static IdentDict* dependDict =0;	// object ID -> dependents list 
static IdentDict* deepCopyDict =0;	// object ID -> object copy dictionary for deepCopy 
static IdentDict* storeObjDict =0;	// object ID -> object number dictionary for storeOn 
static OrderedCltn* storeClassTable =0;	// table of classes stored
static OrderedCltn* readObjTable =0;	// table of objects for readFrom()
static OrderedCltn* readClassTable =0;	// table of classes for readFrom()
static char clsName[256];		// class name storage for readFrom()
	
bool Object::isKindOf(const Class& clid)
{
	const Class* cl = isA();
	do {
		if (cl == &clid) return YES;
		cl = cl->baseClass();
	} while (cl != (Class*) 0);
	return NO;
}

unsigned Object::size() { return 0; }		// # of objects in Array.container subclass 

unsigned Object::capacity() { return 0; }	// subClass.capacity 

const Class* Object::isA()
{
	setOOPSerror(OOPS_NOISA,FATAL);
	exit(1);
	return 0;
}

const Class* Object::species()	{ return isA(); }

bool Object::isEqual(const Object&) { derivedClassResponsibility("isEqual"); return NO; }

unsigned Object::hash() { derivedClassResponsibility("hash"); return 0; }

int Object::compare(const Object&) { derivedClassResponsibility("compare"); return 0; }

Object* Object::copy() { return deepCopy(); }

Object* Object::shallowCopy()
{
	register unsigned i = div_sizeof_ptr((isA()->size()+(sizeof(void*)-1)));
	register void** p = (void**)this;
	register void** q = new void*[i];
	register Object* o = (Object*)q;
	while (i--) *q++ = *p++;
	return o;
}

Object* Object::deepCopy()
{
	bool firstObject = NO;
	if (deepCopyDict == 0) {
		deepCopyDict = new IdentDict;
		deepCopyDict->add(*new Assoc(*nil,*nil));
		firstObject = YES;
	}
	Assoc* asc = (Assoc*)&deepCopyDict->assocAt(*this);
	if (asc == nil) {			// object has not been copied 
		Object* copy = Object::shallowCopy();	// make a shallow copy 
		deepCopyDict->add(*new Assoc(*this,*copy));	// add to dictionary 
		copy->deepenShallowCopy();	// convert shallow copy to deep copy 
		if (firstObject) {		// delete the deepCopy dictionary 
			DO(*deepCopyDict,Assoc*,asc) delete asc; OD
			delete deepCopyDict;
			deepCopyDict = 0;
		}
		return copy;
	}
	else return asc->value();	// object already copied, just return object reference 
}

void Object::deepenShallowCopy()	{ derivedClassResponsibility("deepenShallowCopy"); }

// error reporting 

void Object::derivedClassResponsibility(const char* fname)
{
	setOOPSerror(OOPS_DRVDCLASSRSP,DEFAULT,this,className(),fname);
}

void Object::destroyer() {}

void Object::shouldNotImplement(const char* fname)
{
	setOOPSerror(OOPS_ILLEGALMFCN,DEFAULT,this,className(),fname);
}

void invalidArgClass(const Object& ob, const Class& expect, const char* fname)
{
	setOOPSerror(OOPS_BADARGCL,DEFAULT,fname,expect.className(),fname,ob.className());
}

void invalidArgSpecies(const Object& ob, const Class& expect, const char* fname)
{
	setOOPSerror(OOPS_BADARGSP,DEFAULT,fname,expect.className(),fname,ob.species()->className());
}

void Object::invalidArgClass(const Object& ob, const Class& expect, const char* fname)
{
	setOOPSerror(OOPS_BADARGCLM,DEFAULT,className(),fname,expect.className(),className(),fname,ob.className());
}

void Object::invalidArgSpecies(const Object& ob, const Class& expect, const char* fname)
{
	setOOPSerror(OOPS_BADARGSPM,DEFAULT,className(),fname,expect.className(),className(),fname,ob.species()->className());
}

void invalidClass(const Object& ob, const Class& expect)
{
	setOOPSerror(OOPS_BADCLASS,DEFAULT,expect.className(),ob.className());
}

void invalidSpecies(const Object& ob, const Class& expect)
{
	setOOPSerror(OOPS_BADSPEC,DEFAULT,expect.className(),ob.species()->className());
}

// Object I/O

void Object::printOn(ostream&) { derivedClassResponsibility("printOn"); }

void Object::scanFrom(istream&) { derivedClassResponsibility("scanFrom"); }

void Object::storeOn(ostream& strm)
{
	bool firstObject = NO;
	if (storeObjDict == 0) {
		storeObjDict = new IdentDict(256);
		storeObjDict->add(*new AssocInt(*nil,0));
		storeClassTable = new OrderedCltn(classDictionary.size());
		firstObject = YES;
	}
	AssocInt* asc = (AssocInt*)&storeObjDict->assocAt(*this);
	if (asc == nil) {			// object has not been stored 
		storer(strm);			// call storer for this object 
		strm << "}\n";
		if (firstObject) {		// delete the storeOn dictionaries
			DO(*storeObjDict,AssocInt*,asc) delete asc; OD
			delete storeObjDict;  storeObjDict = 0;
			DO(*storeClassTable,Class*,cl) cl->classNumber(0); OD
			delete storeClassTable;  storeClassTable = 0;
		}
	}
	else strm << "@" << ((Integer*)asc->value())->value() << " ";	// object already stored, just output object reference 
}

void Object::storer(ostream& strm)	// store class Object 
{
	unsigned objectNum = storeObjDict->size();
	storeObjDict->add(*new AssocInt(*this,objectNum));
	if (isA()->classNumber() == 0) { // object of this class has not been previously stored 
		storeClassTable->add(*isA());
		isA()->classNumber(storeClassTable->size());
		strm << ":" << className() << "." << classVersion();
	}
	else				// object of this class already stored, just output class reference
		strm << "#" << isA()->classNumber();
	strm << "{";
}

Object* readFrom(istream& strm, const char* expectedClass, Object& where)
{
	bool firstObject = NO;			// first object flag 
	char delim;				// delimiter character 
	Class* readClass =0;			// class descriptor pointer
	Object* target = &where;			// target object address 
	
// read first character of next object on input stream 
	strm >> delim;
	if (strm.eof()) return nil;		// no objects remaining on input stream 
	checkRead(strm);
	
// allocate table if necessary 
	if (readObjTable == 0) {
		readObjTable = new OrderedCltn(1024);
		readObjTable->add(*nil);	// nil is @0 
		readClassTable = new OrderedCltn(classDictionary.size());
		readClassTable->add(*nil);
		firstObject = YES;
	}

// parse object reference, class reference, or class name & version
	switch (delim) {
		case '@': {			// read object reference 
			unsigned objectNum;	// object reference number 
			strm >> objectNum;
			checkRead(strm);
			if (target != 0) setOOPSerror(OOPS_RDREFERR,DEFAULT,(readObjTable->at(objectNum))->className(),objectNum);
#ifdef DEBUG_OBJIO
cerr << "readFrom: ref to object #" << objectNum
	<< ", class " << (readObjTable->at(objectNum))->className() << "\n";
#endif
			return readObjTable->at(objectNum);
		}
		case '#': {			// read class reference
			unsigned classNum;	// class reference number
			strm >> classNum >> delim;
			checkRead(strm);
			if (delim != '{') syntaxErr(strm,"{",delim);
			readClass = (Class*)readClassTable->at(classNum);
#ifdef DEBUG_OBJIO
cerr << "readFrom: ref to class #" << classNum
	<< ", class " << *readClass << "\n";
#endif
			break;
		}
		case ':': {			// read class name and class version number
			unsigned classVerNum;	// class version # 
			strm.get(clsName, sizeof clsName, '.');  strm >> delim;
			checkRead(strm);
			if (delim != '.') syntaxErr(strm,".",delim);
			strm >> classVerNum >> delim;
			checkRead(strm);
			if (delim != '{') syntaxErr(strm,"{",delim);
			if ((strlen(expectedClass) > 0 || target != 0) && (strcmp(expectedClass,clsName) != 0))
				setOOPSerror(OOPS_RDWRONGCLASS,DEFAULT,expectedClass,clsName);

// lookup class name in dictionary 
			LookupKey* asc = &classDictionary.assocAt(String(clsName));
			if (asc == nil)	setOOPSerror(OOPS_RDUNKCLASS,DEFAULT,clsName);
			readClass = (Class*)asc->value();

// validate class version number, add class to readClassTable
			if (readClass->classVersion() != classVerNum)
					setOOPSerror(OOPS_RDVERSERR,DEFAULT,clsName,readClass->classVersion(),classVerNum);
			readClassTable->add(*readClass);
#ifdef DEBUG_OBJIO
cerr << "readFrom: class " << *readClass << "\n";
#endif
			break;
		}
		default: syntaxErr(strm,":, # or @",delim);
	}
		
// allocate storage for object if needed, add pointer to readObjTable
	if (target == 0) target = (Object*)new char[readClass->size()];
	unsigned readObjTableSize = readObjTable->size();
	if (readObjTableSize == readObjTable->capacity()) readObjTable->reSize(readObjTableSize+1024);
	readObjTable->add(*target);

// call class object reader to read object 
#ifdef DEBUG_OBJIO
cerr << "readFrom: read object #" << readObjTable->size()-1 << "\n";
#endif
	readClass->reader(strm,*target);
	strm >> delim;
	checkRead(strm);
	if (delim != '}') syntaxErr(strm,"}",delim);
	if (firstObject) {
		delete readObjTable;  readObjTable = 0;
		delete readClassTable;  readClassTable = 0;
	}
	return target;
}

// binary object I/O

void Object::storeOn(fileDescTy& fd)
{
	bool firstObject = NO;
	if (storeObjDict == 0) {
		storeObjDict = new IdentDict(256);
		storeObjDict->add(*new AssocInt(*nil,0));
		storeClassTable = new OrderedCltn(classDictionary.size());
		firstObject = YES;
	};
	AssocInt* asc = (AssocInt*)&storeObjDict->assocAt(*this);
	if (asc == nil) {		// object has not been stored 
		storer(fd);		// call storer for this object 
		if (firstObject) { 	// delete the storeOn dictionary 
			DO(*storeObjDict,AssocInt*,asc) delete asc; OD
			delete storeObjDict;  storeObjDict = 0;
			DO(*storeClassTable,Class*,cl) cl->classNumber(0); OD
			delete storeClassTable;  storeClassTable = 0;
		};
	}
	else {
       		char recordType = storeOnObjectRef;	// object already stored, just output object reference 
		storeBin(fd,(char*)&recordType,1);
		storeBin(fd,((Integer*)asc->value())->value());
	};
}

void Object::storer(fileDescTy& fd)	// store Object on file descriptor
{
	unsigned objectNum = storeObjDict->size();
	unsigned short classNum;
	storeObjDict->add(*new AssocInt(*this,objectNum));
	if (isA()->classNumber() == 0) { // object of this class has not been previously stored 
		storeClassTable->add(*isA());
		isA()->classNumber(storeClassTable->size());
		char recordType = storeOnClass;
		storeBin(fd,(char*)&recordType,1);
		unsigned char classNameLen = strlen(className());
		storeBin(fd,(char)classNameLen);
		storeBin(fd,className(),classNameLen);
		storeBin(fd,classVersion());
	}
	else {				// object of this class already stored, just output class reference
		char recordType = storeOnClassRef;
		storeBin(fd,(char*)&recordType,1);
		classNum = isA()->classNumber();
		storeBin(fd,classNum);
	}
}

Object* readFrom(fileDescTy& fd,const char* expectedClass,Object& where)
{
	bool firstObject = NO;			// first object flag 
	char recordType =-1;			// object I/O record type
	Class* readClass =0;			// class descriptor pointer
	Object* target = &where;			// target object address 

// read record type byte from file descriptor
	readBin(fd,&recordType,1);
	
// allocate tables if necessary 
	if (readObjTable == 0) {
		readObjTable = new OrderedCltn(1024);
		readObjTable->add(*nil);	// nil is @0 
		readClassTable = new OrderedCltn(classDictionary.size());
		readClassTable->add(*nil);
		firstObject = YES;
	}

// parse object reference, class reference, or class name & version
	switch (recordType) {
		case storeOnObjectRef: {	// read object reference
			unsigned objectNum;	// object reference number 
			readBin(fd,objectNum);
			if (target != 0) setOOPSerror(OOPS_RDREFERR,DEFAULT,
				(readObjTable->at(objectNum))->className(),objectNum);
#ifdef DEBUG_OBJIO
cerr << "readFrom: ref to object #" << objectNum
	<< ", class " << (readObjTable->at(objectNum))->className() << "\n";
#endif
			return readObjTable->at(objectNum);
		}
		case storeOnClassRef: {		// read class reference
			unsigned short classNum;  // class reference number
			readBin(fd,classNum);
			readClass = (Class*)readClassTable->at(classNum);
#ifdef DEBUG_OBJIO
cerr << "readFrom: ref to class #" << classNum
	<< ", class " << *readClass << "\n";
#endif
			break;
		}
		case storeOnClass: {		// read class name and class version number
			unsigned classVerNum;	// class version # 
			unsigned char namelen = 0;
			readBin(fd,namelen);
			readBin(fd,clsName,namelen);
			clsName[namelen] = '\0';
			readBin(fd,classVerNum);
			if ((strlen(expectedClass) > 0 || target != 0) && (strcmp(expectedClass,clsName) != 0))
				setOOPSerror(OOPS_RDWRONGCLASS,DEFAULT,expectedClass,clsName);
				
// lookup class name in dictionary 
			LookupKey* asc = &classDictionary.assocAt(String(clsName));
			if (asc == nil)	setOOPSerror(OOPS_RDUNKCLASS,DEFAULT,clsName);
			readClass = (Class*)asc->value();

// validate class version number, add class to readClassTable
			if (readClass->classVersion() != classVerNum)
				setOOPSerror(OOPS_RDVERSERR,DEFAULT,clsName,readClass->classVersion(),classVerNum);
			readClassTable->add(*readClass);
#ifdef DEBUG_OBJIO
cerr << "readFrom: class " << *readClass << "\n";
#endif
			break;
		}
		default: setOOPSerror(OOPS_RDBADTYP,DEFAULT,recordType,fd);
	}

// allocate storage for object if needed, add pointer to readObjTable
	if (target == 0) target = (Object*)new char[readClass->size()];
	unsigned readObjTableSize = readObjTable->size();
	if (readObjTableSize == readObjTable->capacity()) readObjTable->reSize(readObjTableSize+1024);
	readObjTable->add(*target);

// call class object reader to read object 
#ifdef DEBUG_OBJIO
cerr << "readFrom: read object #" << readObjTable->size()-1 << "\n";
#endif
        readClass->reader(fd,*target);
	if (firstObject) {
		delete readObjTable;  readObjTable = 0;
		delete readClassTable;  readClassTable = 0;
	}
	return target;
}

// Object Dependence Relationships 

Object* Object::addDependent(const Object& ob)
{
	if (dependDict == 0) dependDict = new IdentDict;
	if (!(dependDict->includesKey(*this)))
		dependDict->add(*new Assoc(*this,*new OrderedCltn));
	((OrderedCltn*)dependDict->atKey(*this))->add(ob);
	return &ob;
}

Object* Object::removeDependent(const Object& ob)
{
	if (dependDict == 0) return nil;
	OrderedCltn* depList = (OrderedCltn*)dependDict->atKey(*this);
	if (depList == nil) return nil;
	Object* dependent = depList->removeId(ob);
	if (depList->size() == 0) release();
	return dependent;
}
	
OrderedCltn& Object::dependents()
{
	if (dependDict == 0) return *new OrderedCltn(1);
	Assoc* asc = (Assoc*)&dependDict->assocAt(*this);
	if (asc == nil) return *new OrderedCltn(1);
	return *new OrderedCltn(*(OrderedCltn*)asc->value());
}
	
void Object::release()
{
	if (dependDict != 0 && dependDict->includesKey(*this)) {
		Assoc* asc = (Assoc*)&dependDict->removeKey(*this);
		delete (OrderedCltn*)asc->value();
		delete asc;
		if (dependDict->size() == 0) {
			delete dependDict;
			dependDict = 0;
		}
	}
}

void Object::changed(const Object& param)
{
	OrderedCltn* depList = &dependents();
	DO(*depList,Object*,depob) depob->update(*this,param); OD
	delete depList;
}

void Object::changed()	{ changed(*nil); }

void Object::update(const Object& /*dependent*/, const Object& /*param*/)
{
	derivedClassResponsibility("update");
}
