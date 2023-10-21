/* Test class ArrayOb

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
	
$Log:	array.c,v $
 * Revision 1.1  88/01/17  22:24:20  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: array.c,v 1.1 88/01/17 22:24:20 keith Exp $";

#include "Point.h"
#include "Rectangle.h"
#include "ArrayOb.h"
//#include "FloatVec.h"

main()
{
	cout << "\nTest class ArrayOb\n";
	Point A(1,2);
	Point B(3,4); 
	Rectangle a(0,0,10,10);
	Rectangle b(1,2,1,2);
	ArrayOb v(10);
	ArrayOb t(2),u(2);
	v[0] = &a;
	v[1] = &A;
	v[2] = new Point(0,1);
	cout << v << "\n";
	t[0] = &a; t[1] = &A;
	u = t;
	cout << "t = " << t << "\n";
	cout << "t == u: " << (t==u) << "\n";
	cout << "t isEqual u: " << t.isEqual(u) << "\n";
	cout << "t isEqual v: " << t.isEqual(v) << "\n";
/*
	FloatVec x(10);
	for (register i=0; i<10; i++) x[i] = i%3;
	x.sort();
	cout << "x = \n" << x << "\n";
*/
}
