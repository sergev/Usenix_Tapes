/* Random.c -- implementation of pseudo-random number generator

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
	
A psuedo-random number generator.  The function next() returns random
numbers uniformly distributed over the interval [0.0,1.0].

$Log:	Random.c,v $
 * Revision 1.2  88/01/16  23:40:37  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Random.h"
#include "oopsconfig.h"
#include <time.h>
#include "oopsIO.h"

#define	THIS	Random
#define	BASE	Object
DEFINE_CLASS(Random,Object,1,"$Header: Random.c,v 1.2 88/01/16 23:40:37 keith Exp $",NULL,NULL);

Random::Random()	{ time(&randx); randx ^= (long)this; }

float Random::next()
{
	register float temp;
	do temp = DRAW(randx)/(float)MAX_INT; while (temp==0);
	return temp;
}

Object* Random::copy()		{ return shallowCopy(); }

void Random::deepenShallowCopy()	{}

void Random::printOn(ostream& strm)
{
	strm << randx;
}

Random::Random(istream& strm, Random& where)
{
	this = &where;
	strm >> randx;
}

void Random::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << randx << " ";
}

Random::Random(fileDescTy& fd, Random& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Random::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}
