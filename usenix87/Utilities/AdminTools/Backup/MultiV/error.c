#include <stdio.h>
	extern	int	errno;
	extern	char	*O_name;
/*VARARGS1*/
fatal(fmt, arg1, arg2, arg3)
	char	*fmt;
	int	arg1, arg2, arg3;
{
	warning(fmt, arg1, arg2, arg3);
#ifdef	DEBUG
	abort();
#else
	exit(1);
#endif
}

sfatal(str)
	char	*str;
{
	swarning(str);
#ifdef	DEBUG
	abort();
#else
	exit(1);
#endif
}

/*VARARGS1*/
warning(fmt, arg1, arg2, arg3)
	char	*fmt;
	int	arg1, arg2, arg3;
{
	fprintf(stderr, "%s: ", O_name);
	fprintf(stderr, fmt, arg1, arg2, arg3);
	fprintf(stderr, "\n");
}

swarning(str)
	char	*str;
{
#ifdef	DEBUG
	fprintf(stderr, "(%d)  ", errno);
#endif
	fprintf(stderr, "%s: ", O_name);
	perror(str);
}
