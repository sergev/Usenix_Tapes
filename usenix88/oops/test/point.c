/* Test class Point

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
	
$Log:	point.c,v $
 * Revision 1.1  88/01/17  22:24:56  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: point.c,v 1.1 88/01/17 22:24:56 keith Exp $";

#include "Point.h"

main()
{
	Point A(1,2);
	Point B(3,4); 
	Point C = A;
	cout << "\nTest class Point\n";
	cout << "A = " << A << "\n";
	cout << "B = " << B << "\n";
	cout << "A+B = " << (A+B) << "\n";
	cout << "A-B = " << (A-B) << "\n";
	cout << "A isEqual C: " << A.isEqual(C) << "\n";
	cout << "10*A = " << (10*A) << "\n";
	cout << "A max B = " << (A.max(B)) << "\n";
	cout << "A min B = " << (A.min(B)) << "\n";
	cout << "A transpose = " << A.transpose() << "\n";
	cout << "A compare B = " << A.compare(B) << "\n";
	A += B;
	cout << "A+=B = " << A << "\n";
}
