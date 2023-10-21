/* zap: interactive process killer
	from Kernighan & Pike p. 191 */
static char SCCSID[] = "@(#)zap.c	Ver. 1.1, 85/03/04 14:10:58";
#include <stdio.h>
#include <signal.h>
char *progname ;	/* name of program (zap) */
char *ps ;	/* form of the ps command */

main(argc,argv)
	int argc;
	char *argv[];
{
	FILE *fin, *popen();
	char buf[BUFSIZ];
	int pid;

	if(geteuid() == 0)
		ps = "ps -ag" ;	/* only root can kill others' programs */
	else
		ps = "ps -g" ;	/* for all ordinary users */

	progname=argv[0];
	if((fin=popen(ps,"r")) == NULL) {
		fprintf(stderr,"%s: can't run %s\n",progname,ps);
		exit(1);
	}
	fgets(buf,sizeof buf,fin); /* get header line */
	fprintf(stderr,"%s",buf);
	while(fgets(buf,sizeof buf,fin) != NULL)
		if(argc == 1 || strindex(buf,argv[1]) >= 0) {
			buf[strlen(buf)-1] = '\0'; /* suppress \n */
			fprintf(stderr,"%s\t? ",buf);
			if(ttyin() == 'y') {
				sscanf(buf,"%d",&pid);
				kill(pid,SIGKILL);
			}
		}
	exit(0);
}
