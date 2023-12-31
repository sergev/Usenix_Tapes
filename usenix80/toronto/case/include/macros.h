/*
 *	Numeric() is useful in while's, if's, etc... but don't use *p++
 *	Max() and min() depend on the types of the operands,
 *	Abs() is absolute value.
 */

# define numeric(c)	(c >= '0' && c <= '9')
# define max(a, b)	(a<b ? b : a)
# define min(a, b)	(a>b ? b : a)
# define abs(x)		(x>=0 ? x : -(x))

# define copy(srce, dest)	cat(dest, srce, 0)
# define compare(str1, str2)	strcmp(str1, str2)
# define equal(str1, str2)	!strcmp(str1, str2)
# define length(str)		strlen(str)
# define size(str)		(strlen(str) + 1)

/*
 *	The global variable Statbuf is available for use as a stat(II)
 *	structure.  Note that "stat.h" is included here and should not
 *	be included elsewhere.
 *	Exists(file) returns 0 if the file does not exist;
 *	the flags word if it does ( the flags word is always > 0 ).
 */

# include "stat.h"

struct inode Statbuf;
# define exists(file)		(stat(file, &Statbuf) < 0? 0 : Statbuf.i_mode)

/*
 *	Lsize() is used in conjunction with a stat(II) structure.
 *	It returns the size as a long.
 */

extern long itol();
# define lsize(size0, size1)	itol(size0&0377, size1)

/*
 *	libS.a interface for xopen() and xcreat()
 */

# define xfopen(file, mode)	fdfopen(xopen(file, mode), mode)
# define xfcreat(file, mode)	fdfopen(xopen(file, mode), 1)

# define remove(file)		xunlink(file)

/*
 *	SAVE() and RSTR() use local data in nested blocks
 *	Make sure that they are nested cleanly!
 */

# define SAVE(name, place)	{ int place name;
# define RSTR(name, place)	name = place; }

/*
 *	Use: DEBUG(sum, d) which becomes fprintf(stderr, "sum = %d\n", sum)
 */

# define DEBUG(variable, type)	fprintf(stderr, "variable = %type\n", variable)

/*
 *	Use: SCCSID(%W%) which becomes static char Sccsid "%W%"
 */
# define SCCSID(arg)	static char Sccsid[] "arg"

/*
 *	Use of ERRABORT() will cause libS.a internal errors to cause aborts
 */

# define ERRABORT()		_error() { abort(); }

/*
 *	Use of USXALLOC() is required to force all calls to alloc()
 *	(e.g. from libS.a) to call xalloc().
 */

# define USXALLOC()	alloc(n){ return(xalloc(n)); } free(n) { xfree(n); }\
			malloc(n) { register p; p = xalloc(n); return(p != -1 ?\
			p : 0); }

# define NONBLANK(p)		while(*p == ' ' || *p == '\t') p++

/*
 *	A global null string.
 */
char Null[1];

/*
 *	A global error message string.
 */
char Error[128];
