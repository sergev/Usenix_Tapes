/* DoubleVect.c -- Data type-specific functions for class DoubleVec

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
	uucp: {decvax!}seismo!elsie!cecil!keith
	June, 1986

Function:
	
Data type -specific functions for class DoubleVec.

Modification History:

$Log:	DoubleVect.c,v $
 * Revision 1.1  88/01/17  09:47:14  keith
 * Initial revision
 * 
	
*/

#include "DoubleVec.h"
#include "oopsconfig.h"
#include <libc.h>

#define	THIS	DoubleVec
#define	BASE	Vector

static int typeCmp(double* a, double* b)
{
	register double t = *a-*b;
	if (t < 0) return -1;
	if (t > 0) return 1;
	return 0;
}

void THIS::sort()
{
	qsort(v,n,sizeof(THIS),typeCmp);
}

unsigned THIS::hash()
{
	register unsigned h = n;
#ifdef LOG2_INT
	register unsigned i = n*sizeof(double) >> LOG2_INT;
#else
	register unsigned i = n*sizeof(double)/sizeof(int);
#endif
	register unsigned* vv = (unsigned*)v;
	while (i--) h ^= *vv++;
	return h;
}

void THIS::printOn(ostream& strm)
{
	for (register unsigned i=0; i<n; i++) {
		if (i>0 && (i%6 == 0)) strm << "\n\t";
		strm << form("%13.4g",v[i]);
	}
}

void THIS::scanFrom(istream& strm)
{
	extern const int OOPS_NYET;
	setOOPSerror(OOPS_NYET,DEFAULT,className(),"scanFrom");
}

THIS::THIS(istream& strm, THIS& where) : (strm,where)
{
	this = &where;
	v = new double[n];
	for (register int i =0; i<n; i++) strm >> v[i];
}

void THIS::storer(ostream& strm)
{
	BASE::storer(strm);
	for (register int i=0; i<n; i++) strm << form("%.14g",v[i]) << " ";
}
