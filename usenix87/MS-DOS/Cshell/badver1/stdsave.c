#include <stdio.h>
#include <fcntl.h>

int console;
int in,out;	/* old standard input and standard output */
std_save()
{
	if (-1 == (console = open("con",O_RDWR)))
	{
		perror("sh : can't open console");
		return -1;
	}
	in = dup(0);
	out = dup(1);
	fdup(console,0);
	fdup(console,1);
	fdup(console,2);
	return 0;
}
void 
std_restore()
{
	fdup(in,0);
	fdup(out,1);
	close(console);
	close(in);
	close(out);
}
