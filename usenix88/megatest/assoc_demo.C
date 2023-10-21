#include "Assoc.H"
#include <stream.h>

char* copy(char* str)
{
  return strcpy(new char[strlen(str)], str);
}

NAME_TABLE(String, char*)

String_table table;

main()
{
  cout << "\nTo put an entry, type p <key> <contents>\n"
       << "To get an entry, type g <key>\n"
       << "To show all entries, type s\n"
       << "To delete an entry, type d\n"
       << "To count the entries, type c\n\n"
  ;
 
  char key[128];
  char cont[128];
  char command;

  do
    {
      cout << ". ";
      cin >> command;
      switch(command)
	{
	  break;case 'q': {}

	  break;case 'p':
            cin >> key >> cont;
	    table[key] = copy(cont);

	  break;case 'g':
	    cin >> key;
	    cout << table[key] << "\n";

	  break;case 's':
	  { 
	    String_entry *p;
	    String_iterator next(table);
	    while(p = next())
	      cout << p->index() << " " << *(p->value()) << "\n";
	  }

	  break;case 'd':
	    cin >> key;
	    delete table[key];
	    table.remove(key);

	  break;case 'c':
	    cout << table.entries() << "\n";

	  break; default:
	    cout << "Huh?\n";

	  break;
	}
    }
  while(command != 'q');

}

