/* Test class Range

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
	
$Log:	range.c,v $
 * Revision 1.1  88/01/17  22:25:00  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: range.c,v 1.1 88/01/17 22:25:00 keith Exp $";

#include "Range.h"

main()
{
	Range a;
	a.firstIndex(0);  a.lastIndex(1);
	cout << a << "\n";
	Range b(3,14);
	cout << "firstIndex(): " << b.firstIndex() << " lastIndex(): " << b.lastIndex() << " length(): " << b.length() << "\n";
	cout << a << "\n";
	cout << b << "\n";
	a=b;
	cout << a << "\n";
}
