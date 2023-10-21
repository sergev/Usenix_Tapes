#include <stdio.h>

int nfiles ;
long freq[128] ;


main(argc, argv)
	char **argv ;
	{
	char **ap ;
	FILE *fp ;

	for (ap = argv + 1 ; *ap ; ap++) {
		if ((fp = fopen(*ap, "r")) == NULL)
			printf("Can't open %s\n", *ap) ;
		else {
			charcnt(fp) ;
			nfiles++ ;
			fclose(fp) ;
		}
	}
	printcounts() ;
}


charcnt(fp)
	register FILE *fp ;
	{
	register c ;

	while ((c = getc(fp)) != EOF) {
		if (c == '\n' && getc(fp) == '\n')
			break ;
	}
	while ((c = getc(fp)) != EOF)
		freq[c]++ ;
}


printcounts() {
	int i ;
	long total ;

	total = 0 ;
	for (i = 0 ; i < 128 ; i++)
		total += freq[i] ;
	total += nfiles ;
	for (i = 0 ; i < 128 ; i++) {
		if (freq[i]) {
			if (i >= '!' && i <= '~')
				printf("%c   ", i) ;
			else
				printf("%-4d", i) ;
			printf("%6ld %4.1f\n", freq[i], 100.0 * (double)freq[i] / total) ;
		}
	}
	printf("EOF %6ld %4.1f\n", nfiles, (double)nfiles / (total * 100.0)) ;
}
