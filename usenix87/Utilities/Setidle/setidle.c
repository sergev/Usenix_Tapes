#include <stdio.h>
#include <sys/types.h>

main(argc,argv)
int	argc;
char	**argv;
{
	long	now;
	long	idle[2];
	long	i;
	int	k;

	nice(20);

	if (argc == 1) {
		i = 0;
		k = 600;
	} else if (argc == 3) {
		sscanf(argv[1],"%d",&i);
		sscanf(argv[2],"%d",&k);
	} else {
		fprintf(stderr,"Useage: setidle [idle time] [update time]\n");
		exit(1);
	}

	while (getppid() != 1) {
		time(&now);
		idle[0] = now - i;
		idle[1] = now - i;
		utime(ttyname(1),idle);
		sleep(k);
	}
	exit(0);
}
