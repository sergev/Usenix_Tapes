/*
 * Open the .newsrc file, creating it if necessary.
 */

#include <stdio.h>
#include "defs.h"
#include "libextern.h"

char newsrc[64] ;

FILE *
openrc() {
	register char *myrc ;
	register FILE *rcfp ;
	static char READ[] = "r" ;
	char *getenv() ;

	if ((myrc = getenv("NEWSRC")) != NULL)
		strcpy(newsrc, myrc);
	else
		sprintf(newsrc, "%s/%s", userhome, NEWSRC);
	if ((rcfp = fopen(newsrc, READ)) == NULL) {
		newrc() ;
		if ((rcfp = fopen(newsrc, READ)) == NULL)
			xerror("Can't create .newsrc file") ;
	}
	return rcfp ;
}


newrc()
{
	register FILE *fp;
	char users[FPATHLEN];

	if (close(creat(newsrc, 0666))) {
		xerror("Cannot create %s", newsrc);
	}

	sprintf(users, "%s/users", LIB);
	if ((fp = fopen(users, "a")) != NULL) {
		fprintf(fp, "%s\n", username);
		fclose(fp);
		chmod(users, 0666);
	}
}
