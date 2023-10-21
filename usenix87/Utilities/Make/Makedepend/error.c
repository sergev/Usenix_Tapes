/* -- Error handling
 *
 * $File: error.c  1.3  1985-03-29  14:46:48 $
 *
 *	Usage:	fatal(format, args)
 *		fatalmesg(format, args)
 *
 *		error(format, args)
 *		errorc(format, args)
 *
 *		mesg(format, args)
 *		mesgc(format, args)
 *
 *		setprogname(s)
 *
 *	Descr:	Setprogname copies a string to be used as error message
 *		prefix into the external array 'progname'. Normally
 *		this function is called once with argv[0] as argument
 *		when a command begins execution.
 *
 *		Mesg prints a message on stderr, followed by a newline.
 *		Error is like mesg, but prints a prefix.
 *		Fatal is like error, but exits.
 *		Fatalmesg is like mesg, but exits.
 *
 *		Mesgc and errorc suppress the newline, and errorc prints
 *		the prefix only the first time it is called.
 *
 *		If a UNIX system call has returned an error number
 *		in the external variable 'errno', then the
 *		corresponding system error string is also output,
 *		enclosed in parentheses.
 *
 * $Log:	error.c,v $
 * 
 * Revision 1.3  1985-03-29  14:46:48  kim
 * Removed system error printout
 * 
 * Revision 1.3  1985-03-29  14:29:58  kim
 * removed system error printout
 * 
 * Revision 1.2  1985-03-08  01:18:57  kim
 * Removed fatalc.
 * 
 * Revision 1.1  1985-02-18  13:22:12  kim
 * Initial revision
 */

# include <stdio.h>

# define ARGS format,a,b,c,d,e,f
# define PROGNAMELEN 100

fatal(ARGS)		{ errpri(1, 0, 1, ARGS); }
fatalmesg(ARGS)		{ errpri(0, 0, 1, ARGS); }

error(ARGS)		{ errpri(1, 0, 0, ARGS); }
errorc(ARGS)		{ errpri(1, 1, 0, ARGS); }

mesg(ARGS)		{ errpri(0, 0, 0, ARGS); }
mesgc(ARGS)		{ errpri(0, 1, 0, ARGS); }

char		progname[PROGNAMELEN] = "NULL";

setprogname(s) char *s; { strcpy(progname, s); }

static errpri(prf, cont, ex, ARGS)
	int		prf,
			cont,
			ex;
{
	static int	prf_printed;

	if (prf && !prf_printed)
	{
		fprintf(stderr, "%s: ", progname);
		prf_printed++;
	}

	fprintf(stderr, ARGS);

	if (!cont)
	{
		fprintf(stderr, "\n");
		prf_printed = 0;
	}

	if (ex)
		exit(1);
}
