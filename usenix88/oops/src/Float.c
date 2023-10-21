/* Float.c -- implementation of Float object

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
	December, 1985

Function:
	
Provides an object that contains a double.

$Log:	Float.c,v $
 * Revision 1.2  88/01/16  23:38:58  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Float.h"
#include "oopsIO.h"

#define	THIS	Float
#define	BASE	Object
DEFINE_CLASS(Float,Object,1,"$Header: Float.c,v 1.2 88/01/16 23:38:58 keith Exp $",NULL,NULL);

Float::Float(istream& strm)	{ parseFloat(strm); }

int Float::compare(const Object& ob)
{
	assertArgSpecies(ob,class_Float,"compare");
	register double t = val-((Float*)&ob)->val;
	if (t < 0) return -1;
	if (t > 0) return 1;
	return 0;
}

Object* Float::copy()		{ return shallowCopy(); }

void Float::deepenShallowCopy()	{}

unsigned Float::hash()
{
	static union {
		unsigned asint[2];
		double asdouble;
#ifdef bug
	};
	asdouble = val;
	return asint[0] ^ asint[1];
#else
	} hun;
	hun.asdouble = val;
	return hun.asint[0] ^ hun.asint[1];
#endif
}

bool Float::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Float) && val==((Float*)&ob)->val;
}

const Class* Float::species()	{ return &class_Float; }

void Float::printOn(ostream& strm)
{
	strm << val;
}

void Float::scanFrom(istream& strm)	{ parseFloat(strm); }

Float::Float(istream& strm, Float& where)
{
	this = &where;
	strm >> val;
}

void Float::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << val << " ";
}

Float::Float(fileDescTy& fd, Float& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Float::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}
