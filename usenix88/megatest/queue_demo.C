#include "queue.H"
#include <stream.h>
#include <string.h>

inline char* strcopy(char* str) { return strcpy(new char[strlen(str)], str); }
Queue foo;

main()
{
  char cmd;
  do
    {
      char val[128];
      cout << ". ";
      cin >> cmd;
      switch(cmd)
	{
	case 'p':   // push a string
	  cin >> val;
	  foo.push(strcopy(val));
	  break;
	case 'g':  // get top string and pop
	  cout << (char*)foo.pop() << "\n";
	  break;
	case 'a':  // append string to end of queue
	  cin >> val;
	  foo.append(strcopy(val));
	  break;
	case 'i':  // iterate through the queue.
	  { Queue_iterator next(foo);
	    while(!next.done())
	      cout << (char*)next() << "\n";
	    break;
	  }
	}
    }
  while(cmd != 'q');
}
