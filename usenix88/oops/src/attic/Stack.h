#ifndef STACKH
#define STACKH

/* stack.h -- declarations for generic stack objects

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: {decvax!}seismo!elsie!cecil!keith
	September, 1985

Function:
	
Defines the macros Stackdeclare(type) and Stackimplement(type) which
expand into the declarations and functions for stacks of the specified
fundamental type: char, int, short, long, unsigned, float, and double.
See stackchar.h and stackchar.c as an example.  Note the use of the
ARRAY_SEP_PRINT and ARRAY_ELEM_HASH macros to customize the element
separator for the printOn() function and the calculation of the hash()
function.

Modification History:
	
*/

#include "Array.h"

#ifndef GENERICH
#include <generic.h>
#endif

#define Stack(type)		name2(Stack,type)
#define STACKCLASS(type)	name3(class_,Stack,type)

#define Stackdeclare(type)						\
extern Class STACKCLASS(type);						\
									\
class Stack(type) : Array(type) {					\
	int t;								\
	void OverflowErr();						\
	void UnderflowErr();						\
	void EmptyErr();						\
public:									\
	Stack(type)(int size=CLTN_DEFAULT_CAPACITY) : (size)		\
		{ isA(STACKCLASS(type)); t = 0; }			\
	Stack(type)(Stack(type)& a) : ((Array(type)&)a)			\
		{ isA(STACKCLASS(type)); t = a.t; }			\
	bool operator==(Stack(type)&);					\
	bool operator!=(Stack(type)& a)	{ return !(*this==a); }		\
	void push(type& a)						\
		{ if (t==capacity()) OverflowErr(); elem(t++) = a; }	\
	type pop()							\
		{ if (t==0) UnderflowErr(); return elem(--t); } 	\
	type& top()							\
		{ if (t==0) EmptyErr(); return elem(t-1); }		\
	virtual type& at(int i)	{ return this->operator[](t-i-1); }	\
	virtual int hash();						\
	virtual bool isEqual(Object&);					\
	virtual void printOn(ostream& strm);				\
	virtual int size()	{ return t; }				\
};

#define Stackimplement(type)						\
bool Stack(type)::operator==(Stack(type)& a)				\
{									\
	if (t != a.t) return NO;					\
	else {								\
		register int i = t;					\
		register type* vv = &this->elem(0);			\
		register type* av = &a.elem(0);				\
		while (i--) if (*vv++ != *av++) return NO;		\
	}								\
	return YES;							\
}									\
									\
bool Stack(type)::isEqual(Object& a)					\
{									\
	return a.isMemberOf(STACKCLASS(type)) && (*this==*(Stack(type)*)&a);\
}									\
									\
void Stack(type)::printOn(ostream& strm)				\
{									\
	strm << className() << "[";					\
	for (register int i=t; i>0; i--) {				\
		if (i<t) ARRAY_SEP_PRINT(strm);				\
		strm << elem(i-1);					\
	}								\
	strm << "]\n";							\
}									\
									\
int Stack(type)::hash()							\
{									\
	register int i = t;						\
	register type* vv = &elem(0);					\
	register int h = t;						\
	while (i--) ARRAY_ELEM_HASH(h,*vv++);				\
	return h;							\
}									\
									\
void Stack(type)::OverflowErr()						\
{									\
	seterror(OOPS_OVERFLOW,DEFAULT,this,className());		\
}									\
void Stack(type)::UnderflowErr()					\
{									\
	seterror(OOPS_UNDERFLOW,DEFAULT,this,className());		\
}									\
void Stack(type)::EmptyErr()						\
{									\
	seterror(OOPS_STACKEMPTY,DEFAULT,this,className());		\
}

#endif
