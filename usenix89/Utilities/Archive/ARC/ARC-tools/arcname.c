#include <stdio.h>
main()
{
	char fnm[13];
	register unsigned c, i, code;

	if(getchar() != 0x1a) {
		fprintf(stderr, "Not in ARC format\n");
		exit(9);
	}
	code = getchar();	/* code (should be 8) */
	for(i=0; i<13; i++) {
		c = getchar();
		if(c >= 'A' && c <= 'Z') {
			c += 'a' - 'A';
		}
		fnm[i] = c;
	}
	printf("%s\n", fnm);
	return code;
}
