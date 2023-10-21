/*--- proc.c -------------------------------------------------------------
Procedure, type, variable, and label parsing routines for the Pascal to C
translator.
3/25/87 Daniel Kegel (seismo!rochester!srs!dan)
--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "p2c.h"
#include "ktypes.h"	/* keyword type definitions */

#define SLEN 80	
typedef char sstr[SLEN+1];	/* short string */
#define PLEN 1024
typedef char pstr[PLEN+1];	/* long string */

/* pgroup is used in parseProcedure to store the procedure's parameters */
struct pgroup {
    sstr pclass;	/* VAR or empty */
    sstr ptype;		/* what type all these guys are */
    pstr params;	/* identifiers separated by commas and space */
};

boolean
isSectionKeyword(k)
register int k;
{
    return(k==T_CONST||k==T_TYPE||k==T_VAR||k==T_PROC||k==T_FUNC||k==T_BEGIN);
}

/*--- skipSpace ---------------------------------------------------------
Accepts and throws away space and comment tokens.
------------------------------------------------------------------------*/
void
skipSpace()
{
    do
	getTok();
    while (cTok.kind == T_SPACE || cTok.kind == T_COMMENT);
    if (cTok.kind == T_EOF) {
	printf("\n/***# EOF ***/\n");
	fflush(stdout);
	exit(1);
    }
}

/*--- parseSpace ---------------------------------------------------------
Accepts and prints space and comment tokens.
------------------------------------------------------------------------*/
void
parseSpace()
{
    do {
	getTok();
	if (cTok.kind == T_SPACE || cTok.kind == T_COMMENT)
	    fputs(cTok.str, stdout);
    } while (cTok.kind == T_SPACE || cTok.kind == T_COMMENT);
    if (cTok.kind == T_EOF) {
	printf("\n/***# EOF ***/\n");
	fflush(stdout);
	exit(1);
    }
}

void
expected(s)
char *s;
{
    printf("/***# Expected %s ***/", s);
    fflush(stdout);
}

/*---- expectThing -------------------------------------------------------
Makes sure current token is of desired type, else prints error message.
------------------------------------------------------------------------*/

void
expectThing(s, k)
char *s;
{
    if (cTok.kind != k)
	expected(s);
}

/*---- getThing -------------------------------------------------------
Gets next nonblank token, makes sure it is desired type, else prints error 
message.
------------------------------------------------------------------------*/
void
getThing(s, k)
char *s;
int k;
{
    skipSpace();
    expectThing(s, k);
}

/*---- parseVarDec ----------------------------------------------------
Translates one (possibly multi-)variable declaration.
Works for complex types, but can't be used to parse procedure parameters.
On entry, cTok is first token in identifier list.
On exit, cTok is the token after the type- probably T_SEMI.
Semicolon is translated, too.
----------------------------------------------------------------------*/

struct ident {			/* Used to save variable declaration body */
    char *str;			/* until type is known */
    int  kind;
};
#define MAXIDENTS 132		/* allows about 32 variables */

void
parseVarDec()
{
    void parseTypeDecl();		/* forward declaration */
    sstr indir, index;
    struct ident idents[MAXIDENTS];
    int i, n;

    /* Get identifiers, up to the colon that marks end of list */
    n=0;
    while (cTok.kind != T_COLON) {
	if (n == MAXIDENTS-1)
	    printf("/***# Variable declaration too long ***/");
	if (n == MAXIDENTS) n--;
	idents[n].str = MALLOC(char, strlen(cTok.str));
	strcpy(idents[n].str, cTok.str);
	idents[n++].kind = cTok.kind;
	if (cTok.kind != T_ZIP && cTok.kind != T_COMMA 
	&& cTok.kind != T_SPACE && cTok.kind != T_COMMENT)
	    expected(" (variable declaration) comma or identifier");
	getTok();		/* don't nuke spaces or comments */
    }

    /* Output any whitespace given before the type declaration */
    for (i=0; i<n&&(idents[i].kind==T_SPACE||idents[i].kind==T_COMMENT); i++){
	fputs(idents[i].str, stdout);
	free(idents[i].str);
    }

    /* Translate type specification */
    indir[0]=index[0]='\0';
    parseTypeDecl(indir, index);

    /* Output the identifiers, with appropriate modification for 
       ptr & array types */
    putchar(' ');		/* separate RECORD from first element...? */
    for (; i<n; i++) {
	if (idents[i].kind == T_ZIP && indir[0]!='\0')
	    fputs(indir, stdout);
	fputs(idents[i].str, stdout);
	if (idents[i].kind == T_ZIP && index[0]!='\0')
	    fputs(index, stdout);
	free(idents[i].str);
    }
    if (cTok.kind == T_SEMI)
	putchar(';');
}

/*---- parseProcedure -------------------------------------------------------
On entry, cTok is "PROCEDURE" or "FUNCTION".
On exit, cTok is the token after the semicolon after the function header.

Turns declarations like
    foo(a:int; b:int)
into
    foo(a,b)
    int a;
    int b;

Breaks up function declarations into 
    1. name
    2. parameter declarations
    3. type (or 'void', if procedure)
Breaks up parameter declarations into an array of pgroups.
----------------------------------------------------------------------------*/
void
parseProcedure()
{
    boolean isProcedure;
    boolean isForward;
    sstr fnName;
    sstr fnType;
    struct pgroup *pgps=NULL;
    int i, npgp=0;
    register struct pgroup *p;

    /* Remember whether is returns a value or not */
    isProcedure = (cTok.kind == T_PROC);
    /* Get function or procedure name, skipping space & comments */
    getThing("function name", T_ZIP);
    strcpy(fnName, cTok.str);
    skipSpace();			/* eat the function name */
    /* Get open paren (or semicolon of a parameterless procedure or fn) */
    if (cTok.kind == T_LPAREN) {
	do {
	    register char *cp;
	    /* Allocate and initialize another parameter group */
	    if (npgp++ == 0) pgps=MALLOC(struct pgroup, 1);
	    else pgps = REALLOC(pgps, struct pgroup, npgp);
	    p = pgps + npgp-1;
	    p->pclass[0] = p->ptype[0] = '\0';

	    /* Get optional class keyword */
	    skipSpace();		/* eat the paren or semicolon */
	    if (cTok.kind == T_VAR) {
		strcpy(p->pclass, cTok.str);
		skipSpace();		/* eat the class keyword */
	    }
	    /* Get identifier list & type */
	    cp = p->params;
	    /* Get identifiers, up to the colon that marks end of list */
	    while (cTok.kind != T_COLON) {
		register char *cq=cTok.str;
		if (cTok.kind != T_ZIP && cTok.kind != T_COMMA)
		    expected(" (variable declaration) comma or identifier");
		while (*cp++ = *cq++)
		    ;
		cp--;
		skipSpace();
	    }
	    *cp = 0;

	    /* Get type specifier, which may be many tokens.  Primitive. */
	    skipSpace();
	    p->ptype[0]=0;
	    do {
		strcat(p->ptype, cTok.str);
		skipSpace();
	    } while (cTok.kind != T_SEMI && cTok.kind != T_RPAREN);
	} while (cTok.kind == T_SEMI);
	expectThing(") at end of param list", T_RPAREN);
	skipSpace();
    }
    /* Get return type */
    if (isProcedure) {
	strcpy(fnType, "void");
    } else {
	expectThing(":", T_COLON);
	getThing("function type", T_ZIP);
	strcpy(fnType, cTok.str);
	skipSpace();
    }
    expectThing("semicolon", T_SEMI);
    /* Get optional FORWARD keyword */
    skipSpace();
    if (isForward = (cTok.kind == T_FORWARD)) {
	getThing(";", T_SEMI);
	skipSpace();
    }

    /* Output the first part of the translated function declaration */
    printf("%s %s(", fnType, fnName);
    for (i=0, p=pgps; i++ < npgp; p++) {
	fputs(p->params, stdout);
	if (i<npgp) putchar(',');
    }
    putchar(')');
    if (isForward)
	puts(";");
    else {
	/* Output second part */
	putchar('\n');
	for (i=0, p=pgps; i++ < npgp; p++) {
	    if (p->pclass[0])
		fputs(p->pclass, stdout);	/* already xlated */
	    printf("%s %s;\n", p->ptype, p->params);
	}
    }
}

/*--- convertArrayBound -----------------------------------------------------
Given the upper bound of a Pascal array, append the C array size specification
to the buffer tindex.
Lower bounds are ignored, 'cause it's safe to do so, and impossibly difficult
to handle.
----------------------------------------------------------------------------*/
void
convertArrayBound(s, tindex)
char *s, *tindex;
{
    sstr buf;
    int ubound;

    ubound = atoi(s);
    if (ubound == 0) {
	/* Probably symbolic */
	sprintf(buf, "[%s+1]", s);
    } else {
	if (ubound < 0)
	    expected("positive upper bound");
	sprintf(buf, "[%d]", ubound+1);
    }
    strcat(tindex, buf);
}

/*---- parseTypeDecl -------------------------------------------------------
Translates a type definition in place.  Appends indirection & array subscrips,
if any, to the buffers tindir and tindex.
Never translates the semicolon- that is done in parseType.

On entry, cTok is the token that made us expect to find a type
(e.g. the colon in a variable declaration, or the equals in a type declaration,
On exit, cTok is the token after the type, usually T_SEMI (but may be T_END 
in the last declaration in a RECORD).

Pascal (or at least, Turbo Pascal) doesn't allow constructions like
    a = ^array [0..10] of integer;
rather, it forces you to define the base type, too:
    b = array [0..10] of integer;
    a = ^b;
Thus any type definition can be unambiguously broken up into 2 parts:
    - the base type (which may be complex)
    - if pointer, how many levels of indirection
      else if array, how many indices the type has, with limits
-----------------------------------------------------------------------*/
void
parseTypeDecl(tindir, tindex)
char *tindir, *tindex;		/* buffer to put * or [n] in */
{
    skipSpace();		/* get initial token of type */

    switch (cTok.kind) {
    case T_DEREF:		/* pointer type */
	strcat(tindir, "*");
	parseTypeDecl(tindir, tindex);
	break;
    case T_LPAREN:		/* enumerated type */
	fputs("enum {", stdout);
	do {
	    parseSpace();
	    if (cTok.kind != T_RPAREN)
		fputs(cTok.str, stdout);
	} while (cTok.kind != T_RPAREN);
	getThing(";", T_SEMI);
	putchar('}');
	break;
    case T_ARRAY:		/* array type */
	getThing("[", T_LBRACKET);
	do {					/* Get all the dimensions */
	    getThing("lower bound", T_ZIP);	/* Ignore lower bound except */
	    if (cTok.str[0] == '-')		/* to make sure >= 0 */
		expected("non-negative lower bound");
	    getThing("..", T_RANGE);
	    getThing("upper bound", T_ZIP);
	    convertArrayBound(cTok.str, tindex);
	    skipSpace();
	} while (cTok.kind == T_COMMA);
	expectThing("]", T_RBRACKET);
	getThing("OF", T_OF);
	parseTypeDecl(tindir, tindex);
	break;
    case T_STRINGTYPE:		/* Turbo (& UCSD?) string type */
	printf("char");
	skipSpace();
	if (cTok.kind != T_LPAREN && cTok.kind != T_LBRACKET) 
	    expected("[ or ( after STRING");
	getThing("string length", T_ZIP);
	convertArrayBound(cTok.str, tindex);
	skipSpace();
	if (cTok.kind != T_RPAREN && cTok.kind != T_RBRACKET) 
	    expected("] or ) after STRING[");
	getThing(";", T_SEMI);
	break;
    case T_FILE:		/* file type - not supported in C */
	strcat(tindir, "*");
	printf("FILE /* OF ");	/* show what it's a file of in the comment */
	do {
	    skipSpace();
	    if (cTok.kind != T_COMMENT);	/* avoid nesting comments */
		fputs(cTok.str, stdout);
	} while (cTok.kind != T_SEMI);
	printf(" */ ");
	break;
    case T_RECORD:		/* struct definition */
	printf("struct {");
	parseSpace();		/* eat RECORD */
	do {
	    if (cTok.kind == T_CASE) {
		printf("/***# Sorry- variant records not supported\n\t");
		do {
		    if (cTok.kind != T_COMMENT)
			fputs(cTok.str, stdout);
		    getTok();
		} while (cTok.kind != T_END);
		printf(" ***/");
		break;
	    }
	    parseVarDec();
	    if (cTok.kind == T_SEMI)
		parseSpace();
	    else if (cTok.kind == T_END)
		putchar(';');		/* Pascal doesn't need ; but C does*/
	    else if (cTok.kind != T_CASE)
		expected("Either semicolon or END");
	} while (cTok.kind != T_END);
	parseSpace();		/* eat the END, get the semi */
	printf("}");
	break;
    case T_ZIP:			/* probably a type keyword like 'integer' */
	fputs(cTok.str, stdout);
	skipSpace();		/* eat the type, get the semi */
	break;
    default:			/* unexpected */
	expected("type");
    }
}

/*---- parseVar -------------------------------------------------------
Translates the VAR section of a program or procedure.

On entry, cTok is "VAR".
On exit, cTok is any section-starting keyword.
Turns declarations like
    foo : ^integer;
into
    int *foo;
----------------------------------------------------------------------------*/
void
parseVar()
{
    getTok();		/* eat the VAR */
    do {
	parseVarDec();
	if (cTok.kind == T_SEMI)
	    parseSpace();
    } while (!isSectionKeyword(cTok.kind));
}

/*---- parseType -----------------------------------------------------------
Translates the TYPE section of a program or procedure.
On entry, cTok is TYPE.
On exit, cTok is any section-starting keyword.

Turns declarations like
    foo = array [0..10, LO..HI] of integer;
    boo = record
	    x : foo;
	    y : ^foo
	  end;

into
    typedef integer foo[11][HI+1];
    typedef struct {
	foo x;
	foo *y;
    } boo;
---------------------------------------------------------------------------*/
void
parseType()
{
    parseSpace();
    do {
	sstr typ;
	sstr tindir, tindex;
	expectThing("type identifier", T_ZIP);
	strcpy(typ, cTok.str);
	parseSpace();
	expectThing("=", T_EQUALS);
	printf("typedef ");
	tindir[0]=tindex[0]=0;
	parseTypeDecl(tindir, tindex);
	expectThing(";", T_SEMI);
	printf(" %s%s%s;", tindir, typ, tindex);
	parseSpace();
    } while (!isSectionKeyword(cTok.kind));
}

/*---- parseLabel -------------------------------------------------------
On entry, cTok is "LABEL".
On exit, cTok is whatever follows the semicolon.

Turns declarations like
LABEL foo, goo;
into
/ * LABEL foo, goo; * /
----------------------------------------------------------------------------*/
void
parseLabel()
{
    skipSpace();		/* eat the LABEL */
    printf("/* LABEL ");
    /* Get identifiers, up to the semicolon that marks end of list */
    while (cTok.kind != T_SEMI) {
	if (cTok.kind != T_ZIP && cTok.kind != T_COMMA)
	    expected(" (label declaration) comma or identifier");
	fputs(cTok.str, stdout);
	skipSpace();
    }
    /* Get semicolon without wiping out trailing space */
    getTok();
    fputs("; */", stdout);
}
