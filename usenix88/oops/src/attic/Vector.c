/* Vector.c  -- implementation of class Vector

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: {decvax!}seismo!elsie!cecil!keith
	May, 1986

Function:
	
Modification History:

*/

#include "Vector.h"

#define	THIS	Vector
#define	BASE	Object
DEFINE_CLASS(Vector,Object,1,NULL,NULL);

extern const int OOPS_RDABSTCLASS;

static void Vector_reader(istream& /*strm*/, Object& /*where*/)
{
	setOOPSerror(OOPS_RDABSTCLASS,DEFAULT,"Vector");
}

void Vector::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << n;
}
