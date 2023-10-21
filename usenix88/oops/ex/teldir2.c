#include "Dictionary.h"
#include "String.h"
#include <osfcn.h>
#include <fcntl.h>

static Dictionary& readDirectory()
{
	int fd = open("teldir2.dat",O_RDONLY);
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
		cout << "\nOperation (f - find, a - add, r - remove, q - quit): ";
		cin >> op;
		if (op == 'q') {	// quit
			{
				ostream out(open("teldir2.dat",O_WRONLY|O_CREAT,0664));
				teldir.storeOn(out);
			}
			exit(0);
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
					teldir.addAssoc(
						*new String(name),
						*new String(phone));
				}
				break;
			}
			case 'r': {	// remove
				if (teldir.includesKey(name))
					teldir.removeKey(name);
				else cout << "not found";
				break;
			}
			default: cout << "Invalid operation";
		}
	}
}
