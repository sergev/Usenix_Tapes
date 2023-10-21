#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

#include <stdio.h>

extern FILE *popen();

curdir(pwd)
char *pwd;		/* Fill this in with the current directory */
{
	register FILE *fp;
	int reply = -1;

	if ((fp = popen("pwd", "r")) != NULL) {
		reply = 0;
		fscanf(fp, " %s ", pwd);
		pclose(fp);
	}
	return(reply);
}
