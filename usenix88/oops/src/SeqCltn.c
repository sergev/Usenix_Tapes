/* SeqCltn.c -- implementation of abstract sequential collections

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
	
SeqCltn is an abstract class representing collections whose elements are
ordered and are externally named by integer indices.

$Log:	SeqCltn.c,v $
 * Revision 1.2  88/01/16  23:41:08  keith
 * Remove pre-RCS modification history
 * 
*/

#include "SeqCltn.h"

#define	THIS	SeqCltn
#define	BASE	Collection
DEFINE_CLASS(SeqCltn,Collection,1,"$Header: SeqCltn.c,v 1.2 88/01/16 23:41:08 keith Exp $",NULL,NULL);

extern const int OOPS_RDABSTCLASS,OOPS_INDEXRANGE;

void SeqCltn::atAllPut(const Object&)
	{ derivedClassResponsibility("atAllPut"); }

void SeqCltn::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
}

Object* SeqCltn::first()
	{ derivedClassResponsibility("first"); return 0; }
	
int SeqCltn::indexOf(const Object&)
	{ derivedClassResponsibility("indexOf"); return 0; }
	
int SeqCltn::indexOfSubCollection(const SeqCltn& /*cltn*/, int /*start*/)
	{ derivedClassResponsibility("indexOfSubCollection"); return 0; }
	
Object* SeqCltn::last()
	{ derivedClassResponsibility("last"); return 0; }

Object* SeqCltn::doNext(Iterator& pos)
{
	if (pos.index < size()) return at(pos.index++);
	return 0;
}

void SeqCltn::replaceFrom(int /*start*/, int /*stop*/, const SeqCltn& /*replacement*/, int /*startAt*/)
	{ derivedClassResponsibility("replaceFrom"); }

void SeqCltn::indexRangeErr()
{
	setOOPSerror(OOPS_INDEXRANGE,DEFAULT,this,className());
}
