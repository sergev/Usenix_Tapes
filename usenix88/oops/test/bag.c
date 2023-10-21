/* Test class Bag

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
	
$Log:	bag.c,v $
 * Revision 1.1  88/01/17  22:24:26  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: bag.c,v 1.1 88/01/17 22:24:26 keith Exp $";

#include "Point.h"
#include "Bag.h"
#include "ArrayOb.h"

main()
{
	cout << "\nTest class Bag\n";
	Point A(1,1);
	Point B(1,2);
	Point C(1,3);
	Point D(1,3);
	Bag b(16);
	Bag c;
	b.add(A);
	b.add(B);
	b.add(C);
	b.add(D);
	cout << "b = " << b << "\n";
	cout << "b.asArrayOb: " << b.asArrayOb() << "\n";
	c = b;
	b.reSize(30);
	cout << "b.includes(C): " << b.includes(C) << "\n";
	cout << "c == b: " << (c==b) << "\n";
	b.remove(C);
	cout << "b.includes(C): " << b.includes(C) << "\n";
	cout << "b = " << b << "\n";
	b.remove(C);
	cout << "b.includes(C): " << b.includes(C) << "\n";
	cout << "b = " << b << "\n";
	cout << "c == b: " << (c==b) << "\n";
}
