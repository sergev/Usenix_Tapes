#

/*
 *      System Usage Program
 *
 *
 * This program prints the statistics collected by "ktd" and plots
 * graphs versus time.  Arguments in the "exec" determine which of
 * the values are desired.  A new page is begun for each new day.
 *
 *
 * Author:  John Bruner
 *
 */

#include        "/usr/sys/stats.h"
#include	<ktd.h>

char *title[]{
	"\t\t\tUser Process Time\n\t\t\t (Seconds/Minute)",
	"\t\t\tKernel Process Time\n\t\t\t  (Seconds/Minute)",
	"\t\t\tSystem Interrupt Time\n\t\t\t   (Seconds/Minute)",
	"\t\t\tSwapping Activity\n\t\t\t (Seconds/Minute)",
	"\t\t\tCPU Idle Time\n\t\t       (Seconds/Minute)",
	"\t\t\tRP04 Activity\n\t\t       (Seconds/Minute)",
	"\t\t\tNumber of RP Transfers",
	"\t\t\tUsers Logged In"
};

main(argc, argv)
char **argv;
{

	int j, indx;
	int *localtime();
	char *dec();
	register fd, *p, k;
	extern fout;

	if ((fd=open(KTMP,0)) < 0){
		printf("Can't open %s\n", KTMP);
		exit();
	}


	indx = NMON;
	fout = dup(1);

	do {
		if (argc > 1){
			if (**++argv >= '0' && **argv < '0'+NMON+1)
				indx = **argv - '0';
		}
	
		j = -1;
		seek(fd, 0, 0);
		while(read(fd, &sum, sizeof sum) == sizeof sum){
			p = localtime(sum.timev);
			if (p[7] != j){
				j = p[7];
				putchar(014);
				printf(title[indx]);
				printf("\n\n%d/%d/%d:\n", p[4]+1, p[3], p[5]);
				if (indx >= NMON){
					putchar('\n');
					printf("* -- student terminals\n");
					printf("& -- system terminals\n");
					printf("$ -- technical typing\n\n");
				}
			}
			printf("%2d:%s:\t", p[2], dec(p[1], 2));
			if (indx < NMON){
				for(k=0; k<sum.cnt[indx]; k++){
					if ((k+1)%5 == 0) putchar('&');
					else putchar('*');
				}
			} else {
				for(k=0; k<sum.u_count[0]; k++) putchar('*');
				for(k=0; k<sum.u_count[1]; k++) putchar('&');
				for(k=0; k<sum.u_count[2]; k++) putchar('$');
			}
			putchar('\n');
		}
	} while(--argc > 1);
	close(fd);
	flush();
}
char *dec(val, noc){

	static char buf[10];
	register j;

	for (j=noc-1; j>=0; j--){
		buf[j] = '0' + val%10;
		val =/ 10;
	}
	buf[noc] = 0;
	return(buf);
}
