#include <stdio.h>
main()
{
	register c, i, size;

	if(getchar() != 0x1a) {
		fprintf(stderr, "Not in ARC format\n");
		exit(1);
	}
	if((c = getchar()) != 8) {
		fprintf(stderr, "Can only handle code 8 ARC files\n");
		fprintf(stderr, "This file is code %d\n", c);
		exit(1);
	}
	for(i=0; i<13; i++) {
		c = getchar();
	}
	size = getchar(); size = (getchar() << 8) + size; 
	size = (getchar() << 16) + size; size = (getchar() << 24) + size;
	for(i=10; i>0; i--) {
		c = getchar();
	}
	c = getchar();
	putchar(0x1f); putchar(0x9d); putchar((0x80 | c));
	while(--size) {
		c = getchar();
		putchar(c);
	}
	return 0;
}
