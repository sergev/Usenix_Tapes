/* Test class SortedCltn

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
	
$Log:	sortedcltn.c,v $
 * Revision 1.1  88/01/17  22:25:07  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: sortedcltn.c,v 1.1 88/01/17 22:25:07 keith Exp $";

#include "Point.h"
#include "SortedCltn.h"
#include "Set.h"

main()
{
	cout << "\nTest class SortedCltn\n";
	Point A(1,1);
	Point B(1,2);
	Point C(1,3);
	Point D(1,3);
	Point E(1,4);
	SortedCltn s(8);
	SortedCltn t(8);
	s.add(A);
	s.add(B);
	s.add(C);
	s.add(D);
	s.add(E);
	cout << "s = " << s << "\n";
	t.add(E);
	t.add(D);
	t.add(C);
	t.add(B);
	t.add(A);
	cout << "s==t: " << (s==t) << "\n";
	Set st = s.asSet();
	cout << "s.asSet(): " << st << "\n";
	cout << "st.asSortedCltn(): " << st.asSortedCltn() << "\n";
}
