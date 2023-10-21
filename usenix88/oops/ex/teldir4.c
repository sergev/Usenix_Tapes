#include "Dictionary.h"
#include "LookupKey.h"
#include "String.h"
#include "Record.h"
#include "SortedCltn.h"
#include <osfcn.h>
#include <fcntl.h>

static Dictionary& readDirectory()
{
	int fd = open("teldir4.dat",O_RDONLY);
	if (fd >= 0) {
		istream in(fd);
		return *(Dictionary*)readFrom(in,"Dictionary");
	}
	return *new Dictionary;		// return empty directory
}

main()
{
	Dictionary& teldir = readDirectory();
	char op;
	String name;
	while (YES) {
		cout << "\nOperation (f - find, a - add, r - remove, l - list, q - quit): ";
		cin >> op;
		switch (op) {
			case 'l': {	// list
				SortedCltn s = teldir.asSortedCltn();
				DO(s,Record*,r) cout << *r << "\n"; OD
				continue;
			}
			case 'q': {	// quit
				{
					ostream out(open("teldir4.dat",O_WRONLY|O_CREAT,0664));
					teldir.storeOn(out);
				}
				exit(0);
			}
		}
		cout << "Name: ";
		cin >> name;
		switch (op) {
			case 'f': {	// find
				if (teldir.includesKey(name))
					cout << *teldir.atKey(name);
				else cout << "not found";
				break;
			}
			case 'a': {	// add
				if (teldir.includesKey(name))
					cout << "already in directory";
				else {
					String address,phone;
					int zip;
					cout << "Address: ";
					cin >> address;
					cout << "ZIP: ";
					cin >> zip;
					cout << "Telephone number: ";
					cin >> phone;
					String& key = *new String(name);
					teldir.addAssoc(key,*new Record(key,address,zip,phone));
				}
				break;
			}
			case 'r': {	// remove
				if (teldir.includesKey(name)) {
					LookupKey* a = &teldir.removeKey(name);
					delete a->key();  delete a->value();
					delete a;
				}
				else cout << "not found";
				break;
			}
			default: cout << "Invalid operation";
		}
	}
}
