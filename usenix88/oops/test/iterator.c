/* Test class Iterator

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

Function:
	
Modification History:
	
$Log:	iterator.c,v $
 * Revision 1.1  88/01/17  22:24:48  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: iterator.c,v 1.1 88/01/17 22:24:48 keith Exp $";

#include "Iterator.h"
#include "LinkedList.h"
#include "OrderedCltn.h"
#include "Set.h"
#include "Point.h"
#include "LinkOb.h"
#include <fcntl.h>
#include <osfcn.h>

const char* fileName = "iterator.obj";

main()
{
	cout << "\nTest class Iterator\n";
	OrderedCltn c,x;
	{ for (int j=0; j<6; j++) c.add(*new Point(j,j)); }
	cout << c;
	Set s;
	s = c.asSet();
	cout << s;
	LinkedList l;
	Iterator i(s);
	Object* p;
	while (p = i++) l.add(*new LinkOb(*p));
	cout << l;
	x.add(c); x.add(l);

    DO(x,Collection*,cp) {
	cout << "\nTest Iterator(" << cp->className() << ").storeOn()\n";
	ostream* out = new ostream(creat(fileName,0664));
	Iterator ii(*cp);
	cout << ii << ": ";
	{ for (int j=0; j<3; j++) cout << *ii++; }
	cout << " " << ii;
	ii.storeOn(*out);
	delete out;

	cout << "\nTest Iterator(" << cp->className() << ") readFrom()\n";
	istream* in = new istream(open(fileName,O_RDONLY));
	Iterator* ip = (Iterator*)readFrom(*in,"Iterator");
	cout << *ip << ": ";
	while (p = (*ip)++) cout << *p;
	delete in;
	delete ip->collection();
	delete ip;

	cout << "\nTest Iterator(" << cp->className() << ").deepCopy()\n";
	ip = (Iterator*)ii.deepCopy();
	cout << *ip << ": ";
	while (p = (*ip)++) cout << *p;
	cout << "\nTest Iterator(" << cp->className() << ").reset()\n";
	ip->reset();
	cout << *ip << ": ";
	while (p = (*ip)++) cout << *p;
	cout << "\n";
	delete ip->collection();
	delete ip;
    } OD;
}
