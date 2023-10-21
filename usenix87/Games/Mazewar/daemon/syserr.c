#ifndef lint
static char rcsid[] = "$Header: syserr.c,v 1.1 84/08/25 17:04:57 lai Exp $";
#endif

/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>

#ifndef NULL
#define NULL	0
#endif

#ifndef	BUFSIZ
#define BUFSIZ	1024
#endif

#ifndef	_IOSTRG
#define _IOSTRG	0
#endif

extern int _doprnt();

FILE	*errfp;		/* error log file pointer (global) */

extern	int	errno;
extern	char	*sys_errlist[];
extern	int	sys_nerr;


/*VARARGS1*/
syserr(format, va_alist)
	char *format;
	va_dcl
{
	va_list ap;

	if (errfp == NULL)
		return (-1);
	else {
		if (format != NULL) {
			va_start(ap);
			_doprnt(format, ap, errfp);
			va_end(ap);

			fputs(": ", errfp);
		}

		if (0 <= errno && errno < sys_nerr &&
		    sys_errlist[errno] != NULL)
			fprintf(errfp, "%s\n", sys_errlist[errno]);
		else
			fprintf(errfp, "Error %d\n", errno);

		fflush(errfp);

		return (ferror(errfp) ? EOF : 0);
	}
}

/*VARARGS1*/
syslog(format, va_alist)
	char *format;
	va_dcl
{
	va_list ap;

	if (errfp == NULL)
		return (-1);
	else {
		if (format != NULL) {
			va_start(ap);
			_doprnt(format, ap, errfp);
			va_end(ap);
		}

		fflush(errfp);

		return (ferror(errfp) ? EOF : 0);
	}
}
