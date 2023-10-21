/* Test class LinkedList

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
	
$Log:	linkedlist.c,v $
 * Revision 1.1  88/01/17  22:24:50  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: linkedlist.c,v 1.1 88/01/17 22:24:50 keith Exp $";

#include "Point.h"
#include "LinkedList.h"
#include "LinkOb.h"
#include "Set.h"

main()
{
	cout << "\nTest class LinkedList\n";
	Point A(1,1);
	Point B(1,2);
	Point C(1,3);
	Point D(1,3);
	LinkOb bA(A);
	LinkOb bB(B);
	LinkOb bC(C);
	LinkOb bD(D);
	LinkOb cA(A);
	LinkOb cB(B);
	LinkOb cC(C);
	LinkOb cD(D);
	LinkedList b;
	LinkedList c;
	b.add(bA);
	b.add(bB);
	b.add(bC);
	b.add(bD);
	c.add(cA);
	c.add(cB);
	c.add(cC);
	c.add(cD);
	cout << "b = " << b << "\n";
	cout << "b.first(): " << *(b.first()) << "\n";
	cout << "b.last(): " << *(b.last()) << "\n";
	cout << "b.at(3): " << *(b.at(3)) << "\n";
	b.reSize(30);
	cout << "b.includes(C): " << b.includes(C) << "\n";
	cout << "c == b: " << (c==b) << "\n";
	b.addFirst(*new LinkOb(*new Point(1,0)));
	b.addLast(*new LinkOb(*new Point(1,19)));
	cout << "b = " << b << "\n";
	b.addAll(c);
	cout << "b = " << b << "\n";
	b.remove(*(b.at(1)));
	cout << "b.includes(A): " << b.includes(A) << "\n";
	cout << "b = " << b << "\n";
	cout << "c == b: " << (c==b) << "\n";
	cout << "b.indexOf(D): " << b.indexOf(D) << "\n";
	cout << "c = " << c << "\n";
	DO(c,Object*,ob) cout << *ob; OD
	cout << "\n";
	cout << "c.size() = " << c.size() << "\n";
	while (c.size() != 0) {
		c.remove(*c.first());
		cout << "c = " << c << "\n";
	}
	cout << "b.asSet(): " << (b.asSet()) << "\n";
	while (b.size() != 0) { b.remove(*b.first()); }
}
