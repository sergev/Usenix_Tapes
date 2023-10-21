/* Test class Dictionary

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
	
$Log:	dict.c,v $
 * Revision 1.1  88/01/17  22:24:35  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: dict.c,v 1.1 88/01/17 22:24:35 keith Exp $";

#include "Point.h"
#include "Dictionary.h"
#include "Assoc.h"
#include "Bag.h"
#include "OrderedCltn.h"

main()
{
	cout << "\nTest class Dictionary\n";
	Dictionary d(16);
	Assoc c(Point(1,3),Point(2,3));
	d.add(Assoc(Point(1,1),Point(2,1)));
	d.add(Assoc(Point(1,2),Point(2,2)));
	d.add(c);
	cout << "d = " << d << "\n";
	cout << "d.atKey(Point(1,1)): " << *(d.atKey(Point(1,1))) << "\n";
	d.atKey(Point(1,1),Point(3,1));
	cout << "d = " << d << "\n";
	cout << "d.includesAssoc(c): " << d.includesAssoc(c) << "\n";
	cout << "d.includesKey(*c.key()): " << d.includesKey(*c.key()) << "\n";
	cout << "d.keyAtValue(Point(3,1)) = " << *d.keyAtValue(Point(3,1)) << "\n";
	d.removeKey(*c.key());
	cout << "d.includesAssoc(c): " << d.includesAssoc(c) << "\n";
	cout << "d.includesKey(*c.key()): " << d.includesKey(*c.key()) << "\n";
	cout << "d.asBag: " << d.asBag() << "\n";
	OrderedCltn keys,vals;
	cout << "d.addKeysTo(keys): " << d.addKeysTo(keys) << "\n";
	cout << "d.addValuesTo(vals): " << d.addValuesTo(vals) << "\n";
}
