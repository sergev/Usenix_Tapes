
/*
    Try to repair uuencoded files when trailing space has been removed.
    It pads short lines with spaces whenever needed.
    It reads from stdin and writes to stdout.

	use: pgm <uuencoded > repaired
	     uudecode repaired

	or:  pgm <uuencoded | uudecode

    If it doesn't work, I can't help you.
*/

#include <stdio.h>

main()
  {
    char c;
    int tab, len;

    tab = 0;
    while ((c = getchar()) != EOF)
      {
	if (tab == 0)
	  {
	    if (' '<=c && c<='Z') len = ((c-' '+2)/3)*4+1;
	    else len = 1;
	  }
	if (c == '\n')
	  {
	    while (tab++<len) putchar(' ');
	    tab = 0;
	  }
	else tab++;
	putchar (c);
      }
  }
/* End of text from mirror:comp.sys.ibm.pc */
