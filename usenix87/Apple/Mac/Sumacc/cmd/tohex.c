
#include <stdio.h>

char hex[] = "@ABCDEFGHIJKLMNO";
int bytes,sum;
unsigned long htonl();

main(argc,argv)
	char **argv;
{
	register i,len;
	register char *cp;

	len = 0;
	while ((i = getchar()) != EOF) {
		bytes++;
		sum += i;
		putchar(hex[i>>4]);
		putchar(hex[i&0xF]);
		if (++len > 32) {
			putchar('\n');
			len = 0;
		}
	}
	fprintf(stderr, "bytes %d, sum %d\n", bytes, sum);
	putchar('|');
	sum += bytes;
	sum = htonl(sum);
	cp = (char *)&sum;
	for (len = 0 ; len < 4 ; len++) {
		i = (*cp++ & 0xff);
		putchar(hex[i>>4]);
		putchar(hex[i&0xF]);
	}
	putchar('\n');
	exit(0);
}

#define nohtonl
#ifdef nohtonl	/* if not in library */
/*
 * "Host" to "net" byte order swappers.
 */
unsigned short htons(a)
	unsigned short a;
{
	unsigned short result;
	register char *sp = (char *)&a;
	register char *dp = (char *)&result;

	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}


unsigned long htonl(a)
	unsigned long a;
{
	unsigned long result;
	register char *sp = (char *)&a;
	register char *dp = (char *)&result;

	dp[3] = *sp++;
	dp[2] = *sp++;
	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}
#endif
