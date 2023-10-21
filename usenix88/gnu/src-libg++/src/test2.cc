// This may look like C code, but it is really -*- C++ -*-

/*
 a little test of Obstacks
 Thu Feb 18 11:16:28 1988  Doug Lea  (dl at rocky.oswego.edu)
*/


#include <stddef.h>
#include <Obstack.h>
#include <stream.h>
#include <ctype.h>

main()
{
  char*   s[10000];
  int     n = 0;
  bool    got_one = FALSE;
  Obstack os;
  char    c;

  s[n++] = os.copy("\nunique words:");

  cout << "enter anything at all, end with an EOF(^D)\n";

  while (cin.good() && n < 10000)
  {
    if (cin.get(c) && isalnum(c))
    {
      got_one = TRUE;
      os.grow(c);
    }
    else if (got_one)
    {
      char* current = os.finish(0);
      for (int i = 0; i < n; ++i) // stupid, but this is only a test.
      {
        if (strcmp(s[i], current) == 0)
        {
          os.free(current);
          current = 0;
          break;
        }
      }
      if (current != 0)
        s[n++] = current;
      got_one = FALSE;
    }
  }

  for (int i = 0; i < n; ++ i)
    cout << s[i] << "\n";

  cout << "\n\nObstack vars:\n";
  cout << "alignment_mask = " << os.alignment_mask() << "\n";
  cout << "chunk_size = " << os.chunk_size() << "\n";
  cout << "size = " << os.size() << "\n";
  cout << "room = " << os.room() << "\n";

  cout << "\nend of test\n";

}

