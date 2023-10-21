#include <osfcn.h>
#include <fcntl.h>
#include "DateDict.h"
#include "Date.h"
#include "SortedCltn.h"

main()
{
	istream in(open("datedict.dat",O_RDONLY));
	DateDict& d = *(DateDict*)readFrom(in,"DateDict");
	Date date;
	while (YES) {
		cout << "\nEnter date: ";
		cin >> date;
		if (cin.eof()) exit(0);
		if (d.includesKey(date)) cout << d.lookup(date);
		else cout << "not found";
	}
}
