#include <stdio.h>
main()
{
	register c, i, size;

	if(getchar() != 0x1a) {
		fprintf(stderr, "Not in ARC format\n");
		exit(1);
	}
	c = getchar();
	for(i=0; i<13; i++) {
		c = getchar();
	}
	size = getchar(); size = (getchar() << 8) + size; 
	size = (getchar() << 16) + size; size = (getchar() << 24) + size;
	for(i=10; i>0; i--) {
		c = getchar();
	}
	while(size--) {
		c = getchar();
		putchar(c);
	}
	return 0;
}
