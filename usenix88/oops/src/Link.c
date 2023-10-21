/* Link.c -- implementation of singly-linked list element

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
	October, 1985

Function:
	
Link is an abstract class that is used to construct LinkedLists.  It
contains a pointer to the next Link in the list, or nil if this is the
last Link.

$Log:	Link.c,v $
 * Revision 1.2  88/01/16  23:39:26  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Link.h"

#define	THIS	Link
#define	BASE	Object
DEFINE_CLASS(Link,Object,1,"$Header: Link.c,v 1.2 88/01/16 23:39:26 keith Exp $",NULL,NULL);

extern const int OOPS_DELLNK;

Link::~Link()
{
	if (next != (Link*)nil) setOOPSerror(OOPS_DELLNK,DEFAULT,className(),next,this);
}

void Link::deepenShallowCopy()
{
	next = (Link*)nil;
}

Object* Link::shallowCopy()	{ shouldNotImplement("shallowCopy"); return 0; }
	
void Link::storer(ostream& strm)
/*
Object I/O for class Link does not store or read the "next" pointer --
this is handled by class LinkedList.
*/
{
	BASE::storer(strm);
}

void Link::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
}
