/* Test class Set

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
	
$Log:	sets.c,v $
 * Revision 1.1  88/01/17  22:25:05  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: sets.c,v 1.1 88/01/17 22:25:05 keith Exp $";

#include "Point.h"
#include "Set.h"
#include "OrderedCltn.h"

main()
{
	cout << "\nTest class Set\n";
	Point A(1,1);
	Point B(1,2);
	Point C(1,3);
	Point D(1,3);
	Point E(1,4);
	Set s(16);
	Set t;
	s.add(A);
	s.add(B);
	s.add(C);
	s.add(D);
	cout << "s = " << s << "\n";
	t = s;
	s.reSize(30);
	cout << "s.includes(C): " << s.includes(C) << "\n";
	cout << "t == s: " << (t==s) << "\n";
	s.remove(C);
	cout << "s.includes(C): " << s.includes(C) << "\n";
	cout << "t == s: " << (t==s) << "\n";
	s.add(E);
	cout << "s = " << s << "\n";
	cout << "t = " << t << "\n";
	cout << "s&t = " << (s&t) << "\n";
	cout << "s|t = " << (s|t) << "\n";
	cout << "s-t = " << (s-t) << "\n";
	cout << "s.asOrderedCltn: " << (s.asOrderedCltn()) << "\n";
}
