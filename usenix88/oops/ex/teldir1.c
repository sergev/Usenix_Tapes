#include "Dictionary.h"
#include "String.h"

main()
{
	Dictionary teldir;
	teldir.addAssoc(*new String("Smith, Mary"), *new String("123-1111"));
	teldir.addAssoc(*new String("Doe, John"), *new String("456-2222"));
	teldir.addAssoc(*new String("Jones, Bill"), *new String("789-3333"));

	String name;
	while (YES) {
		cout << "\nEnter name: ";
		cin >> name;
		if (cin.eof()) exit(0);
		if (teldir.includesKey(name)) cout << *teldir.atKey(name);
		else cout << "not found";
	}
}
