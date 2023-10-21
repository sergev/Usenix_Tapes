%{
/* -- Make include dependency generator
 *
 * $File: depend.y  1.13  1985-03-29  14:06:29 $
 *
 *  Copyright (c) 1983  Kim Walden
 *      ENEA DATA, Sweden and    
 *      SYSLAB, Department of Computer Science,
 *      University of Stockholm, Sweden.
 *
 *	This software may be used and copied freely,
 *	provided that proper credit is given to the originator
 *	by including the above text in each copy.
 *
 *	Descr:	This program implements part of the algorithm described in
 *      	K. Walden, "Automatic Generation of Make Dependencies",
 *      	Softw. Practice & Exper., vol. 14, no. 6, June 1984 
 *
 * $Log:	depend.y,v $
 * 
 * Revision 1.13  1985-03-29  14:06:29  kim
 * New source format
 * 
 * Revision 1.12  1985-03-08  01:16:37  kim
 * Added support for parallell transfer rules.
 * Added -1 option to makedep and depend.
 * 
 * Revision 1.11  1985-02-25  19:40:21  kim
 * Path reduction moved to earlier stage
 * 
 * Revision 1.10  1985-02-19  11:19:59  kim
 * New default include rules
 * 
 * Revision 1.9  1985-02-18  14:23:55  kim
 * New error handling
 * 
 * Revision 1.8  1985-02-14  13:59:52  kim
 * Changed rules options to -r/-R
 * 
 * Revision 1.7  1985-02-14  01:53:19  kim
 * Converted to RCS format
 * 
 * Revision 1.4  1983-10-06  19:50:16  kim
 * Allow empty include line input
 * 
 * Revision 1.3  1983-07-20  19:50:00  kim
 * Closer to algorithm in SYSLAB Report No. 19 (June 1983)
 * 
 * Revision 1.2  1983-02-22  19:49:51  kim
 * First version with include rules
 * 
 * Revision 1.1  1982-12-15  19:49:47  kim
 * Initial revision
 */
%}

%token SPACE RULSEP INCSEP PATH SUFF
%{
# define PREF    struct pref
# define SUF     struct suf
# define INCFILE struct incfile
# define LHSFILE struct lhsfile
# define NAME    struct filename
# define OUTSIZ  10
# define PATHSIZ 100
# define DIRSIZ  14
# define LINSIZ  256
# define PHASHL  503
# define LINE	 80
# define INCLUDED  1
# define CNULL '\0'
# define KOL ':'
# define EQU '='
# define RULES	 ".o: .c .p .f; .c= .y .l .q; .h= .y"

# include <stdio.h>
# include <strings.h>

static char usage[]= "Usage: depend [-1] [-rfile] [-Rrules]";

/* File name prefixes
 */
PREF
{	
	PREF	*	nexthash;
	char		path[PATHSIZ+1];
};

/* File name suffixes:
 *
 * If the suffix occurs in the rhs of a rule,
 * the output file suffix and rule type
 * (KOL or EQU) is recorded.
 */
SUF
{
	SUF	*	next,
		*	out[OUTSIZ];
	char		name[DIRSIZ+1];
	int		top,
			rule;
};

/* List of files included by a given file.
 */
INCFILE
{
	INCFILE	*	next;
	PREF	*	pf;
	SUF	*	sf;
};

/* A node for each file found in the lhs of some include statement.
 * The transfer rules are successively applied to the suffix
 * before the file is inserted.
 * Reference to list of included files.
 */
LHSFILE
{
	LHSFILE	*	next;
	PREF	*	pf;
	SUF	*	sf;
	INCFILE	*	ic;
	int		state;
};

NAME
{
	PREF	*	pf;
	SUF	*	sf;
};

struct
{
	SUF	*	sf;
	LHSFILE *	lf;
} first;


extern char	*	calloc();
void			yyerror();

static NAME		getname(),
			lhs,
			rhs;
static LHSFILE	*	nodelf(),
		*	thislf;
static PREF	*	nodepf(),
		*	phash[PHASHL];
static SUF	*	nodesf(),
		*	lsuf,
		*	rsuf;
static char	*	alloc(),
		*	rulstring,
		*	yystring;
static char		line[LINSIZ];
static void		printdep(),
			copyinc(),
			nodeic();
static int		hash(),
			lineno,
			rule,
			onedep;

# include "depscan.c"

main(argc, argv)
	int		argc;
	char	*	argv[];
{
	char		rulbuf[LINSIZ];

	setprogname(argv[0]);
	rulstring = rulbuf;
	strcpy(rulbuf, RULES);

	while (argc > 1 && argv[1][0] == '-')
	{
		switch (argv[1][1])
		{
		 case 'r':
			strcat(rulbuf, "; ");
			strcat(rulbuf, argv[1]+2);
			break;

		 case 'R':
			rulstring = argv[1]+2;
			break;

		 case '1':
			onedep++;
			break;

		 default:
			fatal("unknown option %s", argv[1]);
		}

		argv++;
		argc--;
	}

	if (argc != 1)
		fatalmesg(usage);

	yystring = rulstring;
	BEGIN rules;

	first.lf = NULL;
	yyparse();
	if (first.lf)
		printdep();
	exit(0);
}
%}

%%

accept	:	rules includes
	;
rules	:	rule
	|	RULSEP rules
	|	rules RULSEP
	|	rules RULSEP rule
	;
rule	:	':' SPACE
			{   /* Lhs of inclusion rule, empty output suffix */
				lsuf = nodesf("");
				rule = KOL;
			}
		rhssuf
	|	lhssuf ':' SPACE
			{   /* Lhs of inclusion rule */
				rule = KOL;
			}
		rhssuf
	|	lhssuf '=' SPACE
			{   /* Lhs of transfer rule */
				rule = EQU;
			}
		rhssuf
	|	rule SPACE rhssuf
	;
lhssuf	:	SUFF
			{   /* Lhs of any rule */
				lsuf = nodesf(yytext);
			}
	;
rhssuf	:	SUFF
			{   /* Rhs of any rule */
				rsuf = nodesf(yytext);
				if (rsuf->rule && (rsuf->rule != EQU || rule != EQU))
				        fatal("ambiguous rules %s", rulstring);
				rsuf->rule = rule;
				if (rsuf->top >= OUTSIZ)
					fatal("to many parallel transfers %s", rulstring);
				rsuf->out[rsuf->top++] = lsuf;
			}
	;
includes:
	|	includes INCSEP
	|	includes INCSEP include
	;
include	:	PATH
			{   /* Lhs of include statement:
			     *      Insert in lhsfile list.
			     */
				lhs = getname(yytext);
				thislf = nodelf(lhs.pf, lhs.sf);
			}
		SPACE '<' SPACE
		PATH
			{   /* Rhs of include statement:
			     *      Append to include list of lhsfile.
			     */
				rhs = getname(yytext);
				nodeic(thislf, rhs.pf, rhs.sf);
			}
	;

%%

void
yyerror(s)
	char	*	s;
{
	if (yystring)
		fatal("%s rules: %s", s, rulstring);
	else
		fatal("%s near line %d: %s", s, lineno, line);
}

/* Go through lhsfile list and print implied Make dependencies.
 */
static void
printdep()
{
	LHSFILE	*	lf,
		*	lf2;
	SUF	*	sf;
	INCFILE	*	ic;
	int		i;

    /* Apply transfer rules to the LHS suffixes.
     * If there are more than one output suffix,
     * make a new LHS node with a copy of the include list for each of them.
     *
     * Since the new nodes are inserted at the end of the LHS list,
     * they will be dealt with again, later in the for-loop.
     */
	for (lf = first.lf; lf; lf = lf->next)
		while (lf->sf->rule == EQU)
		{
			for (i = 1; i < lf->sf->top; i++)
				copyinc(lf, nodelf(lf->pf, lf->sf->out[i]));
			
			lf->sf = lf->sf->out[0];
		}

    /* Construct Transitive Closure.
     */
	for (lf = first.lf; lf; lf = lf->next)
		for (lf2 = first.lf; lf2; lf2 = lf2->next)
			for (ic = lf2->ic; ic; ic = ic->next)
				if (ic->pf == lf->pf && ic->sf == lf->sf)
				{
					lf->state = INCLUDED;
					copyinc(lf, lf2);
					break;
				}

    /* For each lhs file, apply inclusion rules
     * and generate Make dependencies.
     */
	for (lf = first.lf; lf; lf = lf->next)
	{
		char	lhsdep[PATHSIZ+1], rhsdep[PATHSIZ+1];
		int	brk = 0;

	    /* If the file was included in some other file,
	     * no separate output file will be generated for it.
	     * The corresponding dependencies have thus already
	     * been taken care of.
	     */
		if (lf->state == INCLUDED)
			continue;

		sf = lf->sf;

	    /* If the file was not included in any other file,
	     * and there is no inclusion rule for its suffix,
	     * the file is discarded.
	     */
		if (sf->rule != KOL)
			continue;

		sprintf(lhsdep, "%s%s", lf->pf->path, sf->out[0]->name); 
		sprintf(line, "%s:", lhsdep); 

		for (ic = lf->ic; ic; ic = ic->next)
		{
			sprintf(rhsdep, "%s%s", ic->pf->path, ic->sf->name);

			if (strlen(line)+strlen(rhsdep)+1 > LINE || brk)
			{
				printf("%s\n", line);
				sprintf(line, "%s:", lhsdep); 
			}
			strcat(line, " ");
			strcat(line, rhsdep);
			brk = onedep;
		}
		printf("%s\n", line);
	}
}

/* Copy entire include file list.
 *
 */
static void
copyinc(from, to)
	LHSFILE *	from,
		*	to;
{
	INCFILE	*	ic;

		for (ic = from->ic; ic; ic = ic->next)
			nodeic(to, ic->pf, ic->sf);
}

/* Return file name.  
 *
 */
static NAME
getname(path)
	char	*	path;
{
	NAME		nm;
	char	*	p;
	int		c;

	if (strlen(path) > PATHSIZ)
		fatal("path too long: %s", path);

	if ((p = rindex(path, '/')) == NULL)
		p = path;

	if ((p = rindex(p, '.')) == NULL)
		p = path + strlen(path);

	nm.sf = nodesf(p);
	c = *p;
	*p = CNULL;
	nm.pf = nodepf(path);
	*p = c;

	return nm;
}

/* Return suffix node.
 * If the given suffix was not in the suffix list,
 * allocate a new node.
 */
static SUF *
nodesf(name)
	char	*	name;
{
	SUF	*	node,
		*	prev;

	for (prev = node = first.sf; node; prev = node, node = node->next)
		if (strcmp(name, node->name) == 0)
			return node;

	node = (SUF *) alloc(sizeof(SUF), name);
	strcpy(node->name, name);

	if (prev)
		prev->next = node;
	else
		first.sf = node;

	return node;
}

/* Return prefix node.
 * If the given (reduced) path was not in the prefix list,
 * allocate a new node.
 */
static PREF *
nodepf(path)
	char	*	path;
{
	PREF	*	node,
		*	prevhash;
	int		h;

	h = hash(path, PHASHL);

	for (node = phash[h]; node; node = node->nexthash)
		if (strcmp(path, node->path) == 0)
			return node;
		else
			prevhash = node;

	node = (PREF *) alloc(sizeof(PREF), path);
	strcpy(node->path, path);

	if (phash[h])
		prevhash->nexthash = node;
	else
		phash[h] = node;

	return node;
}

/* Return lhsfile node.
 * If the given lhsfile was not in the lhsfile list,
 * allocate a new node.
 */
static LHSFILE *
nodelf(pf, sf)
	PREF	*	pf;
	SUF	*	sf;
{
	LHSFILE *	node,
		*	prev;

	for (prev = node = first.lf; node; prev = node, node = node->next)
		if (node->pf == pf && node->sf == sf)
			return node;

	node = (LHSFILE *) alloc(sizeof(LHSFILE), line);

	node->pf = pf;
	node->sf = sf;

	if (prev)
		prev->next = node;
	else
		first.lf = node;

	return node;
}

/* Add the given rhsfile to the include list
 * of the given lhsfile,
 * if it was not there already.
 */
static void
nodeic(lf, pf, sf)
	LHSFILE	*	lf;
	PREF	*	pf;
	SUF	*	sf;
{
	INCFILE *	node,
		*	prev;

	if (sf->rule == EQU)
	   /* Must follow the Suffix Convention for Included Files.
	    */
		fatal("bad include file suffix %s%s < %s%s",
		   lf->pf->path, lf->sf->name, pf->path, sf->name);
		
	for (prev = node = lf->ic; node; prev = node, node = node->next)
		if (node->pf == pf && node->sf == sf)
			return;

	if (pf == lf->pf && sf == lf->sf)
	{
		error("infinite recursion on %s%s - dependency ignored", pf->path, sf->name);
		return;
	}

	node = (INCFILE *) alloc(sizeof(INCFILE), line);
	node->pf = pf;
	node->sf = sf;

	if (prev)
		prev->next = node;
	else
		lf->ic = node;
}

/* Allocate a zero initialized node.
 */
static char *
alloc(size, obj)
	char	*	obj;
{
	char	*	node;

	if ((node = calloc(1, size)) == NULL)
		fatal("can't alloc: %s", obj);

	return node;
}

/* Hash a character string, and return the hash value
 * modulo the given hash length.
 */
static int
hash(p, hashl)
	char	*	p;
	int		hashl;
{
	register 	unsigned sum;
	register 	c;

	sum = 0;
	while ((c = *p++) != 0)
	{
		if (sum & 01)
			sum = (sum >> 1) + 0x8000;
		else
			sum >>= 1;
		sum += c;
		sum &= 0xFFFF;
	}
	return sum % hashl;
}
