/* Test class IdentDict

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
	
$Log:	identdict.c,v $
 * Revision 1.1  88/01/17  22:24:46  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: identdict.c,v 1.1 88/01/17 22:24:46 keith Exp $";

#include "Point.h"
#include "IdentDict.h"
#include "Assoc.h"
#include "AssocInt.h"
#include "Bag.h"
#include "OrderedCltn.h"

main()
{
	cout << "\nTest class IdentDict\n";
	IdentDict d(16);
	Point a(1,1), b(1,2), c(1,3);
	AssocInt asc(c,3);
	d.add(AssocInt(a,1));
	d.add(Assoc(b,Point(2,2)));
	d.add(asc);
	cout << "d = " << d << "\n";
	cout << "d.atKey(a): " << *(d.atKey(a)) << "\n";
	cout << "d.includesKey(Point(1,1)): " << d.includesKey(Point(1,1)) << "\n";
	d.atKey(a,Integer(0));
	cout << "d = " << d << "\n";
	cout << "d.includesAssoc(asc): " << d.includesAssoc(asc) << "\n";
	cout << "d.includesKey(" << (*asc.key()) << "): " << d.includesKey(*asc.key()) << "\n";
	cout << "d.keyAtValue(Integer(0)) = " << *d.keyAtValue(Integer(0)) << "\n";
	d.removeKey(c);
	cout << "d.includesAssoc(asc): " << d.includesAssoc(asc) << "\n";
	cout << "d.includesKey(" << (*asc.key()) << "): " << d.includesKey(*asc.key()) << "\n";
	cout << "d.asBag: " << d.asBag() << "\n";
	OrderedCltn keys,vals;
	cout << "d.addKeysTo(keys): " << d.addKeysTo(keys) << "\n";
	cout << "d.addValuesTo(vals): " << d.addValuesTo(vals) << "\n";
}
