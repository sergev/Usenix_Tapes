#include <stdio.h>
static char SCCSID[] = "@(#)ttyin.c	Ver. 1.3, 85/03/04 16:04:32";
ttyin() /* process response from /dev/tty -- Kernighan & Pike p. 185 */
{
	char buf[BUFSIZ];
	FILE *fopen();	/* K&P use their own efopen, p. 182 */
	static FILE *tty=NULL;

	if(tty==NULL)
		tty=fopen("/dev/tty","r");
	for(;;) {
		if(fgets(buf,BUFSIZ,tty)==NULL || buf[0]=='q')
			exit(0);
		else if(buf[0]=='!') {
			system(buf+1);	/* p. 224, available as system(3) */
			printf("!\n");
		}
		else	/* ordinary line */
			return buf[0];
	}
}
