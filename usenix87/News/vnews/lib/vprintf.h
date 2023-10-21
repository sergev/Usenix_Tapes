/*
 * These defines allow you to define routines that call printf on their
 * arguments in a way that at least marks them as machine dependent.
 * Example:
 *
 * fatal(VPARGS)
 *	VPDCL;
 *	{
 *	VFPRINTF(stderr, VPARGS);
 *	exit(1);
 * }
 *
 * Later:  use varargs.h.
 */

#define VPARGS msg, a1, a2, a3, a4
#define VPDCL char *msg;
#define VPRINTF(args) printf(VPARGS)
#define VFPRINTF(fp, args) fprintf(fp, VPARGS)
#define VSPRINTF(s, args) sprintf(s, VPARGS)
