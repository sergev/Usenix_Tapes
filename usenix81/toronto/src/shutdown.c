/* shutdown -- more or less safely shut a version 7 unix system down.
 */

#include <stdio.h>
#include <utmp.h>

main()
	{
	FILE *utmp;

	/* make sure sole user is at console */
	if(strcmp(ttyname(1), "/dev/console"))
		{
		printf("you must shut the system down from the console\n");
		exit(1);
	}
	if((utmp = fopen("/etc/utmp", "r")) == NULL)
		{
		printf("could not open utmp\n");
		exit(1);
	}
	if(! single(utmp)){
		printf("somebody else is still on the system. shutdown aborted.\n");
		execl("/usr/bin/apb", "apb", " I would like to shutdown.", 0);
		exit(1);
	}
	printf("When the single user prompt ('#') is printed, please\n");
	printf("type sync, then halt the cpu\n");
	sync();
	kill(1, 1);
	while(1)
		{
		sync();
		sleep(1);
		}
	}



single(fileid)
	FILE *fileid;
	{
	struct utmp utmp;

	while(!feof(fileid) && !ferror(fileid))
		{
		fread(&utmp, sizeof utmp, 1, fileid);
		if(utmp.ut_name[0] && strcmp(utmp.ut_line, "console"))
			return(0);
		}
	return(1);
	}
