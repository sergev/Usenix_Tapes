/* Stackobid.c -- member functions of class stackobid

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
	
Member function definitions for class Stackobid (Stack of obid, or
Object*).

Modification History:
	
*/

#include "Stackobid.h"
DEFINE_CLASS(Stackobid,Arrayobid,1);

bool Stackobid::operator==(Stackobid& a)
{
	if (t != a.t) return NO;
	else {
		register int i = t;
		register obid* vv = &this->elem(0);
		register obid* av = &a.elem(0);
		while (i--) if (!((*vv++)->isEqual(**av++))) return NO;
	}
	return YES;
}

bool Stackobid::isEqual(Object& a)
{
	return a.isMemberOf(class_Stackobid) && (*this==*(Stackobid*)&a);
}

void Stackobid::printOn(ostream& strm)
{
	strm << className() << "[";
	for (register int i=t; i>0; i--) {
		if(i<t) strm << "\n";
		elem(i-1)->printOn(strm);
	}
	strm << "]\n";
}

int Stackobid::hash()
{
	register int h = t;
	register int i = t;
	register obid* vv = &elem(0);
	while (i--) h^=(*vv++)->hash();
	return h;
}

void Stackobid::OverflowErr()
{
	seterror(OOPS_OVERFLOW,DEFAULT,this,className());
}
void Stackobid::UnderflowErr()
{
	seterror(OOPS_UNDERFLOW,DEFAULT,this,className());
}
void Stackobid::EmptyErr()
{
	seterror(OOPS_STACKEMPTY,DEFAULT,this,className());
}
