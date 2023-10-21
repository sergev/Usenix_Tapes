
/*
 * Gprolog 1.4/1.5
 *
 * Barry Brachman
 * Dept. of Computer Science
 * Univ. of British Columbia
 * Vancouver, B.C. V6T 1W5
 *
 * .. {ihnp4!alberta, uw-beaver}!ubc-vision!ubc-cs!brachman
 * brachman@cs.ubc.cdn
 * brachman%ubc.csnet@csnet-relay.arpa
 * brachman@ubc.csnet
 */

#include <usercore.h>

#define NCOLS	2		/* Defaults for list table */
#define COLW	40

#define MAXARGS		10	/* Maximum number of args to a CORE routine */

struct Core_info {
	char *Core_name;
	int (*Core_func)();
	char Core_arity;
	char Core_arg_type[MAXARGS];
};

struct Surface {
	char *surface_name;
	struct vwsurf *surface;
};

extern int (*oldbussignal)();
extern int (*oldsegvsignal)();

#define INT_ARG		0
#define FLOAT_ARG	1
#define CHAR_ARG	2
#define STRING_ARG	3
#define FLOAT_VEC_ARG	4
#define INT_VEC_ARG	5
#define ADDR_ARG	6
#define INT_PTR		7
#define FLOAT_PTR	8
#define INT_MAT_PTR	9
#define FLOAT_MAT_PTR	10
#define STRUCT_PTR	11
#define STRING_PTR	12
#define ADDR_PTR	13
