#include <stdio.h>
rd(argc,argv)
char *argv[];
{
	static char *usage = "usage : rmdir directory";
	if (argc == 1)
		write(2,usage,strlen(usage));
	if (-1 == rmdir(*(++argv)))
	{
		perror("rmdir");
		return -1;
	}
	return 0;
}
