/* Test class OrderedCltn

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
	
$Log:	orderedcltn.c,v $
 * Revision 1.1  88/01/17  22:24:54  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: orderedcltn.c,v 1.1 88/01/17 22:24:54 keith Exp $";

#include "Point.h"
#include "OrderedCltn.h"
#include "Set.h"

main()
{
	cout << "\nTest class OrderedCltn\n";
	Point A(1,1);
	Point B(1,2);
	Point C(1,3);
	Point D(1,3);
	OrderedCltn b(16);
	OrderedCltn c;
	b.add(A);
	b.add(B);
	b.add(C);
	b.add(D);
	cout << "b = " << b << "\n";
	cout << "b.first(): " << *(b.first()) << "\n";
	cout << "b.last(): " << *(b.last()) << "\n";
	b.addAfter(Point(1,2),Point(1,21));
	b.addBefore(Point(1,2),Point(1,19));
	cout << "b = " << b << "\n";
	cout << "b.after(Point(1,2)): " << *(b.after(Point(1,2))) << "\n";
	cout << "b.before(Point(1,2)): " << *(b.before(Point(1,2))) << "\n";
	c = b;
	b.reSize(30);
	cout << "b.includes(C): " << b.includes(C) << "\n";
	cout << "c == b: " << (c==b) << "\n";
	b.addAllLast(c);
	cout << "b = " << b << "\n";
	b.remove(C);
	cout << "b.includes(C): " << b.includes(C) << "\n";
	cout << "b = " << b << "\n";
	cout << "c == b: " << (c==b) << "\n";
	cout << "c&b:" << (c&b) << "\n";
	cout << "b.indexOfSubCollection(c): " << b.indexOfSubCollection(c,1) << "\n";
	cout << "c = " << c << "\n";
	b.replaceFrom(1,3,c);
	cout << "b.replaceFrom(1,3,c): " << b << "\n";
	b.sort();
	cout << "b.sort(): " << b << "\n";
	cout << "b.asSet(): " << (b.asSet()) << "\n";
}
