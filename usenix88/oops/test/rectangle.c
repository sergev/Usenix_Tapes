/* Test class Rectangle

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
	
$Log:	rectangle.c,v $
 * Revision 1.1  88/01/17  22:25:02  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: rectangle.c,v 1.1 88/01/17 22:25:02 keith Exp $";

#include "Point.h"
#include "Rectangle.h"
#include "OrderedCltn.h"

main()
{
	cout << "\nTest class Rectangle\n";
	Rectangle a(0,0,10,10);
	Rectangle b(Point(1,2),Point(11,3));
	Rectangle c;
	cout << "a = " << a << "\n";
	cout << "b = " << b << "\n";
	cout << "a == b: " << (a==b) << "\n";
	c = a;
	cout << "a isEqual c: " << a.isEqual(c) << "\n";
	cout << "a contains b: " << a.contains(b) << "\n";
	cout << "a intersects b: " << a.intersects(b) << "\n";
	cout << "a||b = " << (a||b) << "\n";
	cout << "a&&b = " << (a&&b) << "\n";
	cout << a.areasOutside(b);
}
