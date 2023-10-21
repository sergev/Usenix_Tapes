/* Test class Date

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
	
$Log:	date.c,v $
 * Revision 1.1  88/01/17  22:24:32  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: date.c,v 1.1 88/01/17 22:24:32 keith Exp $";
#include "Date.h"
#include "SortedCltn.h"

main()
{
	cout << "\nTest class Date\n";
	char junk[100];
	Date today;
	Date tomorrow(today+1);
	Date yesterday(today-1);
	Date reference(25,"Dec",85);
	SortedCltn datelist;
	cerr << "Today is " << nameOfDay(today.weekDay()) << ", " << today << "\n";
	cerr << "Tomorrow is " << nameOfDay(tomorrow.weekDay()) << ", " << tomorrow << "\n";
	cerr << "Yesterday was " << nameOfDay(yesterday.weekDay()) << ", " << yesterday << "\n";
	while (YES) {
		cout << "Enter date: "; Date& date = *new Date(cin); cout << "\n";
		if (cin.eof()) break;
		if (cin.fail()) { cout << "Bad date\n";  cin.clear();  cin.get(junk,sizeof junk);  continue; }
		cout << nameOfDay(date.weekDay()) << ", " << date << "\n";
		cout << reference << " - " << date << " = " << (reference-date) << "\n";
		cout << date << " between(1-Jan-85, 31-Dec-85): " << (date.between(Date(1,"Jan",85), Date(31,"Dec",85))) << "\n";
		cout << date << " max(" << reference << ") = " << date.max(reference) << "\n";
		cout << date << " min(" << reference << ") = " << date.min(reference) << "\n";
		cout << "The date of the previous Sunday is " << date.previous("Sun") << "\n";
		datelist.add(date);
		cout << datelist << "\n";
	}
}
