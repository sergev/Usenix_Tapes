#include <stdio.h>
/* Lisa format to normal format converter */
main ()
{
	int c;

	while ( (c=getchar()) != EOF) {
		if (c >= 040 && c < 0177) {
			putchar(c);
			continue;
		}
		if (c >= 0177)
			continue;
		switch (c) {
		case 16:
			for (c=getchar(); c>32; c--) 
				putchar(' ');
			break;

		case '\n':
		case '\t':
			putchar(c);
			break;

		default:
			break;
		}
	}
}
