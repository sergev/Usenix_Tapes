	/*
	** grammar for the input to the Redcode Assembler
	**
	** [cw by Peter Costantinidis, Jr. @ University of California at Davis]
	**
	static	char	rcsid[] = "$Header: rcasm.y,v 7.0 85/12/28 14:37:04 ccohesh Dist $";
	**
	*/

	/* instructions */
%{
#include <stdio.h>
#include "cw.h"

typedef	struct	/* simple single purpose symbol table */
{
	int	sym_abs;	/* absolute address of symbol */
	char	*sym_nam;	/* symbols name */
} symtab;

memword	inst;		/* current instruction */
memword	memory[JMAX_LEN];
symtab	symbols[MAX_LABELS];	/* symbol table */
symtab	*unres[JMAX_LEN];	/* list of unresolved references */
uns	opcode;		/* set in lexical analyzer */
int	icnt = 0;	/* instruction count */
int	lcnt = 1;	/* line count */
int	ocnt = 0;	/* # of operands for current instruction */
int	toggle = 0;	/* for multi-arg inst.'s indicates which arg */
int	orc;		/* required # of operands for current inst. */
char	*dlabel, *alabel, *blabel;/* set in lexical analyzer */

extern	void	yyerror();
%}


	/* operand tokens */
%token	OIMM	/* number is operand					(#) */
%token	OREL	/* number specifies offset from current instruction	( ) */
%token	OIND	/* number specifies offset from current instruction	(@) */
		/* to location where relative address of operand is found   */

%token	OPCODE
%token	INTEGER	/* the operands */
%token	COMMENT	/* a comment */
%token	ERROR	/* error in lex */
%token	NL	/* new line */

%token	DLABEL	/* defining a label */
%token	LABEL	/* a label being used */

%start	program

%%	/* beginning of rules section */

program	:	/* empty */
	|	program line
	|	program error NL
		{
			toggle = 0;
			lcnt++;
			yyerrok;
		}
	;

line	:	comment NL
		{
			lcnt++;
		}
	|	label statement comment NL
		{
			inst.op = $2;
			lcnt++;
			toggle = 0;	/* a hack for labels */
			if (icnt > JMAX_LEN)
			{
				static	int	once = FALSE;

				if (!once)
				{
					once = TRUE;
					fprintf(stderr, "Maximum instruction limit of %d exceeded\n", JMAX_LEN);
					fprintf(stderr, "Will continue processing...\n");
				}
			}
			movmem(&(memory[icnt++]), &inst);
		}
	;

label	:
	|	DLABEL
		{
			if (addsym(dlabel))
				YYERROR;
		}
	;

comment	:
	|	COMMENT
	;

statement:	OPCODE arga argb
		{
			if (orc != 2)
			{
				yyerror("bad argument count");
				YYERROR;
			}
			if ((opcode == MOV && $3 == IMM) ||
			    (opcode == ADD && $3 == IMM) ||
			    (opcode == SUB && $3 == IMM) ||
			    (opcode == JMZ && $3 == IMM) ||
			    (opcode == DJZ && $3 == IMM) ||
			    (opcode == MUL && $3 == IMM) ||
			    (opcode == DIV && $3 == IMM))
			{
				yyerror("non-sensicle mode for operand");
				YYERROR;
			}
			$$ = opcode;
		}
	|	OPCODE argb
		{
			if (orc != 1)
			{
				yyerror("bad argument count");
				YYERROR;
			}
			if ((opcode == JMP && $2 == IMM) ||
			    (opcode == RND && $2 == IMM) ||
			    (opcode == DAT && $2 != REL))
			{
				yyerror("non-sensicle mode for operand");
				YYERROR;
			}
			$$ = opcode;
		}
	;

arga	:	addr INTEGER	/* returns addr */
		{
			$$ = $1;
			inst.moda = $1;
			inst.arga = $2;
		}
	|	addr LABEL
		{
			$$ = $1;
			inst.moda = $1;
			inst.arga = getsymval(alabel, 0);
		}
	;

argb	:	addr INTEGER	/* returns addr */
		{
			$$ = $1;
			inst.modb = $1;
			inst.argb = $2;
		}
	|	addr LABEL
		{
			$$ = $1;
			inst.modb = $1;
			inst.argb = getsymval(blabel, 1);
		}
	;

addr:	/* no operand (relative addressing mode) */
		{	$$=REL;		}
	|	OIMM
		{	$$=IMM;		}
	|	OIND
		{	$$=IND;		}
	;

%%

char	*argv0;

extern	void	fatal(), init();

/*
** main() -	the main program
*/
void	main (argc, argv)
reg	int	argc;
reg	char	**argv;
{
	auto	char	buf[BUFSIZ];
	reg	char	*ptr;
	auto	char	*ifname, *ofname;
	auto	int	error = 0;

	argv0 = *argv++, argc--;
	ofname = RC_OUT;
	if (argc < 1)
		usage();
	ifname = *argv++, argc--;
	if (argc > 1)
		usage();
	if (argc)
		ofname = *argv;
	if (!freopen(ptr = ifname, "r", stdin) ||
	    !freopen(ptr = ofname, "w", stdout))
	{
		perror(ptr);
		exit(1);
	}
	init();
	error = yyparse();
	fclose(stdin);
	if (error)
		fprintf(stderr, "%s: yyparse() error\n", argv0);
	if (error || fini())
	{
		fclose(stdout);
		unlink(ofname);
		exit(1);
	}
	exit(0);
}

/*
** fini()	- resolve any unresolved symbols
**		- write instructions
*/
int	fini ()
{
	reg	int	i;
	reg	symtab	**us = unres;
	auto	int	error = FALSE;

	for (us = unres; *us; us++)
	{
		reg	symtab	*s;
		auto	int	inum = abs((*us)->sym_abs)-1;
		extern	symtab	*getsym();

		if (!(s = getsym((*us)->sym_nam)) || s->sym_abs < 0)
		{
			fprintf(stderr, "Inst. %d: label \"%s\" not defined\n",
				inum, (*us)->sym_nam);
			error++;
			continue;
		}
		/*
		fprintf(stderr, "Us inum: %d, \"%s\", %d\n",
			inum, (*us)->sym_nam, (*us)->sym_abs);
		*/
		if ((*us)->sym_abs < 0)
			memory[inum].argb = s->sym_abs - inum;
		else
			memory[inum].arga = s->sym_abs - inum;
	}
	if (error)
	{
		fprintf(stderr, "%d unresolved labels\n", error);
		fprintf(stderr, "%s: object file not created\n", argv0);
		return(TRUE);
	}
	for (i=0; i<icnt; i++)
		if (fwrite(&(memory[i]), sizeof(memory[i]), 1, stdout) != 1)
		{
			fprintf(stderr, "%s: write error to output file\n",
				argv0);
			perror(argv0);
			return(TRUE);
		}
	return(FALSE);
}

/*
** init()	- perform various initializations
*/
void	init ()
{
	reg	int	i;

	for (i=0; i<MAX_LABELS; i++)
		symbols[i].sym_nam = (char *) 0;
	for (i=0; i<JMAX_LEN; i++)
		unres[i] = (symtab *) NULL;
}

/*
** usage() -	print a usage message and exit
*/
void	usage ()
{
	fprintf(stderr, "Usage: %s infile [outfile]\n", argv0);
	exit(1);
}

/*
** yyerror() -	yacc error routine
*/
void	yyerror (msg)
reg	char	*msg;
{
	fprintf(stderr, "%s: error on line %d (%s)\n", argv0, lcnt, msg);
}

static	int	sym_cnt = 0;	/* number of symbols in table */
extern	symtab	*_addsym(), *addunres(), *getsym();

/*
** getsymval()	- return the value of the specified symbol relative to
**		  the current instruction
**		- if symbol is undefined:
**			add it to symtab w/o a value
**			mark this instruction as having an unresolved symbol
**			return 0
**		- if which==0, then arga, else argb
*/
int	getsymval (sname, which)
reg	char	*sname;
reg	int	which;
{
	reg	symtab	*s;
	auto	int	len;

	if (s = getsym(sname))
	{
		if (s->sym_abs >= 0)
			return(s->sym_abs - icnt);
	}
	else
		s = _addsym(sname);
	s = addunres(s->sym_nam);
	s->sym_abs += 1;		/* must be >0 */
	if (which)
		s->sym_abs *= -1;	/* negative for argb */
	return(0);
}

/*
** addunres()	- add a symbol to the unresolved list
*/
symtab	*addunres (sname)
reg	char	*sname;
{
	static	int	index = 0;
	reg	symtab	**s = &(unres[index++]);

	if (index >= JMAX_LEN)
		fatal("Symbols used to often");
	if (!(*s = (symtab *) malloc(sizeof(symtab))))
		fatal("memory allocation failure");
	(*s)->sym_nam = sname;
	(*s)->sym_abs = icnt;
	return(*s);
}

/*
** addsym()	- add a symbol to the symbols[] list
*/
int	addsym (sname)
reg	char	*sname;
{
	reg	symtab	*s;
	auto	int	len;

	if ((s = getsym(sname)) && s->sym_abs >= 0)
	{
		fprintf(stderr, "label \"%s\" multiply defined\n", sname);
		return(TRUE);
	}
	if (s)
	{	/* previously used symbol is declared */
		s->sym_abs = icnt;
		return(FALSE);
	}
	s = _addsym(sname);
	s->sym_abs = icnt;
	return(FALSE);
}

/*
** _addsym()	- slightly lower lever function, used by both addsym()
**		  and getsymval()
*/
symtab	*_addsym (sname)
reg	char	*sname;
{
	reg	symtab	*s = &(symbols[sym_cnt++]);
	reg	int	len;

	if (!(s->sym_nam = (char *) malloc(len=(strlen(sname)+1))))
	{
		fprintf(stderr, "%s: can't malloc %d bytes\n", argv0, len);
		fatal("memory allocation failure");
	}
	strcpy(s->sym_nam, sname);
	s->sym_abs = -1;
	return(s);
}

/*
** getsym()	- return a pointer to the specified symbol in the symbol table
*/
symtab	*getsym (sname)
reg	char	*sname;
{
	reg	int	i;

	for (i=0; i<sym_cnt; i++)
		if (!strcmp(sname, symbols[i].sym_nam))
			return(&(symbols[i]));
	return((symtab *) NULL);
}

/*
** fatal()	- print an error message and exit
*/
void	fatal (mesg)
reg	char	*mesg;
{
	fprintf(stderr, "%s: FATAL error: %s\n", argv0);
	exit(1);
}

#include "rcasm.h"	/* the lexical analyzer */
