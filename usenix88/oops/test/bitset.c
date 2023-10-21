/* Test class Bitset

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
	
$Log:	bitset.c,v $
 * Revision 1.1  88/01/17  22:24:29  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: bitset.c,v 1.1 88/01/17 22:24:29 keith Exp $";
#include "Bitset.h"
main()
{
	Bitset a(10),b(10,11);
	cout << "a = " << a << "\n";
	cout << "b = " << b << "\n";
	a = 1;
	a = a | 2 | 3;
	b = 2;
	b |= 4;
	cout << "a = " << a << "\n";
	cout << "a.size(): " << a.size() << "\n";
	cout << "b = " << b << "\n";
	cout << "a&b = " << (a&b) << "\n";
}
