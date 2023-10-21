/* fromhex.c, UNIX version */

#include <stdio.h>

int bytes,sum;

main()
{
	register i,v;
	register n;

	n = 0;
	v = 0;
	while ((i = getchar()) != EOF) {
		i &= 0177;
		if (i == '|') 
			break;
		if (i < 0100 || i > 0117)
			continue;
		v = (v << 4) | (i & 0xF);
		if ((++n & 1) == 0) {
			putchar(v);
			sum += v;
			v = 0;
			bytes++;
		}
	}
	n = 0;
	for (i = 0 ; i < 8 ; i++)
		n = (n << 4) | (getchar() & 0xF);
	if (n != (bytes + sum))
		fprintf(stderr, "bad checksum\n");
	else
		fprintf(stderr, "checksum good!\n");
	exit(0);
}
