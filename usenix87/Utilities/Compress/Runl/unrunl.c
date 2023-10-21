/* Decoder of files run-length encoded by runl. */
/* Main slowness is put/get char. */

#define SPECIAL 0267
#include <stdio.h>

main()
{register int c, cnt;
 nextchar:
  while ((c=getchar()) != SPECIAL) putchar(c);	/* Copy normal characters */
  if ((cnt=getchar()) == 0) goto end;		/* SP 0 means EOF */
  if (cnt == 1) {putchar(c); goto nextchar;};	/* SP 1 means SP */
  c=getchar();
  while (cnt--) putchar(c);			/* Multiply the char */
  goto nextchar;

 end: ;
}
