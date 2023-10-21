#include "Assoc.H"

#include <stream.h>
#include <string.h>
#include <ctype.h>

//
// Give this program "foo.o" in its argv, and it will filter out 
// dependancy lines such as
//
// foo.o: foo.c
// foo.o: bar.h
//
// 

NAME_TABLE(bool, int)
bool_table doomed;     // table of all doomed targets.

main(int argc, char** argv)
{

  argc--; argv++;
  int maxlen = 0;

  while(argc--)
    {
      doomed[*argv] = 1;

      if(strlen(*argv) > maxlen) maxlen= strlen(*argv);
      argv++;
    }

  char* prefix = new char[maxlen+1];

  while(1)
    { 
      int i = 0; char ch;
      while(i < maxlen)
	{
	  cin.get(ch);
	  if(cin.good() && ch != ':' && !isspace(ch) )
	    prefix[i++] = ch;
	  else
	    { if(cin.good()) cin.putback(ch);
	      break;
	    }
	}
      prefix[i] = 0;

      int dont_print = doomed[prefix];

      /* flush prefix */
      if(!dont_print) cout << prefix;

      /* flush remainder of line */
      do
	{ cin.get(ch);
	  if(!cin.good())  exit(cin.eof() ? 0 : -1);
	  if( !dont_print  ) cout.put(ch);
	}
      while( ch != '\n');
  
    }
}
