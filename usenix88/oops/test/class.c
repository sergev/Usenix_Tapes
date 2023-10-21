/* Test class Class

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
	
$Log:	class.c,v $
 * Revision 1.2  88/01/18  11:29:00  keith
 * Remove classIdent()
 * 
 * Revision 1.1  88/01/17  22:24:31  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: class.c,v 1.2 88/01/18 11:29:00 keith Exp $";

#include "Dictionary.h"
#include "SortedCltn.h"

extern Dictionary classDictionary;

main()
{
	cout << "\nDump of Dictionary of all Classes\n";
	cout << classDictionary.asSortedCltn();
}
