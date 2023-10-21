/* Test class Fraction

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
	
$Log:	fraction.c,v $
 * Revision 1.1  88/01/17  22:24:42  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: fraction.c,v 1.1 88/01/17 22:24:42 keith Exp $";

#include "Fraction.h"

main()
{
	Fraction a(40902,24140), b(7,66), c(17,12);
	cout << "a = " << a << "\n";
	cout << "b = " << b << "\n";
	cout << "c = " << c << "\n";
	cout << "7/66 + 17/12 = " << (b+c) << "\n";
	cout << "7/66 - 17/12 = " << (b-c) << "\n";
	cout << "7/66 * 17/12 = " << (b*c) << "\n";
	cout << "7/66 / 17/12 = " << (b/c) << "\n";
	cout << "7/66 < 11/100 = " << (b<Fraction(11,100)) << "\n";
	cout << "Fraction(0.5) = " << Fraction(0.5) << "\n";
	cout << "Fraction(0.333333e6) = " << Fraction(0.333333e6) << "\n";
	cout << "Fraction(0.333333e-6) = " << Fraction(0.333333e-6) << "\n";
	cout << "(float)Fraction(1,2) = " << (float)Fraction(1,2) << "\n";
}
