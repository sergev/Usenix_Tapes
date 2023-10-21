/* Test class Random

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
	
$Log:	random.c,v $
 * Revision 1.1  88/01/17  22:24:59  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: random.c,v 1.1 88/01/17 22:24:59 keith Exp $";

#include "Random.h"

main()
{
	Random rangen;
	for (register i=10; i>0; i--) cerr << rangen.next() << "\n";
	Random rand(0);
	for (i=10; i>0; i--) cout << rand.next() << "\n";
}
