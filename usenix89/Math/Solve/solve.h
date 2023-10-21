#include <stdio.h>
#include <math.h>
#include <curses.h>

#ifndef bool
typedef short bool;
#endif 

/*
 *  The "assert" macro causes solve to abort "gracefully" when
 *  the "asserts" argument is TRUE.
 */
#define assert(x) { if (!(x)) {endwin(); fprintf(stderr,"\nassertion in line %d file %s\n", __LINE__, __FILE__); abort();}}

#define nil(x) ((x) -511)


/* 
 *  Global Constants
 */

#define ERROR (0.0000001)	
#define ZERO_DIVIDE 7

#define EQUAL	(0)

#define ADD	((short) 1)
#define SUB 	((short) 2)
#define MULT	((short) 3)
#define DIV	((short) 4)
#define MOD	((short) 5)
#define POW	((short) 6)
#define MINUS	((short) 7)
#define CONST	((short) 8)
#define ID	((short) 9)

#ifndef TRUE
#define TRUE	((bool) 1)
#define true 	((bool) 1)
#endif

#ifndef FALSE
#define false	((bool) 0)
#define FALSE	((bool) 0)
#endif

/*
 *  This typedef defines each element in an
 *  expression tree.
 */
typedef struct expr {
	short op;
	union {
		char *name;
		struct {
			struct expr *left, *right;
		} exprs;
		double value;
	} u;
} EXPRESSION;
	
typedef struct var {
	char *name;		/* name of variable */
	int lock;

	struct {
		bool set;	/* TRUE if value set */
		double value;
	} assignment;

	struct {
		EXPRESSION *ptr;	/* not nil if assignment made it var */
	} expr;

	/*
	 *  List of all other equations we are refrenced in.
	 */
	struct {
		char **ptr;
	} reference;
} VARIABLE;


/*
 *  Global variables
 */
FILE *lexin, *lexout;
char *current_var;
int lineno;


/*
 *  Type all of the external functions so that they
 *  maybe used without type conflicts.
 */
void main(), setname(), assign();
int nameIndex();
double fmod(), solveForName(), solve(), solveExpr();
EXPRESSION *new();
int yywrap(), input(), output(), unput(), yyerror();
char *emalloc();
