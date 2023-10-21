/* Run length encoding */

/* Output file is identical to input file except that any sequence of
   four or more identical characters is replaced by a special code:
   SPECIAL <repeat indication> <character>, which represents the
   <character> repeated <repeat indication> times.
   The character SPECIAL in the input file is replaced by SPECIAL 1.
   File ends with SPECIAL 0. */

#include <stdio.h>

#define SPECIAL 0267
    /* 0267 appears to be a relatively little-used char in tar and bin */

/* Main performance problem is get/put char */

main()
{ register int c1,c2,c3,c4;

	c2=getchar();		/* Fill the pipeline */

 restart: c3=getchar(); c4=getchar();

/* Last 4 chars are kept in regs */
/* Nothing is output until it is known whether last 4 chars are equal;
     thus, the code works for repeated SP's as well. */

 main:	if ((c1=getchar())==c2 && c3==c4 && c1==c3) goto repeat;
 put1:  putchar(c2); if (c2==SPECIAL) putchar(1);
	if ((c2=getchar())==c3 && c3==c4 && c1==c3) goto repeat;
 put2:  putchar(c3); if (c3==SPECIAL) putchar(1);
	if ((c3=getchar())==c4 && c3==c2 && c1==c3) goto repeat;
 put3:  putchar(c4); if (c4==SPECIAL) putchar(1);
	if ((c4=getchar())==c1 && c3==c4 && c1==c2) goto repeat;
 put4:  putchar(c1); if (c1==SPECIAL) putchar(1);
	goto main;

 repeat:
  /* Last four chars are identical, and we haven't output any of them */
  if (c1 == EOF) goto end;
  c4 = 4;
  while ((c2=getchar()) == c1) c4++;
  while (c4 > 0)
    { putchar(SPECIAL);
      if (c4 > 0376) putchar(0376); else putchar(c4);
      putchar(c1);
      c4 -= 0376; }
  goto restart;

 end: putchar(SPECIAL); putchar(0);
}
