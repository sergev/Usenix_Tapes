#ifndef VECTORH
#define VECTORH

/* Vector.h -- declarations for generic vector objects

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: {decvax!}seismo!elsie!cecil!keith
	November, 1985


Function:

Defines the macros Vectordeclare(classname,type) and
Vectorimplement(classname,type) which expand into the declarations and
functions for vectorss of the specified fundamental type: char, int,
short, long, unsigned, float, and double.  See IntVec.h and IntVec.c as
an example.

Modification History:
	
*/

#include "Object.h"

const int VECTOR_DEFAULT_CAPACITY = 10;

#ifndef VECTOR_FRIENDS
#define VECTOR_FRIENDS
#endif

#define	VECTOR_CLASS(classname)	class_\
classname

#define Vectordeclare(classname,type)					\
extern Class VECTOR_CLASS(classname);					\
									\
class classname: public Object {					\
	type* v;							\
	int sz;								\
	void AllocSizeErr();						\
	void IndexRangeErr();						\
	VECTOR_FRIENDS							\
public:									\
	classname(int size =VECTOR_DEFAULT_CAPACITY);			\
	classname(const classname&);					\
	~classname()		{ delete v; }				\
	classname(istream&,classname&);					\
	type& elem(int i)	{ return v[i]; }			\
	bool operator!=(const classname& a)	{ return !(*this==a); }	\
	void operator=(const classname&);				\
	bool operator==(const classname&);				\
	type& operator[](int i)						\
	  { if ((unsigned)i >= sz) IndexRangeErr();			\
	  return v[i]; }						\
	virtual type& at(int i);					\
	virtual int capacity();						\
	virtual void deepenShallowCopy();				\
	virtual int hash();						\
	virtual const Class* isA();					\
	virtual bool isEqual(const Object&);				\
	virtual void printOn(ostream& strm);				\
	virtual void reSize(int);					\
	virtual obid shallowCopy();					\
	virtual int size();						\
	virtual void sort();						\
	virtual const Class* species();					\
	virtual void storer(ostream&);					\
};

#endif
