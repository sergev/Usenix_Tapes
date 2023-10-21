/* IntVect.c -- Data type-specific functions for class IntVec

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

Function:
	
Data type -specific functions for class IntVec.

Modification History:

$Log:	IntVect.c,v $
 * Revision 1.1  88/01/17  09:47:17  keith
 * Initial revision
 * 
	
*/

#include "IntVec.h"
#include "oopsconfig.h"
#include <libc.h>

#define	THIS	IntVec
#define	BASE	Vector

static int typeCmp(int* a, int* b)
{
	return *a-*b;
}

void THIS::sort()
{
	qsort(v,n,sizeof(THIS),typeCmp);
}

unsigned THIS::hash()
{
	register unsigned h = n;
	register unsigned i = n;
	register int* vv = v;
	while (i--) h ^= (unsigned)*vv++;
	return h;
}

void THIS::printOn(ostream& strm)
{
	for (register int i=0; i<n; i++) {
		if (i>0 && (i%8 == 0)) strm << "\n\t";
		strm << dec(v[i],10);
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
	v = new int[n];
	for (register int i =0; i<n; i++) strm >> v[i];
}

void THIS::storer(ostream& strm)
{
	BASE::storer(strm);
	for (register int i=0; i<n; i++) strm << v[i] << " ";
}
