#include <stdio.h>
cd(argc,argv)
char *argv[];
{
	static char *usage = "usage : cd newdir";
	if (argc == 1)
		write(2,usage,strlen(usage));
	if (-1 == chdir(*(++argv)))
	{
		perror("cd");
		return -1;
	}
	return 0;
}
