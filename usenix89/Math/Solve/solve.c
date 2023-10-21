/*
 *	$Date: 86/04/01 19:11:29 $, Keith Rule
 *
 *	NAME
 *		solve.c -- solve equations 
 *	AUTHOR
 *		Keith Rule
 *		3750 SW 117th #38
 *		Beaverton, OR 97005
 *		tektronix!teklds!keithr
 *
 *	DESCRIPTION
 *		This program solves equations. It is intented to be
 *		a poor mans TK!SOLVER.
 *
 */

#include "solve.h"
#include <curses.h>

static char	rcs_id[] = "$Header: solve.c,v 1.4 86/04/01 19:11:29 keithr Exp $";
VARIABLE *vars[100];

/*
 *	EXTERNAL NAME
 *		main -- execution begins here.
 *
 *	SYNOPSIS
 */

void
main(argc, argv)
int argc;
char *argv[];

/*
 *	DESCRIPTION
 *		This is the main section of the program.
 *
 */
{
	int j, i, x, y, status;

	lexin = stdin;
	lexout = stdout;

	if (argc == 2) {
		if ((lexin = fopen(argv[1], "r")) == NULL) {
			fprintf(stderr, "can't open %s\n", argv[1]);
			exit(1);
		}
	} else
		exit(1);

	/*
	 *  Initialize  vars.
	 */
	for (i=0; i < 100; i++)
		vars[i] = nil(VARIABLE *);

	/*
	 *  parse the equations 
	 */
	yyparse();


	/*
	 *  Set up curses.
	 */
	initscr();
	noecho();
	crmode();

	/*
	 *  Paint the title info on the screen.
	 */
	move(0,0); printw("INPUT");
	move(0, 20); printw("NAME");
	move(0, 40); printw("OUTPUT");
	
	x=0; y = 2;

	/*
	 *  Execute commands until Quit is selected.
	 */
	for (;;) {
		int c;
		double value;
		char	test[100];

		for (i=0; vars[i] != nil(VARIABLE *); i++) {
			move(i+2, 20); printw("%s", vars[i]->name);
		}

		move(y, x); refresh();
		switch (c=getch()) {
		case 'u':		/* move cursor up */
			if (y > 2)
				y--;
			break;
		case 'd':		/* move cursor down */
			if (y < i+1)
				y++;
			break;
		case 'q':		/* quit */
			endwin();
			exit(0);
		case 'a':		/* add new entry */
			echo();
			move(y, 0); printw("             ");
			move(y, 40); printw("                    ");
			move(y, 0); refresh();
			if (scanw("%f", &value) != ERR) {
				move(y,x);
				printw("%f", value);
			} else  {
				move(y, 0);
				printw("ERROR     ");
			}
			noecho();
			move(y,0);
			refresh();
			vars[y-2]->assignment.set = TRUE;
			vars[y-2]->assignment.value = value;
			break;
		case 'D':		/* delete old entry */
			move(y,x); printw("            ");
			move(y,x); refresh();
			vars[y-2]->assignment.set = FALSE;
			break;
		case 'S':		/* solve equations */
			for (j=0; j < i; j++) {
				double answer;

				if (vars[j]->assignment.set) {
					move(j+2, 0);
					printw("%f", vars[j]->assignment.value);
					move(j+2, 40);
					printw("                   ");
				} else {
					move(j+2, 0);
					printw("        ");
					move(j+2, 40);
					printw("                   ");
					move(j+2, 40);
					answer = solve(vars[j]->name, &status);
					if (status == TRUE) 
						printw("%f", answer);
					else if (status == ZERO_DIVIDE)
						printw("Not Continuous");
					else
						printw("Need more info");
				}
			}
			move(y,x);
			break;
		}
	}
}



/*
 *	EXTERNAL NAME
 *		setname -- set variable "name" to value.
 *
 *	SYNOPSIS
 */

void
setname(name, value)
char *name;
double value;

/*
 *	DESCRIPTION
 *		This function sets variable "name" to value.
 */
{
	int i;

	i = nameIndex(name);
	assert(i >= 0);
	vars[i]->assignment.set = TRUE;
	vars[i]->assignment.value = value;
}

/*
 *	EXTERNAL NAME
 *		nameIndex -- returns index of "name" in vars[].
 *
 *	SYNOPSIS
 */


int
nameIndex(name)
char *name;

/*
 *	DESCRIPTION
 *		This function returns the index that the "name" 
 *		occurs in "vars[]". A -1 is returned if "name" is
 *		not found.
 */
{
	register int i;

	for (i=0; vars[i] != nil(VARIABLE *); i++)
		if (strcmp(name, vars[i]->name) == EQUAL)
			return i;
	return -1;
}


/*
 *	EXTERNAL NAME
 *		fmod -- returns the modulo value of x % y.
 *
 *	SYNOPSIS
 */

double
fmod(x, y)
double x, y;

/*
 *	DESCRIPTION
 *		This function returns the modulo value (remainder)
 *		of x in y a for floats.
 */
{
	return ((double) ((long) x) / ((long) y));
}

/*
 *	EXTERNAL NAME
 *		solveForName -- solve for var "name1" in "name2".
 *
 *	SYNOPSIS
 */

double
solveForName(name1, name2, status)
char *name1, *name2;
int *status;

/*
 *	DESCRIPTION
 *		This function finds a value for "name1" which
 *		satisfies "name2".
 */
{
	int nameIndex1, nameIndex2, internalStatus, assignSet = FALSE, j;
	double guess, new, name2Value;
	double tmp;

	nameIndex1 = nameIndex(name1);
	assert(nameIndex1 >= 0);
	nameIndex2 = nameIndex(name2);
	assert(nameIndex2 >=0);

	if (vars[nameIndex2]->assignment.set != TRUE) {
		if (vars[nameIndex2]->reference.ptr != nil(char **)) {
			for (j=0;  vars[nameIndex2]->reference.ptr[j] != nil(char *); j++) {
				guess = solveForName(vars[nameIndex2]->name, vars[nameIndex2]->reference.ptr[j], status);
				if (*status) {
					setname(name2, guess);
					assignSet = TRUE;
					break;
				}
			}
		}
		if (assignSet == FALSE) {
			*status = FALSE;
			return 0.0;
		}
	}

	new = 0.0;
	name2Value = vars[nameIndex2]->assignment.value;
	vars[nameIndex2]->assignment.set = FALSE;

	for (;;) {
		setname(name1, new);
		new = solve(name2, &internalStatus) - name2Value;
		if (internalStatus != ZERO_DIVIDE)
			break;
		new += 0.01;
	}

	if (internalStatus != TRUE) {
		vars[nameIndex1]->assignment.set = FALSE;
		vars[nameIndex2]->assignment.set = TRUE;
		*status = FALSE;
		return 0.0;
	}

	for (;;) {
		double a, b;

		guess = new;	
		setname(name1, guess);
		a = solve(name2, &internalStatus) - name2Value;
		if (fabs(a) < ERROR || !internalStatus)
			goto done;
		setname(name1, guess + .01);
		b = ((solve(name2, &internalStatus) - name2Value) - a) / (.01);
		if (fabs(b) < ERROR) {
			new = guess + 50;
		} else {
			if (!internalStatus)
				goto done;
			new = guess - a/b; 
		}
	}
		
done:
	vars[nameIndex1]->assignment.set = FALSE;
	if (assignSet)
		vars[nameIndex2]->assignment.set = FALSE;
	else
		vars[nameIndex2]->assignment.set = TRUE;
	*status = internalStatus;
	return new;
}

/*
 *	EXTERNAL NAME
 *		solve -- find value for "name".
 *
 *	SYNOPSIS
 */

double
solve(name, status)
char *name;
int *status;

/*
 *	DESCRIPTION
 *		This function tries to solve "name".
 */
{	
	int i, j;

	if ((i = nameIndex(name)) >= 0) {
		if (vars[i]->assignment.set == TRUE)
			return(vars[i]->assignment.value);
		if (vars[i]->expr.ptr != nil(EXPRESSION *)) {
			vars[i]->assignment.value = solveExpr(vars[i]->expr.ptr, status);
			if (*status == TRUE) {
				/* vars[i]->assignment.set = TRUE;*/
				return(vars[i]->assignment.value);
			}
		}
		if (vars[i]->reference.ptr != nil(char **)) {
			for (j=0;  vars[i]->reference.ptr[j] != nil(char *); j++) {
				vars[i]->assignment.value = solveForName(vars[i]->name, vars[i]->reference.ptr[j], status);
				if (*status) {
					/* vars[i]->assignment.set = TRUE; */
					return(vars[i]->assignment.value);
				}
			}
		}
	}
	*status = FALSE;
	return (0.0); 
}

/*
 *	EXTERNAL NAME
 *		solveExpr -- solve expression "e".
 *
 *	SYNOPSIS
 */

double 
solveExpr(e, status)
EXPRESSION *e;
int *status;

/*
 *	DESCRIPTION
 *		This function evaluates expression "e" and
 *		returns a double and status.
 */
{
	int i, j;
	double a, b;

	*status = TRUE;
	

	switch (e->op) {
	case ADD:
		a = solveExpr(e->u.exprs.left, status);
		if (*status) {
			b = solveExpr(e->u.exprs.right, status);
			if (*status)
				return (a+b);
		}
		*status = FALSE;
		return (0.0);
	case SUB:
		a = solveExpr(e->u.exprs.left, status);
		if (*status) {
			b = solveExpr(e->u.exprs.right, status);
			if (*status)
				return (a-b);
		}
		*status = FALSE;
		return (0.0);
	case MULT:
		a = solveExpr(e->u.exprs.left, status);
		if (*status) {
			b = solveExpr(e->u.exprs.right, status);
			if (*status)
				return (a*b);
		}
		*status = FALSE;
		return (0.0);
	case DIV:
		a = solveExpr(e->u.exprs.left, status);
		if (*status) {
			b = solveExpr(e->u.exprs.right, status);
			if (*status && b != 0.0)
				return (a/b);
			else if (*status && b == 0.0) {
				*status = ZERO_DIVIDE;
				return 0.0;
			}
		}
		*status = FALSE;
		return (0.0);
	case MOD:
		a = solveExpr(e->u.exprs.left, status);
		if (*status) {
			b = solveExpr(e->u.exprs.right, status);
			if (*status)
				return (fmod(a, b));
		}
		*status = FALSE;
		return (0.0);
	case POW:
		a = solveExpr(e->u.exprs.left, status);
		if (*status) {
			b = solveExpr(e->u.exprs.right, status);
			if (*status)
				return (pow(a, b));
		}
		*status = FALSE;
		return (0.0);
	case MINUS:
		return(-solveExpr(e->u.exprs.left, status));
	case CONST:
		return(e->u.value);
	case ID:
		return(solve(e->u.name, status));
	default:
		fprintf(stderr, "Internal error %d\n", e->op);
		exit(5);
	}
}

/*
 *	EXTERNAL NAME
 *		assign -- assign expression "e" to var "s".
 *
 *	SYNOPSIS
 */

void
assign(s, e)
char *s;
EXPRESSION *e;

/*
 *	DESCRIPTION
 *		This function assigns the expression "e" to
 *		the variable "s".
 */
{
	int i;
	
	for (i=0; vars[i] != nil(VARIABLE *) ; i++)
		if (strcmp(vars[i]->name, s) == EQUAL) {
			free(s);
			if (vars[i]->expr.ptr == nil(EXPRESSION *))
				vars[i]->expr.ptr = e;
			else {
				/* error */
			}
		} 
	
	if (vars[i] == nil(VARIABLE *)) {
		vars[i] = (VARIABLE *) emalloc(sizeof(VARIABLE));	
		vars[i]->name = s;
		vars[i]->expr.ptr = e;
		vars[i]->assignment.set = FALSE;
		vars[i]->assignment.value = 0.0;
		vars[i]->reference.ptr = nil(char **);
	}
}

/*
 *	EXTERNAL NAME
 *		new -- add new operation to an expression.
 *
 *	SYNOPSIS
 */

EXPRESSION *
new(op, e1, e2)
short op;
EXPRESSION *e1, *e2;

/*
 *	DESCRIPTION
 *		This function adds a new operation to an
 *		expression.
 */
{
	EXPRESSION *eptr;
	int i, j;
	char **tmp;

	eptr = (EXPRESSION *) emalloc(sizeof(EXPRESSION));
	eptr->op = op;

	switch (op) {
	case ADD:
	case SUB:
	case MULT:
	case DIV:
	case POW:
	case MOD:
		eptr->u.exprs.left = e1;
		eptr->u.exprs.right = e2;
		break;
	case MINUS:
		eptr->u.exprs.left = e1;
		break;
	case CONST:
		eptr->u.value = (double) *((double *) e1);
		break;
	case ID:
		eptr->u.name = (char *) e1;
		for (i=0; vars[i] != nil(VARIABLE *) ; i++)
			if (strcmp(vars[i]->name, e1) == EQUAL) {
				j=0;
				if (vars[i]->reference.ptr != nil(char **)) {
					for (; vars[i]->reference.ptr[j] !=  nil(char *); j++)
						if (strcmp(vars[i]->reference.ptr[j], current_var) == EQUAL)
							return eptr;
				} else {
					tmp = (char **) emalloc(sizeof(char *)*2);	
					tmp[j++] = current_var;
					tmp[j] = nil(char *);
					vars[i]->reference.ptr = tmp;
					return eptr;
				}

				tmp = (char **) emalloc(sizeof(char *)*(j+2));	
				for (j=0; vars[i]->reference.ptr[j] != nil(char  *); j++)
					tmp[j] = vars[i]->reference.ptr[j];
				tmp[j++] = current_var;
				tmp[j] = nil(char *);
				free(vars[i]->reference.ptr);
				vars[i]->reference.ptr = tmp;
				return eptr;
			} 
	
		if (vars[i] == nil(VARIABLE *)) {
			vars[i] = (VARIABLE *) emalloc(sizeof(VARIABLE));	
			vars[i]->name = (char *) emalloc(strlen(e1)+1);
			strcpy(vars[i]->name, e1);
			vars[i]->expr.ptr = nil(EXPRESSION *);
			vars[i]->assignment.set = FALSE;
			vars[i]->assignment.value = 0.0;
			vars[i]->reference.ptr = (char **) emalloc(sizeof(char *) *2);
			vars[i]->reference.ptr[0] = current_var;
			vars[i]->reference.ptr[1] = nil(char *);
		}
		break;
	default:
		fprintf(stderr,"\nEnternal error\n");
		exit(1);
	}
	return eptr;
}
	

/*
 *	EXTERNAL NAME
 *		yywrap -- required by lex for some reason (I forget).
 *
 *	SYNOPSIS
 */

int
yywrap()

/*
 *	DESCRIPTION
 *		<complete external description of the function>
 *
 */
{
	return 1;
}

/*
 *	EXTERNAL NAME
 *		input -- tells lex where to get input.
 *
 *	SYNOPSIS
 */

int
input() 

/*
 *	DESCRIPTION
 *		This function is used by lex. It gets the
 *		input for lex.
 */
{
	int c;

	if ((c=getc(lexin)) != EOF)
		return c;
	else
		return 0;
}

/*
 *	EXTERNAL NAME
 *		output -- outputs a character (for lex).
 *
 *	SYNOPSIS
 */

int
output(c)
char c;

/*
 *	DESCRIPTION
 *		<complete external description of the function>
 */
{
	putc(c, lexout);
}

/*
 *	EXTERNAL NAME
 *		unput -- puts back "c" into the input stream.
 *
 *	SYNOPSIS
 */

int
unput(c)
char c;

/*
 *	DESCRIPTION
 *		<complete external description of the function>
 */
{
	if (c != 0)
		ungetc(c, lexin);
	else
		ungetc(EOF, lexin);
}

/*
 *	EXTERNAL NAME
 *		yyerror -- handle yacc error messages.
 *
 *	SYNOPSIS
 */

int
yyerror(s)
char *s;

/*
 *	DESCRIPTION
 *		<complete external description of the function>
 */
{
	fprintf(stderr,"%s in line %d\n", s, lineno);
	exit(1);
}

/*
 *	EXTERNAL NAME
 *		emalloc -- error handling malloc.
 *
 *	SYNOPSIS
 */

char *
emalloc(n)
long n;

/*
 *	DESCRIPTION
 *		<complete external description of the function>
 */
{
	char *ptr;

	if ((ptr = (char *) malloc(n)) != (char *) 0)
		return ptr;
	fprintf(stderr,"out of core");
	exit(1);
}
