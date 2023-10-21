/*	Copyright 1981 Gary Perlman	*/
/* No part of this program may be duplicated without the written
   permission of the author.
*/
%{
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <sgtty.h>
#include <signal.h>
#ifndef iscntrl
#define iscntrl(a) ((a)>0&&(a)<26)
#endif
#define OPERATOR     1
#define PARSERROR    1
#define	MAXVAR    1000 
#define	UNDEFINED   -99999999999.987654321
int	nvar = 0;
char	*varname[MAXVAR];
char	*exprptr;
int	printequation = 1;
char	*prompt = "CALC: ";
int	printprompt = 0;
char	*indent = "      ";
struct	exprnode
	{
	int	nodetype;
	int	operator;
	double	*value;
	struct	exprnode *left;
	struct	exprnode *right;
	} *expression, *variable[MAXVAR];
double	eval (), answer;
double	*constant;
char	*calloc ();
char	*getline ();
FILE	*outfile = stdout;
%}
%token	NUMBER
%token	VARIABLE
%nonassoc '#'
%right	'='
%left	'?' IF THEN
%left	':' ELSE
%left	'|'
%left	'&'
%nonassoc '!'
%nonassoc EQ NE GE LE '<' '>'
%left	'+' '-'
%left	'*' '/' '%'
%right	'^'
%nonassoc UMINUS ABS EXP LOG SQRT
%%
start:
	expr = { expression = (struct exprnode *) $1;};
expr :
	'('  expr ')' = { $$ = $2; }|
	VARIABLE '=' expr
		= { variable[$1] = (struct exprnode *) $3; $$ = $3; }|
	'#' expr = { constant = (double *) calloc (1, sizeof (double));
		if (constant == NULL)
			errorexit ("Out of storage space");
		*constant = eval ((struct exprnode *) $2);
		$$ = (int) node ((int) constant, NUMBER, NULL, NULL); }|
	expr '+' expr = { $$ = (int) node ('+', OPERATOR, $1, $3); }|
	expr '-' expr = { $$ = (int) node ('-', OPERATOR, $1, $3); }|
	expr '*' expr = { $$ = (int) node ('*', OPERATOR, $1, $3); }|
	expr '%' expr = { $$ = (int) node ('%', OPERATOR, $1, $3); }|
	expr '/' expr = { $$ = (int) node ('/', OPERATOR, $1, $3); }|
	expr '^' expr = { $$ = (int) node ('^', OPERATOR, $1, $3); }|
	'-' expr %prec UMINUS
		= { $$ = (int) node ('_', OPERATOR, NULL, $2); }|
	expr EQ expr = { $$ = (int) node (EQ, OPERATOR, $1, $3); }|
	expr NE expr = { $$ = (int) node (NE, OPERATOR, $1, $3); }|
	expr LE expr = { $$ = (int) node (LE, OPERATOR, $1, $3); }|
	expr '<' expr = { $$ = (int) node ('<', OPERATOR, $1, $3); }|
	expr GE expr = { $$ = (int) node (GE, OPERATOR, $1, $3); }|
	expr '>' expr = { $$ = (int) node ('>', OPERATOR, $1, $3); }|
	expr '&' expr = { $$ = (int) node ('&', OPERATOR, $1, $3); }|
	expr '|' expr = { $$ = (int) node ('|', OPERATOR, $1, $3); }|
	'!' expr      = { $$ = (int) node ('!', OPERATOR, NULL, $2); }|
	expr '?' expr ':' expr = { $$ = (int) node ('?', OPERATOR, $1,
		(int) node (':', OPERATOR, $3, $5)); }|
	IF expr THEN expr = { $$ = (int) node ('?', OPERATOR, $2, $4); }|
	expr ELSE expr = { $$ = (int) node (':', OPERATOR, $1, $3); }|
	LOG expr = { $$ = (int) node (LOG, OPERATOR, NULL, $2); }|
	EXP expr = { $$ = (int) node (EXP, OPERATOR, NULL, $2); }|
	ABS expr = { $$ = (int) node (ABS, OPERATOR, NULL, $2); }|
	SQRT expr = { $$ = (int) node (SQRT, OPERATOR, NULL, $2); }|
	VARIABLE = { $$ = (int) node ($1, VARIABLE, NULL, NULL); }|
	NUMBER = { $$ = (int) node ($1, NUMBER, NULL, NULL); };
%%
yylex ()
	{
	extern	int yylval;
	char	tmpvarname[BUFSIZ];
	int	i;
	while (isspace (*exprptr)) exprptr++;
	if (begins ("log", exprptr)) {exprptr += 3; return (LOG);}
	if (begins ("sqrt", exprptr)) {exprptr += 4; return (SQRT);}
	if (begins ("exp", exprptr)) {exprptr += 3; return (EXP);}
	if (begins ("abs", exprptr)) {exprptr += 3; return (ABS);}
	if (begins ("if", exprptr)) {exprptr += 2; return (IF);}
	if (begins ("then", exprptr)) {exprptr += 4; return (THEN);}
	if (begins ("else", exprptr)) {exprptr += 4; return (ELSE);}
	if (isalpha (*exprptr))
		{
		i = 0;
		while (isalnum (exprptr[i]))
			{
			tmpvarname[i] = exprptr[i];
			i++;
			}
		tmpvarname[i] = NULL;
		exprptr += i;
		i = 0;
		while (i < nvar && strcmp (tmpvarname, varname[i])) i++;
		if (i == nvar)
			{
			varname[i] = calloc (1, strlen (tmpvarname) + 1);
			if (varname[i] == NULL)
				errorexit ("Out of storage space");
			strcpy (varname[i], tmpvarname);
			if (++nvar == MAXVAR)
				errorexit ("Too many variables");
			}
		yylval = i;
		return (VARIABLE);
		}
	if (isdigit (*exprptr) || *exprptr == '.')
		{
		constant = (double *) calloc (1, sizeof (double));
		if (constant == NULL)
			errorexit ("Out of storage space");
		*constant = atof (exprptr);
		yylval = (int) constant;
	    /* now skip over the number */
		while (isdigit (*exprptr)) exprptr++;
		if (*exprptr == '.') exprptr++;
		while (isdigit (*exprptr)) exprptr++;
		if (*exprptr == 'E' || *exprptr == 'e')
			{
			exprptr++;
			if (*exprptr == '+' || *exprptr == '-') exprptr++;
			while (isdigit (*exprptr)) exprptr++;
			}
		return (NUMBER);
		}
	if (begins ("!=", exprptr)) { exprptr += 2; return (NE); }
	if (begins (">=", exprptr)) { exprptr += 2; return (GE); }
	if (begins ("<=", exprptr)) { exprptr += 2; return (LE); }
	if (begins ("==", exprptr)) { exprptr += 2; return (EQ); }
	return (*exprptr++);
	}
yyerror ()
	{
	fprintf (outfile, "Parsing error.  ");
	fprintf (outfile, "This is left in input: [%s]\n", exprptr-1);
	}
struct exprnode *
node (datum, datatype, lson, rson) struct exprnode *lson, *rson;
	{
	struct exprnode *newnode;
	newnode = (struct exprnode *) calloc (1, sizeof (struct exprnode));
	if (newnode == NULL)
		errorexit ("Out of storage space");
	newnode->nodetype = datatype;
	if (datatype == OPERATOR || datatype == VARIABLE)
		newnode->operator = datum;
	else	newnode->value = (double *) datum;
	newnode->left = lson;
	newnode->right = rson;
	return (newnode);
	}
main (argc, argv) int argc; char *argv[];
    {
    int	i;
    struct sgttyb tty;
    signal (SIGINT, SIG_IGN);
    if (gtty (fileno (stdin), &tty) == 0)
    	{
	printprompt = 1;
    	printf ("Welcome to CALC (version 1.1)\n");
    	printf ("Enter expressions after the prompt '%s', quit with ^D, get help with ?\n", prompt);
    	}
    for (i = 1; i < argc; i++) process (argv[i]);
    process (NULL);
    }

process (filename) char *filename;
    {
    char	exprline[BUFSIZ];
    FILE	*ioptr;

    if (filename == NULL || *filename == NULL) ioptr = stdin;
    else if ((ioptr = fopen (filename, "r")) == NULL)
	    {
	    fprintf (stderr, "Can't open %s\n", filename);
	    return;
	    }
    if (filename) fprintf (outfile, "Reading from %s\n", filename);
    for (;;)
	{
	if (ioptr == stdin && printprompt) fprintf (outfile, prompt);
	if (!getline (exprline, ioptr)) break;
	exprptr = exprline;
	while (isspace (*exprptr)) exprptr++;
	if (!*exprptr || *exprptr == '?')
		{
		printmenu ();
		continue;
		}
	if (iscntrl (*exprptr))
		{
		control (exprptr);
		continue;
		}
	if (yyparse() == PARSERROR) continue;
	if (printequation || ioptr != stdin) ptree (outfile, expression);
	if (zero (answer = eval (expression))) answer = 0.0;
	if (answer == UNDEFINED) fprintf (outfile, " = UNDEFINED\n");
	else fprintf (outfile, " = %g\n", answer);
	}
    fclose (ioptr);
    }

printmenu ()
	{
	printf ("The following CTRL characters have special functions:\n");
	printf ("(You may have to precede the character with a ^V)\n");
	printf ("^D	end of input to CALC\n");
	printf ("^P	toggles the printing of equations\n");
	printf ("^Rfile	reads the expressions from the named file\n");
	printf ("^Wfile	writes all expressions to the named file\n");
	printf ("	If no file is supplied, then print to the screen\n");
	}

control (key) char *key;
    {
    int	var;
    FILE	*saveoutfile;
    switch (*key)
	{
	case '': printequation = !printequation; return;
	case '': while (iscntrl (*key) || isspace (*key)) key++;
		process (key);
		return;
	case '':
	case '':
		while (*key && (iscntrl (*key) || isspace (*key)))
			key++;
		if (*key)
		    {
		    fprintf (outfile, "Writing to %s\n", key);
		    saveoutfile = outfile;
		    if ((outfile = fopen (key, "a")) == NULL)
			{
			fprintf (stderr, "Can't open %s\n", key);
			outfile = saveoutfile;
			}
		    }
		for (var = 0; var < nvar; var++)
		    {
		    fprintf (outfile, "%-10s = ", varname[var]);
		    if (outfile == stdout)
			{
			if (zero (answer = eval (variable[var])))
			    answer = 0.0;
			if (answer == UNDEFINED)
			    fprintf (outfile, " UNDEFINED = ");
			else
			    fprintf (outfile, "%10g = ", answer);
			}
		    ptree (outfile, variable[var]);
		    fprintf (outfile, "\n");
		    }
		    if (*key)
			  {
			  fclose (outfile);
			  outfile = saveoutfile;
			  }
		    return;
	default: fprintf (stderr, "Unknown control character.\n");
	}
    }

double
eval (expression) struct exprnode *expression;
    {
    double	tmp1, tmp2;
    if (expression == NULL) return (UNDEFINED);
    if (expression->nodetype == VARIABLE)
	    if (variable[expression->operator])
		    return (eval (variable[expression->operator]));
	    else	return (UNDEFINED);
    if (expression->nodetype == NUMBER)
	    return (*expression->value);
    if ((tmp2 = eval (expression->right)) == UNDEFINED) return (UNDEFINED);
    switch (expression->operator)
	{
	case '_': return (-tmp2);
	case LOG: if (tmp2 <= 0.0) return (UNDEFINED);
		else return (log (tmp2));
	case EXP: return (exp (tmp2));
	case ABS: return (fabs (tmp2));
	case SQRT: if (tmp2 < 0.0) return (UNDEFINED);
		else return (sqrt (tmp2));
	}
    if ((tmp1 = eval (expression->left)) == UNDEFINED) return (UNDEFINED);
    switch (expression->operator)
	{
	case '+': return (tmp1 + tmp2);
	case '-': return (tmp1 - tmp2);
	case '*': return (tmp1 * tmp2);
	case '%': if ((int) tmp2 == 0) return (tmp1);
		else return ((double) (((int) tmp1) % ((int) tmp2)));
	case '/': if (zero (tmp2)) return (UNDEFINED);
		else return (tmp1/tmp2);
	case '^': return (pow (tmp1, tmp2));
	case '&': return (!zero (tmp1) && !zero (tmp2));
	case '|': return (!zero (tmp1) || !zero (tmp2));
	case '!': return (zero (tmp1) ? 1.0 : 0.0);
	case '>': return (tmp1 > tmp2 ? 1.0 : 0.0);
	case '<': return (tmp1 < tmp2 ? 1.0 : 0.0);
	case EQ : return (zero (tmp1 - tmp2) ? 1.0 : 0.0);
	case NE : return (!zero (tmp1 - tmp2) ? 1.0 : 0.0);
	case LE : return (tmp1 <= tmp2 ? 1.0 : 0.0);
	case GE : return (tmp1 >= tmp2 ? 1.0 : 0.0);
	case ':': return (0.0); /* dummy return for ? */
	case '?':
		if (expression->right->operator == ':')
			return (!zero (tmp1)
				? eval (expression->right->left)
				: eval (expression->right->right));
		else if (!zero (tmp1)) return (eval (expression->right));
		else return (UNDEFINED);
	default: fprintf (stderr, "Unknown operator '%c' = %d\n",
		expression->operator, expression->operator);
		return (UNDEFINED);
	}
    }

ptree (ioptr, expression) struct exprnode *expression; FILE *ioptr;
    {
    if (expression == NULL)
	    return;
    if (expression->nodetype == VARIABLE)
	{
	fprintf (ioptr, "%s", varname[expression->operator]);
	return;
	}
    else if (expression->nodetype == NUMBER)
	{
	if (*expression->value == UNDEFINED)
		fprintf (ioptr, "UNDEFINED");
	else	fprintf (ioptr, "%g", *expression->value);
	return;
	}
    switch	(expression->operator)
	{
	case LOG: fprintf (ioptr, "log("); break;
	case ABS: fprintf (ioptr, "abs("); break;
	case EXP: fprintf (ioptr, "exp("); break;
	case SQRT: fprintf (ioptr, "sqrt("); break;
	case '_' : fprintf (ioptr, "-");
		ptree (ioptr, expression->right); return;
	case '?':
		fprintf (ioptr, "(if ");
		ptree (ioptr, expression->left);
		fprintf (ioptr, " then ");
		if (expression->right->operator == ':')
		    {
		    ptree (ioptr, expression->right->left);
		    fprintf (ioptr, " else ");
		    ptree (ioptr, expression->right->right);
		    }
		else ptree (ioptr, expression->right);
		fprintf (ioptr, ")");
		return;
	default: fprintf (ioptr, "(");
		ptree (ioptr, expression->left);
		switch (expression->operator)
		    {
		    case EQ: fprintf (ioptr, " == "); break;
		    case NE: fprintf (ioptr, " != "); break;
		    case GE: fprintf (ioptr, " >= "); break;
		    case LE: fprintf (ioptr, " <= "); break;
		    default: fprintf (ioptr, " %c ",expression->operator);
		    }
	}
    ptree (ioptr, expression->right);
    fprintf (ioptr, ")");
    }
begins (s1, s2) char *s1, *s2;
	{ while (*s1) if (*s1++ != *s2++) return (0); return (1); }
zero (x) double x; { return (fabs (x) < 10e-10); }
char *
getline (line, ioptr) char *line; FILE *ioptr;
	{
	register char *lptr = line;
	while ((*lptr = fgetc (ioptr)) != '\n' && *lptr != EOF) lptr++;
	if (*lptr != '\n') return (NULL);
	*lptr = NULL;
	return (line);
	}

errorexit (string) char *string;
	{
	fprintf (stderr, "%s\n", string);
	control ("calc.save");
	fprintf (stderr, "Current state saved in calc.save\n");
	exit (1);
	}
