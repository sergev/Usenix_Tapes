/**			center.c		 **/

/** Simple program that filters stdin to stdout centering each line
    it reads.  For batch/shell script processing, mostly. **/

#include <stdio.h>

main()
{
	char buffer[100];
	register int i;

	while (gets(buffer, 100) != NULL) {
	  for (i = 0; i < (80-strlen(buffer))/2; i++)
	    putchar(' ');
	  printf("%s\n", buffer);
	}
}
