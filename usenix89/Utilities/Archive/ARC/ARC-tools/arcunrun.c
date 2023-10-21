#include <stdio.h>
#define CODE	0x90
main()
{
	register c, lastc;

	while((c = getchar()) != EOF) {
		if(c == CODE) {
			c = getchar();	/* count */
			if(c) {
				while(--c) {
					putchar(lastc);
				}
			} else {
				putchar(CODE);
			}
		} else {
			putchar(c);
			lastc = c;
		}
	}
}
