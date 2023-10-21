#include <stdio.h>

touch(argc,argv)
	char *argv[];
{
	while(--argc)
		utime(*(++argv),NULL);
}
