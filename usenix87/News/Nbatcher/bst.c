/*
 *	bst.c - a utility for indicating how many
 *		news articles are ready for batching
 *		for each site in the BACTHDIR directory.
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>


char buf[512];
FILE *dd;
struct direct dp;
struct stat st;

main()
{
	int fd, j;
	int bcnt, lcnt;

	if(chdir(BATCHDIR) != 0) {
		perror(BATCHDIR);
		exit(1);
	}

	if((dd=fopen(".", "r")) == NULL) {
		printf("can't open %s\n", BATCHDIR);
		exit(1);
	}

	while((fread((char *)&dp, sizeof(dp), 1, dd)) == 1) {
		if(dp.d_ino == 0 || dp.d_name[0] == '.')
			continue;
		if(stat(dp.d_name, &st) != 0) {
			printf("can't stat %s\n", dp.d_name);
			exit(1);
		}
		if(st.st_size <= 0 )
			continue;
		if((fd=open(dp.d_name, 0)) < 0) {
			printf("can't open %s\n", dp.d_name);
			exit(1);
		}
		lcnt = 0;
		while((bcnt=read(fd,buf,512)) > 0) {
			for(j=0; j<=bcnt; j++)
				if(buf[j] == '\n')
					lcnt += 1;
		}
		close(fd);
		printf("%s\t  %d article", dp.d_name, lcnt);
		printf("%c\n", lcnt > 1 ? 's' : ' ');
	}
}
