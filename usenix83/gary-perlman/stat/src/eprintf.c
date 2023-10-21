#include	<stdio.h>
/* error exit fprintf to stderr */
eprintf(fmt, args) char *fmt;
	{
	fputc ('', stderr);
	_doprnt(fmt, &args, stderr);
	fputc ('\n', stderr);
	exit(1);
	}
