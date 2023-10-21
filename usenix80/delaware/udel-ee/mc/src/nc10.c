#
/*

		MOTOROLA M6800 C COMPILER
		Pass 2


	All comments or questions should be directed to:

	Dave Sincoskie
	Department of Electrical Engineering
	University of Delaware
	Newark, Delaware 19711
	(302) 738-8005


	Version history:

	Version 1.0	8/25/78




	The C Compiler for the Motorola M6800 is essentially a rewrite of the
	code generation parts of the version 6 C compiler for the PDP-11.  When
	this project was begun, no documentation, except for the source code,
	was available for the C compiler.  The algorithm used to generate code
	for the 6800 is from "Compiler Design for Digital Computers" by Gries.

	The expression optimizer has been left essentially unchanged, excepting
	the removal of PDP-11 dependencies.  The code generator is entirely
	new, and does not interact with the optimizing routines.


*/

#include "nc1h.c"

char	maprel[] {	EQUAL, NEQUAL, GREATEQ, GREAT, LESSEQ,
			LESS, GREATQP, GREATP, LESSEQP, LESSP
};

char	notrel[] {	NEQUAL, EQUAL, GREAT, GREATEQ, LESS,
			LESSEQ, GREATP, GREATQP, LESSP, LESSEQP
};

struct tconst czero { CON, INT, 0, 0};
struct tconst cone  { CON, INT, 0, 1};
struct tconst fczero { SFCON, DOUBLE, 0, 0 };

struct	table	*cregtab;

int	nreg	3;
int	isn	10000;
int	namsiz	8;

main(argc, argv)
char *argv[];
{
	extern fout;
	int out;

	out = modused = multused = divused = 0;

/*	for (xxx=0; xxx<argc; xxx++) diag("%s\n",argv[xxx]);*/

	if (argc<4) {
		error("Arg count");
		exit(1);
	}

	if ((fout = creat(argv[3], 0666)) < 0) {
		error("Can't create %s", argv[3]);
		exit(1);
	}
	spacep = treespace;

	/*
	 * tack on the string file.
	 */
	if (fopen(argv[2], ascbuf)<0) {
		error("Missing temp file");
		exit(1);
	}
	getree();
	close(ascbuf[0]);


	if(fopen(argv[1], ascbuf)<0) {
		error("Missing temp file");
		exit(1);
	}
	tmplabel = 20000;
	depth = 0;
	xvalid = 0;
	getree();
	flush();
	/*
	 * If any floating-point instructions
	 * were used, generate a reference which
	 * pulls in the floating-point part of printf.
	 */
	if (nfloat)
		{
		error("floating point not allowed");
		exit(1);
		}
	out = (nerror != 0);
	if (modused)
		out =| 040;
	if (multused)
		out =| 0200;
	if (divused)
		out =| 0100;
	exit(out);
}


/*
	This is the routine that "walks" the expression trees after they have
	been constructed from the intermediate code and optimized.  It is
	simply an implementation of the algorithm for code generation in
	accumulator based machines discussed by Gries.
	The routine will cause code to be generated which will leave the value
	of the expression in the accumulator.  No optimization is done to take
	care of "dead" accumulator values, and type conversion is rather
	primitive.  However, these things are picked up by the next pass.
	The "gen" routine is called to do the actual generation of code for
	an operator with various operands.

 */
ccode(tree)
struct tnode *tree;
{
    int oldtype, temptype;

  loop:
    if (tree -> op == 0) return;  /* NULL operator  */

    if (tree->op == INIT)	/* INIT's are wierd */
	{
	if (tree->tr1->op == AMPER)
	    tree->tr1 = tree->tr1->tr1;
	gen(INIT,0,tree->tr1);
	return;
	}

    if (opdope[tree->op] & LEAF)
	{
	gen(LOADAC,tree->type,tree,tree->type,tree->type);
	acctype = tree->type;
	return;
	}

    if (tree->op == COMMA)  /* commas are wierd, so handle separately  */
	{
	ccode(tree->tr2);
	gen(PUSH,0,0,INT);
	depth =+ 2;
	ccode(tree->tr1);
	return;
	}

    if (opdope[tree->op] & BINARY)
	{
	/*  differentiate between *x= and =*x  */
	if ((opdope[tree->op] & ASSGOP) && (tree->tr1->op == STAR))
	    {
	    tree->tr1->op = LSTAR;
	    tree->tr1->type =| INDEX;
	    }

	if (tree->tr2->type & ACC)
	    {
	    if (tree->tr1->type & ACC)
		{
		error("code gen foul up");
		exit();
		}

	    if (opdope[tree->tr1->op] & LEAF)
		{
		gen(tree->op,tree->type,tree->tr1,tree->tr1->type,acctype);
		acctype = tree->type;
		return;
		}

	    error("code gen foul up");
	    exit();
	    }


	if (opdope[tree->tr2->op] & LEAF)
	    {
	    if (tree->tr1->type & ACC)
		{
		if ((opdope[tree->op] & COMMUTE) == 0)
		    {
		    oldtype = acctype;
		    if ((oldtype & INDEX) == 0)
			{
			oldtype =| STACK;
			gen(PUSH,0,0,0,acctype);
			}
		    gen(LOADAC,tree->tr2->type,tree->tr2,tree->tr2->type,0);
		    acctype = tree->tr2->type;
		    gen(tree->op,tree->type,tree->tr1,oldtype,acctype);
		    acctype = tree->type;
		    if ((oldtype & INDEX) == 0)
			gen(POP,0,0,0,oldtype);
		    return;
		    }

		gen(tree->op,tree->type,tree->tr2,tree->tr2->type,acctype);
		acctype = tree->type;
		return;
		}

	    if (opdope[tree->tr1->op] & LEAF)
		{
		gen(LOADAC,tree->tr2->type,tree->tr2,tree->tr2->type,0);
		acctype = tree->tr2->type;
		gen(tree->op,tree->type,tree->tr1,tree->tr1->type,acctype);
		acctype = tree->type;
		if (tree->op == COMMA)
		    {
		    gen(LOADAC,tree->tr1->type,tree->tr1,tree->tr1->type,0);
		    acctype = tree->tr1->type;
		    }

		return;
		}

	    ccode(tree->tr1);
	    if (opdope[tree->op] & COMMUTE)
		{
		gen(tree->op,tree->type,tree->tr2,tree->tr2->type,acctype);
		acctype = tree->type;
		return;
		}

	    oldtype = acctype;
	    if ((oldtype & INDEX) == 0)
		{
		oldtype =| STACK;
		gen(PUSH,0,0,0,acctype);
		}
	    gen(LOADAC,tree->tr2->type,tree->tr2,tree->tr2->type,0);
	    acctype = tree->tr2->type;
	    gen(tree->op,tree->type,tree->tr1,oldtype,acctype);
	    acctype = tree->type;
	    if ((oldtype & INDEX) == 0)
		gen(POP,0,0,0,oldtype);
	    return;
	    }


	if (tree->tr1->type & ACC)
	    {
	    oldtype = acctype;
	    if ((oldtype & INDEX) == 0)
		{
		oldtype =| STACK;
		gen(PUSH,0,0,0,acctype);
		}
	    ccode(tree->tr2);
	    gen(tree->op,tree->type,tree->tr1,oldtype,acctype);
	    acctype = tree->type;
	    if ((oldtype & INDEX) == 0)
		gen(POP,0,0,0,oldtype);
	    return;
	    }

	if (opdope[tree->tr1->op] & LEAF)
	    {
	    ccode(tree->tr2);
	    gen(tree->op,tree->type,tree->tr1,tree->tr1->type,acctype);
	    acctype = tree->type;
	    return;
	    }


	ccode(tree->tr1);
	tree->tr1->type =| ACC;
	goto loop;
	}



/*  unary operators  */

    if (tree->tr1->type & ACC)
	{
	gen(tree->op,tree->type,tree,0,acctype);
	acctype = tree->type;
	return;
	}

    if (opdope[tree->tr1->op] & LEAF)
	{
	if (tree->op == AMPER)
	    {
	    gen(LOADADD,tree->tr1->type,tree->tr1,tree->tr1->type,0);
	    acctype = tree->type;
	    return;
	    }

	gen(LOADAC,tree->tr1->type,tree->tr1,tree->tr1->type,0);
	acctype = tree->tr1->type;
	gen(tree->op,tree->type,tree,0,acctype);
	return;
	}

    ccode(tree->tr1);
    gen(tree->op,tree->type,tree,0,acctype);
    acctype = tree->type;
}
