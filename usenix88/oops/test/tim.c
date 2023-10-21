/* Test class Time

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
	
$Log:	tim.c,v $
 * Revision 1.1  88/01/17  22:25:14  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: tim.c,v 1.1 88/01/17 22:25:14 keith Exp $";

#include "Date.h"
#include "Time.h"
#include "SortedCltn.h"

main()
{
	cout << "\nTest class Time\n";
	Time now;
	int hr,mn,sc;
	char junk[512];
	SortedCltn timelist;
	cerr << "It is now " << now << "\n";
	while (YES) {
		cout << "Enter date and time: ";
		Date& date = *new Date(cin);
		if (cin.eof()) break;
		if (cin.fail()) {
			cout << "Bad date\n";
			cin.clear();  cin >> junk;
			continue;
		}
		cin >> hr >> mn >> sc;
		cout << "\n";
		if (cin.eof()) break;
		if (cin.fail()) {
			cout << "Bad time\n";
			cin.clear();  cin.get(junk,sizeof junk);
			continue;
		}
		Time& time = *new Time(date,hr,mn,sc);
		timelist.add(time);
		cout << time << "\n";
	}
	cout << timelist << "\n";
}
