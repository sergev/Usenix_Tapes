#include "Dictionary.h"
#include "LookupKey.h"
#include "String.h"
#include "TelDict.h"
#include <osfcn.h>
#include <fcntl.h>

static TelDict& readDirectory()
{
	int fd = open("teldir3.dat",O_RDONLY);
	if (fd >= 0) {
		istream in(fd);
		return *(TelDict*)readFrom(in,"TelDict");
	}
	return *new TelDict;		// return empty directory
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
				cout << teldir;
				continue;
			}
			case 'q': {	// quit
				{
					ostream out(open("teldir3.dat",O_WRONLY|O_CREAT,0664));
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
					String phone;
					cout << "Telephone number: ";
					cin >> phone;
					teldir.addAssoc(*new String(name),*new String(phone));
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
