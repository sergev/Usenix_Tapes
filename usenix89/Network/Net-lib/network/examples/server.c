#ifndef lint
static char     rcsid[] = "$Header$";
#endif

#include <stdio.h>
#include <ctype.h>
#include <signal.h>

typedef enum { false = 0, true = 1} bool;

char           *progname;

int             p_val,s;

exiter()
{
	fprintf(stderr,"Shutting down...\n");
	pkill(s);
	exit(0);
}

main(argc, argv)
	int             argc;
	char           *argv[];
{
	extern int      opterr;
	extern char    *optarg;
	int             c;

	opterr = 1;
	progname = argv[0];

	p_val = 2500;

	while ((c = getopt(argc, argv, "p:")) != EOF)
		switch (c) {
		case 'p':
			p_val = atoi(optarg);
			break;
		case '?':
			usage();
			break;
		}

	signal(SIGHUP,exiter);
	signal(SIGINT,exiter);
	signal(SIGQUIT,exiter);
	signal(SIGTERM,exiter);

	do_prog();
	exit(0);
}

usage()
{
	fprintf(stderr, "Usage: %s [ -p port ] \n", progname);
	exit(1);
}
do_prog()
{
	int c,q,n;
	char buff[1024];

	s=attach(useport(p_val));

	if(s==-1) {
		perror("attach");
		exit(-1);
	}

	pkill(0);

	initfd();

	addfd(s);

	while(1) {
		c=block();
		if(c==s) {
			q=answer(s);
			if(q==-1) {
				perror("answer");
			} else {
				addfd(q);
				printf("Conenction accepted: %d\n",q);
			}
		} else {
			n=read(c,buff,sizeof(buff)-1);
			if(n) {
				buff[n]=0;
				printf("%d: %s\n",c,buff);
			} else {
				delfd(c);
				printf("File descriptor closed: %d\n",c);
				pkill(c);
			}
		}
	}
}
