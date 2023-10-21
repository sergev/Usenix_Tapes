#include <stdio.h>
#include <signal.h>

char *progname;
char *ps = "ps -ag";

main(argc, argv)
int argc;
char *argv[];
{
	FILE *fin, *popen();
	char buf[BUFSIZ];
	char a;
	int pid;
	long hpid;

	progname = argv[0];

	if((fin = popen(ps,"r")) == NULL) {
	fprintf(stderr,"%s: Cannot run  %s \n",progname,ps);
	exit(1);
	}
	       fprintf(stderr," k for kill ; s for stop ; c for continue.\n");
	       hpid = getpid();
	       fgets(buf, sizeof buf, fin);
	       fprintf(stderr, "%s",buf);
	       while(fgets(buf, sizeof buf, fin) != NULL)
		if(argc == 1 || strindex(buf, argv[1]) >= 0) {
			buf[strlen(buf)-1] = '\0';
			sscanf(buf, "%d", &pid);
			if(pid != hpid ){
			fprintf(stderr, "%s? ",buf);
			a = ttyin();
			if(a == 'k')
				kill(pid,SIGKILL);
			if(a == 's')
				kill(pid,SIGSTOP);
			if(a == 'c')
				kill(pid,SIGCONT);
			}
		}
	exit(0);
}

-------------------------------------------------------
Michael Rivero
Robert Abel and Associates
randvax!kovacs!rivero
