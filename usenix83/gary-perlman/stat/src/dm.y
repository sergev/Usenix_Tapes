/*
                  Copyright 1980  Gary Perlman.
Distribution of this program and/or any aupporting documentation
for material or personal gain is prohibited.  Copies may be freely
distributed provided this copyright notice is included.

No guarantee of performance in made or this program, but it has
been extensively checked.  Comments and/or complaints are welcome
and should be directed to:  G. Perlman, Psychology C009, UCSD,
La Jolla, CA 92093.
*/

/*yacc this file, then cc y.tab.c [-lm -lS -ly]
				A DATA MANIPULATOR
				  Gary Perlman
dm is a data manipulator designed to manipulate files of columns of number and
strings.  The major components of this program are:
	(1) a parser, built by yacc, called yyparse.
	(2) a scanner, yylex, called by yyparse.
	(3) a parse tree node function, called by yyparse.
	(4) a function, eval, that evaluates the parse trees.
	(5) a main that calls I/O routines and the control loop.
The following section is a bunch of global declarations that will be put
literally into the program, y.tab.c by yacc.
*/
%{
#include <stdio.h>			/* standard I/O package		      */
#include <ctype.h>			/* isspace ()			      */
double asin(), sqrt(), atof(), pow(), fabs(), ceil(), floor(), exp(), log();
/*****************************************************************************/
/*				CONSTANTS				     */
/*****************************************************************************/
#define MAXEXPR      100		/*maximum number of expressions      */
#define MAXSTRING    32			/*maximum length of input string     */
#define MAXCOL       100		/*maximum number of input columns    */
#define MAXCONST     100		/*maximum number of constants        */
#define LOGe10       2.302585092994	/*used for L function                */
#define FLOATPTR     0			/*codes for parse tree node types    */
#define OPERATOR     1
#define STRINGOP     2
#define STRINGPTR    3
typedef	int	boolean;		/* no boolean type in C		      */
#define FALSE        0
#define TRUE         1
#define PARSERROR    1                   /*returned by yyparse on error       */
/*
The following few numbers are reserved by dm to signal special conditions by
being returned by various routines.  They are hopefully numbers that no
expressions would ever evaluate to.
*/
#define	LARGE         9999999999999.0		/* a large number	      */
#define	SUPPRESS     -1125899906842624.0	/* suppress output	      */
#define	STRINGFLAG   -8888888888888777.0	/* returned by eval	      */
#define	NIL          -998888677484837274.	/* cause nil output	      */
#define	EXITFLAG     -99999999999999.9		/* cause exit		      */
char	outpipe = 0;				/* true if output is piped    */
FILE	*infile;				/* data read from here	      */
char	inputline[BUFSIZ];			/* INPUT read into here	      */
FILE	*outfile;				/* output from dm	      */
char	*evalstr[MAXCOL+1];			/* ptrs to strings from eval  */
char	str[MAXCOL+1][MAXSTRING];		/* columns from each dataline */
char	*expra;					/* ptr to each expression     */
struct enode					/* expression node in tree    */
	{
	int etype;				/* type of node		      */
	union	{
		char opr;			/* if operatr or stringop     */
		double *num;			/* if FLOATPTR		      */
		char *str;			/* if STRINGOP		      */
		} contents;
	struct enode *lchild;
	struct enode *rchild;
	} *expr[MAXEXPR+1];			/* ptr to each parse tree     */
double	input[MAXCOL+1];			/* input numbers	      */
boolean	usenumber[MAXCOL+1];			/* true is input[i] is used   */
int	usecols;				/* maximum col numbr accessed */
double	output[MAXEXPR+1];			/* output numbers	      */
double	const[MAXCONST];			/* constants stored here      */
double	nil = NIL;				/* flagged by NIL	      */
double	suppress  = SUPPRESS;			/* flagged by KILL	      */
double	stringflag = STRINGFLAG;		/* eval returns string	      */
double	exitflag = EXITFLAG;			/* flagged by EXIT	      */
double	randu;					/* uniform rand num	      */
double	maxrand;
double	N;					/* number of input cols	      */
double	sum;					/* sum of input cols	      */
int	nconst;					/* number of constants	      */
int	should_output[MAXEXPR+1];		/* used with X		      */
int	exprno;					/* expression number	      */
int	nexpr;					/* number of expressions      */
%}
/******************************************************************************/
/*                        G R A M M A R                                       */
/******************************************************************************/
/*
First is the set of precedences and associativities, followed by the grammar
Actions are associated with each part of the grammar matched.  These amount
to creating an enode representing the expression parsed, created by node.
One notable feature about the grammar is that it uses the same symbols for
comparing both strings and numbers, the difference being made explicit in
the actions for the string comparison functions.
*/
/******************************************************************************/
%token NUMBER				/* returnd by yylex for numbers	      */
%token STRING				/* returnd by yylex for strings	      */
%token STRINDEX				/* string index operator flag	      */
%left '?' ':' IF THEN ELSE		/* if then else			      */
%left '|' NOR				/* logical or			      */
%left '&' NAND				/* logical and			      */
%nonassoc '!'				/* logical not			      */
%nonassoc EQ NE GE GT LE LT NOTIN	/* comparators			      */
%left  '+' '-'				/* plus and minus		      */
%left  '*' '/' '%'			/* mult, div, mod		      */
%right '^'				/* exponentiation		      */
%nonassoc UMINUS			/* unary minus			      */
%nonassoc '#' 'l' 'L' 'e' 'a' 'f' 'c' 'v'/* unary functions		      */
%%					/* beginnig grammar rules section     */
/******************************************************************************/
/*                     P R O D U C T I O N S                                  */
/******************************************************************************/
start:
	stringexpr = { expr[exprno] = (struct enode *) $1;};
stringexpr :
	expr = { $$ = $1; }|
	string = { $$ = $1; };
expr :
	'x' '[' expr ']' { $$ = (int) node ('x', OPERATOR, NULL, $3); }|
	'y' '[' expr ']' { $$ = (int) node ('y', OPERATOR, NULL, $3); }|
	'('  expr ')' { $$ = $2; }|
	expr '+' expr { $$ = (int) node ('+', OPERATOR, $1, $3); }|
	expr '-' expr { $$ = (int) node ('-', OPERATOR, $1, $3); }|
	expr '*' expr { $$ = (int) node ('*', OPERATOR, $1, $3); }|
	expr '%' expr { $$ = (int) node ('%', OPERATOR, $1, $3); }|
	expr '/' expr { $$ = (int) node ('/', OPERATOR, $1, $3); }|
	expr '^' expr { $$ = (int) node ('^', OPERATOR, $1, $3); }|

	string EQ string { $$ = (int) node ('=', STRINGOP, $1, $3); }|
	string NE string { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('=', STRINGOP, $1, $3)); }|
	expr EQ expr { $$ = (int) node ('=', OPERATOR, $1, $3); }|
	expr NE expr { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('=', OPERATOR, $1, $3)); }|

	string GT string { $$ = (int) node ('>', STRINGOP, $1, $3); }|
	string GE string { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('<', STRINGOP, $1, $3)); }|
	expr GT expr { $$ = (int) node ('>', OPERATOR, $1, $3); }|
	expr GE expr { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('<', OPERATOR, $1, $3)); }|

	string LT string { $$ = (int) node ('<', STRINGOP, $1, $3); }|
	string LE string { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('>', STRINGOP, $1, $3)); }|
	expr LT expr { $$ = (int) node ('<', OPERATOR, $1, $3); }|
	expr LE expr { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('>', OPERATOR, $1, $3)); }|

	expr '&' expr { $$ = (int) node ('&', OPERATOR, $1, $3); }|
	expr NAND expr { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('&', OPERATOR, $1, $3)); }|
	expr '|' expr { $$ = (int) node ('|', OPERATOR, $1, $3); }|
	expr NOR expr { $$ = (int) node ('!', OPERATOR, NULL,
		(int) node ('|', OPERATOR, $1, $3)); }|

	'-' expr %prec UMINUS
		{ $$ = (int) node ('_', OPERATOR, NULL, $2); }|
	'!' expr { $$ = (int) node ('!', OPERATOR, NULL, $2); }|
	'l' expr { $$ = (int) node ('l', OPERATOR, NULL, $2); }|
	'L' expr { $$ = (int) node ('L', OPERATOR, NULL, $2); }|
	'v' expr { $$ = (int) node ('v', OPERATOR, NULL, $2); }|
	'e' expr { $$ = (int) node ('e', OPERATOR, NULL, $2); }|
	'a' expr { $$ = (int) node ('a', OPERATOR, NULL, $2); }|
	'f' expr { $$ = (int) node ('f', OPERATOR, NULL, $2); }|
	'c' expr { $$ = (int) node ('c', OPERATOR, NULL, $2); }|
	'#' string { $$ = (int) node ('#', STRINGOP, NULL, $2); }|
	string '[' expr ']' { $$ = (int) node ('[', STRINGOP, $1, $3); }|
	string 'C' string { $$ = (int) node ('C', STRINGOP, $1, $3); }|
	string NOTIN string { $$ = (int) node ('!', OPERATOR, NULL,
			(int) node ('C', STRINGOP, $1, $3)); }|
	expr '?' stringexpr ':' stringexpr { $$ = (int) node ('?', OPERATOR, $1,
		(int) node (':', OPERATOR,  $3, $5)); }|
	IF expr THEN stringexpr ELSE stringexpr 
	 { $$ = (int) node ('?',OPERATOR,$2, (int) node (':',OPERATOR,$4,$6)); }|
	NUMBER { $$ = (int) node ($1, FLOATPTR, NULL, NULL); };
string :
	STRING { $$ = (int) node ($1, STRINGPTR, NULL, NULL); };
%%
/******************************************************************************/
/*				YYLEX == SCANNER			      */
/******************************************************************************/
/*
Next is the scanner that will be repeatedly called by yyparse, yylex.
This simple program reads from a global char *expra, set by main.  Variables
are handled by returning NUMBER or STRING, tokens defined in above grammar.
*/
/******************************************************************************/
#define	CONST	(*expra=='.'||(*expra>='0'&&*expra<='9'))
yylex ()
	{
	extern	int yylval;
	char	*strsave ();
	int	column;

	while (isspace (*expra)) expra++;
	if (CONST)
		{
		const[nconst] = atof (expra);
		yylval = (int) &const[nconst++];
		while (CONST) expra++;
		return (NUMBER);
		}
	switch (*expra)
		{
		case '"':
		case '\'':
			yylval = (int) strsave ();
			return (STRING);
		case 's':
			column = atoi (++expra);
			if (column > usecols) usecols = column;
			yylval = (int) str[column];
			while (CONST) expra++;
			return (STRING);
		case 'x':
			if (expra[1] == '[')
			   {
			   expra++;
			   return ('x');
			   }
			column = atoi (++expra);
			if (column > usecols) usecols = column;
			usenumber[column] = TRUE;
			yylval = (int) &input[column];
			while (CONST) expra++;
			return (NUMBER);
		case 'y':
			if (expra[1] == '[')
			   {
			   expra++;
			   return ('y');
			   }
			column = atoi (++expra);
			if (column > usecols) usecols = column;
			yylval = (int) &output[column];
			while (CONST) expra++;
			return (NUMBER);
		case 'K':
			if (begins ("KILL", expra)) expra += 4; else expra++;
			yylval = (int) &suppress;
			return (NUMBER);
		case 'E':
			if (begins ("EXIT", expra)) expra += 4; else expra++;
			yylval = (int) &exitflag;
			return (NUMBER);
		case 'N':
			if (begins ("NIL", expra))
				{
				expra += 3;
				yylval = (int) &nil;
				}
			else if (begins ("NEXT", expra))
				{
				expra += 4;
				yylval = (int) &suppress;
				}
			else
				{
				expra++;
				yylval = (int) &N;
				}
			return (NUMBER);
		case 'S':
			yylval = (int) &sum;
			if (begins ("SKIP", expra))
				{
				expra += 4;
				yylval = (int) &suppress;
				}
			else if (begins ("SUM", expra)) expra += 3;
			else expra++;
			return (NUMBER);
		case 'R':
			if (begins ("RAND", expra)) expra += 4; else expra++;
			yylval = (int) &randu;
			return (NUMBER);
		case 'I':
			if (begins ("INLINE", expra))
				{
				expra += 6;
				yylval = (int) input;
				return (NUMBER);
				}
			if (begins ("INPUT", expra)) expra += 5; else expra++;
			yylval = (int) inputline;
			return (STRING);
		case 'O':
			if (begins ("OUTLINE", expra))
				{
				expra += 7;
				yylval = (int) output;
				return (NUMBER);
				}
			break;
		case '=': if (expra[1] == '=') expra += 2;
			else expra++;
			return (EQ);
		case '<': if (expra[1] == '=')
			{
			expra += 2;
			return (LE);
			}
			expra++;
			return (LT);
		case '>': if (expra[1] == '=')
			{
			expra += 2;
			return (GE);
			}
			expra++;
			return (GT);
		case '!': switch (expra[1])
				{
				case '=': expra += 2; return (NE);
				case 'C': expra += 2; return (NOTIN);
				case '&': expra += 2; return (NAND);
				case '|': expra += 2; return (NOR);
				default: expra++; return ('!');
				}
		case '&': if (expra[1] == '&') expra++; break;
		case '|': if (expra[1] == '|') expra++; break;
		case 'i': if (begins ("if", expra))
				{expra += 2; return (IF);} break;
		case 't': if (begins ("then", expra))
				{expra += 4; return (THEN);} break;
		case 'e': if (begins ("else", expra))
				{expra += 4; return (ELSE);}
			  else if (begins ("exp", expra))
				{expra += 3; return ('e');}
			  break;
		case 'a': if (begins ("abs", expra))
				{expra += 3; return ('a');}
			  break;
		case 'f': if (begins ("floor", expra))
				{expra += 5; return ('f');}
			  break;
		case 'c': if (begins ("ceil", expra))
				{expra += 4; return ('c');}
			  break;
		case 'l': if (begins ("log", expra))
				{expra += 3; return ('l');}
			  break;
		case 'v': if (begins ("var", expra))
				{
				if (begins ("variance", expra))
					expra += 8;
				else expra += 3;
				return ('v');
				}
			    break;
		case 'L': if (begins ("Log", expra))
				{expra += 3; return ('L');}
			  break;
		} /* ends switch */
	expra++; return (*(expra-1));
	} /* ends yylex */
/******************************************************************************/
/*				YYERROR					      */
/******************************************************************************/
yyerror ()
	{
	fprintf (stderr,
		"Failure occured with this left in input: (%s)\n", expra-1);
	ptree (expr[exprno]); fprintf (stderr, "\n");
	}
/******************************************************************************/
/*                       S T R S A V E                                        */
/******************************************************************************/
char *
strsave ()
	{
	char	buf[BUFSIZ], *bptr = buf;
	char	*p;
	char	*malloc (), *strcpy ();
	char	quotechar = *expra++;
	while (*expra && *expra != quotechar)
		*bptr++ = *expra++;
	if (*expra == quotechar) expra++;
	*bptr = NULL;
	if ((p = malloc (strlen (buf)+1) ) == NULL)
	    eprintf ("dm: No space to store string in expr[%d]\n", exprno);
	return (strcpy (p, buf));
	}
/******************************************************************************/
/*				NODE CREATOR				      */
/******************************************************************************/
struct enode *
node (datum, dtype, lson, rson)
	int datum, dtype;
	struct enode *lson, *rson;
	{
	struct enode *newnode;
	newnode = (struct enode *) calloc (1, sizeof (struct enode));
	if (newnode == NULL)
		eprintf ( "dm: No space to store expressions\n");
	newnode->etype = dtype;
	switch (dtype)
		{
		case FLOATPTR: newnode->contents.num = (double *) datum; break;
		case STRINGPTR: newnode->contents.str = (char *) datum; break;
		case STRINGOP:
		case OPERATOR: newnode->contents.opr = datum; break;
		default: fprintf (stderr, "dm/enode: unknown data type.\n");
		}
	newnode->lchild = lson;
	newnode->rchild = rson;
	return (newnode);
	}/*ends node*/
/******************************************************************************/
/*				MAIN					      */
/******************************************************************************/
main (argc, argv) int argc; char *argv[];
	{
	initial (argc, argv);
	loop ();
	}

/******************************************************************************/
/*				INITIAL					      */
/******************************************************************************/
/*
	initial does the following:
		1) sets the random number generator with the time as seed.
		2) reads in expressions from file, user, or argv[i].
		3) parses expressions.
		4) opens input and output files.
*/
initial (argc, argv) int argc; char **argv;
	{
	boolean	interactive = FALSE;	/* if true, input in interactive mode */
	boolean	input_by_hand;		/* true if expressions input by hand  */
	char	exprline[BUFSIZ];	/* expressions read into here	      */
	FILE	*exprfile;		/* expressions read from here	      */
	FILE	*getfile ();		/* gets a file open		      */

	maxrand = sizeof (int) == 4 ? 2147483648.0 : 32768.0;
	srand ((int) time (0));
	argc--;
	if (argc) checkstdin (argv[0]);
	if (argc == 0)
		{
		interactive = TRUE;
		exprfile = getfile ("Expression file? ", "r");
		if (exprfile == NULL)
			{
			input_by_hand = TRUE;
			exprfile = stdin;
			printf ("Enter ONE expression per line.\n");
			printf ("End with an empty line.\n");
			}
		}
	else if (argv[1][0] == 'E') /*expra file flag */
		{
		if ((exprfile = fopen (&argv[1][1], "r")) == NULL)
			eprintf ( "dm: Can't open '%s'.\n", &argv[1][1]);
		}
	else exprfile = NULL;

	/* PARSE expressions */
	while (1)
		{
	readexpr:
		if (exprfile == NULL) /*read expras from argv[i]   */
			{
			if (++exprno > argc) break;
			expra = argv[exprno];
			}
		else /*read expras from exprfile  */
			{
			++exprno;
			if (input_by_hand) printf ("expression[%d]: ", exprno);
			if (getline (exprline, BUFSIZ, exprfile) <= 0) break;
			expra = exprline;
			}

		while (isspace (*expra)) expra++;
		if (*expra == 'X')
			{
			should_output[exprno] = FALSE;
			expra++;
			}
		else should_output[exprno] = TRUE;

		/* call parser */
		if (yyparse() == PARSERROR)
			{
			fprintf (stderr, "error in parsing expr[%d].\n",
				exprno--);
			if (input_by_hand) goto readexpr; else exit (1);
			}
		if (input_by_hand)
			{
			ptree (expr[exprno]);
			printf ("\n");
			}
		}
		nexpr = exprno - 1;
		if (nexpr == 0)
			{
			exprno = 0;
			fprintf (stderr, "dm: No expressions were read in\n");
			if (input_by_hand) goto readexpr;
			else exit (1);
			}

		/* OPEN I/O files */
		if (interactive)
			{
			if ((infile = getfile ("Input file? ", "r")) == NULL)
				infile = stdin;
			if ((outfile = getfile ("Output file or pipe? ", "w")) == NULL)
				outfile = stdout;
			}
		else
			{
			infile = stdin;
			outfile = stdout;
			}
	} /* ends initial */

/******************************************************************************/
/*				LOOP					      */
/******************************************************************************/
/*	loop runs the process on the input to produce the output	      */
loop ()
	{
	double	eval ();
	boolean	skip = FALSE;

	while (getinput () != EOF)
		{
		skip = FALSE;
		for (exprno = 1; exprno <= nexpr; exprno++)
			if ((output[exprno] = eval(expr[exprno])) == suppress)
				{skip = TRUE; break;}
			else if (output[exprno] == exitflag) exit (0);
		if (skip == TRUE) continue;
		*output += 1.0;
		for (exprno = 1; exprno <= nexpr; exprno++)
			if (should_output[exprno])
				{
				if (output[exprno] == stringflag)
				    fprintf (outfile, "%s",evalstr[exprno]);
				else if (output[exprno] == nil) continue;
				else fprintf (outfile,"%.6g",output[exprno]);
				if (exprno < nexpr) fputc ('\t', outfile);
				}
		fprintf (outfile, "\n");
		fflush (outfile);
		} /* ends getinput loop */
	if (outpipe) pclose (outfile);
	} /* ends loop */
/******************************************************************************/
/*				GETINPUT				      */
/******************************************************************************/
int
getinput ()
	{
	int ncols;
	register int col;

	if (getline (inputline, BUFSIZ, infile) == EOF) return (EOF);
	randu = rand()/maxrand;
	sum = 0.0;
	*input += 1.0;
	for (col = 1; col <= usecols; col++)
		str[col][0] = NULL;
	ncols = sstrings (inputline, str[1], MAXCOL, MAXSTRING);
	for (col = 1; col <= ncols; col++)
		if (number (str[col]))
			sum += input[col] = atof(str[col]);
		else
		if (usenumber[col])
		    eprintf ( "dm: number expected in column %d on line %.0f\n",
			col, *input);
	N = ncols;
	return (ncols);
	} /* ends getinput */

/******************************************************************************/
/*				EVAL					      */
/******************************************************************************/
/*
eval is a recursive function that takes a parse tree of an expression,
and returns its value.  The major cluge in this program is how it handles
strings.  Since it wants to return a double, it cannot return a string, so
the use of strings is somewhat restricted.  When eval evals to a string, it
returns STRINGFLAG after setting a global char *evalstr[exprno] to the str
MAIN will look for this flag and switch its output to evalstr[exprno] rather
than output[exprno].
*/
double
eval (expression) struct enode *expression;
	{
	int	comp;				/*for string comparisons*/
	int	index, character;		/*for STRINDEX function */
	char	*string_2b_indexed;		/*for STRINDEX function */
	double	tmp1, tmp2;
	char	operator;
	boolean	fzero ();			/* true if arg == 0.0	*/

	if (expression == NULL) return (0.0);
	if (expression->etype == FLOATPTR)
		return (*expression->contents.num);
	if (expression->etype == STRINGPTR) 
		{
		evalstr[exprno] = expression->contents.str;
		return (stringflag);
		}
	if (expression->etype == STRINGOP)
		{
		switch (expression->contents.opr)
			{
			case '=': /*string compare*/
			     return (!strcmp (expression->lchild->contents.str,
					expression->rchild->contents.str));
			case '>': /*string compare*/
				comp = strcmp (expression->lchild->contents.str,
					expression->rchild->contents.str);
				if (comp > 0) return (1.0);
				return (0.0);
			case '<': /*string compare*/
				comp = strcmp (expression->lchild->contents.str,
					expression->rchild->contents.str);
				if (comp < 0) return (1.0);
				return (0.0);
			case 'C': /*true is s1 is in s2 */
			      return ( substr (expression->lchild->contents.str,
					expression->rchild->contents.str));
			case '#':
			    return (strlen (expression->rchild->contents.str));
			case '[': /* string index */
			   index = eval (expression->rchild);
			   string_2b_indexed = expression->lchild->contents.str;
			   character = string_2b_indexed[index-1];
			   return (1.0 * character);
			}
		}
	operator = expression->contents.opr;
	if (operator == ':') return (0.0); /*dummy for conditional */
	tmp1 = eval (expression->lchild);
	tmp2 = eval (expression->rchild);
switch (operator)
	{
	case 'x':
		index = (int) tmp2;
		if (index < 0 || index > N)
		    eprintf ("dm: computed index for x is out of range\n");
		if (usenumber[index]) return (input[index]);
		if (number (str[index])) return (atof (str[index]));
		eprintf ("dm: number expected in column %d of line %.0f\n",
			index, *input);
    case 'y':
	    index = (int) tmp2;
	    if (index >= 0 && index <= nexpr)
	    return (output[index]);
	    eprintf ( "dm: computed index for y is  out of range\n");
    case '_': return (-tmp2);
    case '!': return (fzero (tmp2) ? 1.0 : 0.0);
    case 'v': if (tmp2 < 0.0 || tmp2 > 1.0)
       eprintf ("dm/eval: asin(sqrt) undefined for %f on line %.0f, expr[%d]\n",
	    tmp2, input[0],exprno);
       return (asin (sqrt (tmp2)));
    case 'l': if (tmp2 <= 0.0)
	   eprintf ("dm/eval: log undefined for %f on line %.0f, expr[%d]\n",
		    tmp2, input[0],exprno);
	   return (log (tmp2));
    case 'L': if (tmp2 <= 0.0)
	   eprintf("dm/eval: Log undefined for %f Input line %.0f, expr[%d]\n",
		    tmp2,input[0],exprno);
	   return (log (tmp2) / LOGe10);
    case 'e': return (exp (tmp2));
    case 'a': return (fabs (tmp2));
    case 'c': return (ceil (tmp2));
    case 'f': return (floor (tmp2));
    case '+': return (tmp1 + tmp2);
    case '-': return (tmp1 - tmp2);
    case '*': return (tmp1 * tmp2);
    case '%': if (fzero (tmp2))
	   eprintf ("dm/eval: division by zero. Input line %.0f, expr[%d]\n",
		    input[0],exprno);
	   return ((double) (((int) tmp1) % ((int) tmp2)));
    case '/': if (fzero (tmp2))
	   eprintf ("dm/eval: division by zero. Input line %.0f, expr[%d]\n",
		    input[0],exprno);
	   return (tmp1/tmp2);
    case '^':
	    if (tmp1 < 0.0 && (floor (tmp2) != tmp2))
		eprintf ("dm: power failure at line %.0f\n",*input);
	    return (pow (tmp1, tmp2));
    case '>': return (tmp1 > tmp2 ? 1.0 : 0.0);
    case '<': return (tmp1 < tmp2 ? 1.0 : 0.0);
    case '=': return (fzero (tmp1 - tmp2) ? 1.0 : 0.0);
    case '&': return ((!fzero (tmp1) && !fzero (tmp2)) ? 1.0 : 0.0);
    case '|': return ((!fzero (tmp1) || !fzero (tmp2)) ? 1.0 : 0.0);
    case '?': if (!fzero (tmp1))
		    return (eval (expression->rchild->lchild));
		    return (eval (expression->rchild->rchild));
    default:
	    eprintf ("dm/eval: unknown operator '%c'\n", operator);
    }/*ends switch*/
	}/*ends eval*/
boolean
fzero (floatest) double floatest; { return (fabs (floatest) < 8e-10); }

/******************************************************************************/
/*				PTREE					      */
/******************************************************************************/
ptree (tree) struct enode *tree;
	{
	if (tree == NULL) return;
	if (tree->etype == FLOATPTR)
		if (*tree->contents.num < -LARGE)
			printf ("CONTROL");
		else printf ("%.2f", *tree->contents.num);
	else if (tree->etype == STRINGPTR)
		printf ("'%s'", tree->contents.str);
	else /* operator */
		{
		printf ("(");
		ptree (tree->lchild);
		printf (" %c ",tree->contents.opr);
		ptree (tree->rchild);
		printf (")");
		}
	} /* ends ptree */

/******************************************************************************/
/*				GETFILE					      */
/******************************************************************************/
FILE *
getfile (prompt, mode) char *prompt, *mode;
	{
	FILE	*popen (), *fopen (), *ioptr;
	char	filename[BUFSIZ];
	char	*ptr = filename;
    newfile:
	printf ("%s", prompt);
	if (getline (filename, MAXSTRING, stdin) <= 0) return (NULL);
	while (isspace (*ptr)) ptr++;
	if (*ptr == NULL) return (NULL);
	if (*mode == 'w')
		if (*ptr == '|') outpipe = 1;
		else if (!canwrite (filename)) goto newfile;
	if (outpipe)
		{
		if ((ioptr = popen (ptr+1, "w")) == NULL)
			{
			printf ("Cannot creat pipe.\n");
			outpipe = 0;
			goto newfile;
			}
		}
	else if ((ioptr = fopen (filename, mode)) == NULL)
		{
		printf ("Cannot open '%s'.\n", filename);
		goto newfile;
		}
	return (ioptr);
	}

/******************************************************************************/
/*				GETLINE					      */
/******************************************************************************/
int
getline (string, size, ioptr) char *string; int size; FILE *ioptr;
	{
	register int len = 0;
	char	escape = '\\';
	while ((string[len] = fgetc (ioptr)) != EOF)
		{
		if (string[len] == '\n') break;
		else if (string[len] == escape)
			string[len] = fgetc (ioptr);
		if (++len == size) break;
		}
	string[len] = NULL;
	if (len == 0 && feof (ioptr)) return (EOF);
	return (len);
	} /*ends getline*/

/******************************************************************************/
/*			BEGINS and SUBSTR				      */
/******************************************************************************/
begins (s1, s2) char *s1, *s2;
	{
	while (*s1)
		if (*s1++ != *s2++) return (0);
	return (1);
	} /* ends begins */
substr (s1, s2) char *s1, *s2;
	{
	while (*s2)
	      if (begins (s1, s2)) return (1);
	      else s2++;
	return (0);
	} /* ends substring */
