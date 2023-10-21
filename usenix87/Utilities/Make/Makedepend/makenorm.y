%{
/* -- Normalize include references
 *
 * $File: makenorm.y  1.8  1985-03-29  14:11:19 $
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
 *	Descr:	This program implements part of the algorithm described
 *      	in K. Walden, "Automatic Generation of Make Dependencies",
 *      	Softw. Practice & Exper., vol. 14, no. 6, June 1984 
 *
 *
 * $Log:	makenorm.y,v $
 * 
 * Revision 1.8  1985-03-29  14:11:19  kim
 * New source format
 * 
 * Revision 1.7  1985-03-21  09:31:11  kim
 * Use sprintf in a System V compatible way
 * 
 * Revision 1.6  1985-03-08  01:18:35  kim
 * Added support for parallell transfer rules.
 * Added -1 option to makedep and depend.
 * 
 * Revision 1.5  1985-02-25  19:41:56  kim
 * Added -a option for absolute directories
 * 
 * Revision 1.4  1985-02-20  11:53:09  kim
 * Bug fix
 * 
 * Revision 1.3  1985-02-19  11:21:23  kim
 * New default include rules
 * 
 * Revision 1.2  1985-02-18  14:24:15  kim
 * New error handling
 * 
 * Revision 1.1  1985-02-16  18:57:07  kim
 * Initial revision
 */
%}

%token SPACE RULSEP INCSEP PATH SUFF INCL QUOTE LPAR RPAR
%{
# define DIRLIST  struct dirlist
# define PREF     struct pref
# define SUF      struct suf
# define SOURCE   struct source
# define NAME     struct filename
# define PATHSIZ  100
# define DIRSIZ   14
# define LINSIZ   256
# define OUTSIZ   10
# define PHASHL   503
# define LINE	  80
# define EXACT    0
# define DERIVED  1
# define FORCE	  2
# define NOREDUCE 0
# define REDUCE   1
# define FALSE	  0
# define TRUE	  1
# define CNULL   '\0'
# define KOL     ':'
# define EQU     '='
# define RULES	 ".o: .c; .c= .y .l .q; .h= .y"

# include <stdio.h>
# include <strings.h>
# include <sys/types.h>
# include <sys/stat.h>

static char usage[]=
"Usage: makenorm [-rfile] [-Rrules] [-Ilist] [-alist] [-i] makedir sourcefiles";

/* Directory lists
 */
DIRLIST
{
	DIRLIST	*	next;
	char		path[PATHSIZ+1];
};

/* File name prefixes   (including directory part)
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

/* Declared sources:
 *
 * The transfer rules are successively applied
 * to the suffix before a file is inserted.
 */
SOURCE
{
	SOURCE	*	next;
	PREF	*	pf;
	SUF	*	sf;
};

/* File names
 */
NAME
{
	PREF	*	pf;
	SUF	*	sf;
};


struct 
{
	SOURCE	*	so;		/* source list */
	SUF	*	sf;		/* suffix list */
	DIRLIST	*	idir, 		/* list of include directories */
		*	adir;		/* list of absolute directories */
} first;

extern char	*	calloc(),
		*	mpath();
void			yyerror();
static PREF 	*	nodepf(),
		*	phash[PHASHL];	/* hash table for prefixes */
static SUF	*	nodesf(),
		*	lsuf,
		*	rsuf;
static NAME		getname(),
			lhs,
			rhs;
static char	*	alloc(),
		*	makedir,
		*	rulstring,
		*	yystring;
static char		line[LINSIZ];
static int		derived(),
			hash(),
			findso(),
			printcond(),
			lineno,
			rule,
			remote,
			sysflag;
static void		getdirlist(),
			nodedl(),
			printnorm();

# include "normscan.c"

main(argc, argv)
	int		argc;
	char	*	argv[];
{
	char		rulbuf[LINSIZ];

	setprogname(argv[0]);
	rulstring = rulbuf;
	strcpy(rulbuf, RULES);
	nodedl("/usr/include", &first.adir);
	sysflag = 0;

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

		 case 'i':
			sysflag++;
			break;

		 case 'a':
		 case 'I':
			getdirlist(argv[1]);
			break;

		 default:
			error("unknown option %s", argv[1]);
			fatalmesg(usage);
		}

		argv++;
		argc--;
	}

	if (argc < 2)
		fatalmesg(usage);
	makedir = argv[1];
	argv ++;
	argc --;

	if (argc < 2)
		fatal("no source files specified");

	while (--argc)
		findso(getname(argv++[1], REDUCE), FORCE);

	yystring = rulstring;
	BEGIN rules;

	yyparse();
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
include	:	lhs
		QUOTE
			{	remote = 0;	}
		rhs
		QUOTE
			{	unput('#');
				BEGIN comment;
			}
	|	lhs
		LPAR
			{	remote = 1;	}
		rhs
		RPAR
			{	unput('#');
				BEGIN comment;
			}
	;
lhs	:	PATH
			{   /* Lhs of include statement:
			     *      Reduce filename, and check that
			     *      file name was in the source list.
			     */
				lhs = getname(yytext, REDUCE);
				if (!findso(lhs, EXACT))
					error("%s not in source list",
					   yytext);
			}
		':' INCL 
	;
rhs	:	PATH
			{   /* Rhs of include statement:
			     *      Resolve file reference and print
			     *      normalized include statement.
			     */
				rhs = getname(yytext, NOREDUCE);
				if (rhs.sf->rule == EQU)
					fatal("bad include file suffix: %s",
					   line);
				printnorm();
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

/* Resolve current include file reference and print
 * the corresponding normalized include statement.
 */
static void
printnorm()
{
	DIRLIST *	dl;
	char	*	p;
	char		dir[PATHSIZ+1];

    /* Try directory of left hand side file when
     *    format: #include "file".
     */
	if (!remote)
	{
		strcpy(dir, lhs.pf->path);

		if ((p = rindex(dir, '/')) == NULL)
			p = dir;

		*p = CNULL;

		if (printcond(dir))
			return;
	}

    /* Try user include libraries */

	for (dl = first.idir; dl; dl = dl->next)
		if (printcond(dl->path))
			return;

    /* Try system include library */

	if (printcond("/usr/include"))
		return;
	
	error("cannot resolve: %s", line);
}

static int
printcond(dir)
	char	*	dir;
{
	struct stat	stbuf;
	NAME		nm;
	char		buf[PATHSIZ+1];

    /* If include reference is not absolute,
     * prepend directory.
     */

	if (strlen(dir) != 0 && rhs.pf->path[0] != '/')
		sprintf(buf, "%s/%s%s", dir, rhs.pf->path, rhs.sf->name);
	else
		sprintf(buf, "%s%s", rhs.pf->path, rhs.sf->name);

	nm = getname(buf, REDUCE);

	sprintf(buf, "%s%s", nm.pf->path, nm.sf->name);

    /* If the resulting file name path can be derived from a source,
     * accept include reference and print dependency.
     */
	if (findso(nm, DERIVED))
	{
		printf("%s%s < %s\n", lhs.pf->path, lhs.sf->name, buf); 
		return TRUE;
	}

    /* Ignore system include references not specified as sources,
     * unless -i option was specified.
     */
	if (strncmp(buf, "/usr/include/", strlen("/usr/include/")) == 0
	       && !sysflag && (stat(buf, &stbuf) == 0))
		return TRUE;

	return FALSE;
}


/* Return file name,
 *    reduced if flag == REDUCE.
 */
static NAME
getname(path, flag)
	char	*	path;
	int		flag;
{
	NAME		nm;
	DIRLIST *	dl;
	char	*	mp,
		*	p;
	char		buf[PATHSIZ+1];
	int		c;

	mp = path;

	if (flag == REDUCE )
	{
	    int absdir = 0;

	    /* Make reference absolute */
		if (*path != '/')
		{
			sprintf(buf, "%s/%s", makedir, path);
			path = buf;
		}

	    /* If the file name refers to one of the absolute directories,
	     * it should be in absolute form.
	     */
		for (dl = first.adir; dl; dl = dl->next)
		{
			if ((mp = mpath(path, dl->path)) == NULL)
				fatal("can't reduce path");

			if (strncmp(mp, "../", 3) != 0)
			{
				if (strlen(mp)+strlen(dl->path)+1 > PATHSIZ)
					fatal("path too long: %s/%s",
					   dl->path, mp);

				absdir++;
				sprintf(buf, "%s/%s", dl->path, mp);
				mp = buf;
				break;
			}
		}

	    /* If the file name does not refer to any of the
	     * absolute directories, it should be in relative form.
	     */
		if (!absdir)
		{
			if ((mp = mpath(path, makedir)) == NULL)
				fatal("can't reduce path");

			if (strlen(mp) > PATHSIZ)
				fatal("path too long: %s", mp);
		}
	}


	if ((p = rindex(mp, '/')) == NULL)
		p = mp;

	if ((p = rindex(p, '.')) == NULL)
		p = mp + strlen(mp);

	nm.sf = nodesf(p);
	c = *p;
	*p = CNULL;
	nm.pf = nodepf(mp);
	*p = c;

	return nm;
}

static char *
getword(p, buf)
	char	*	p,
		*	buf;
{
	while (*p == ' ' || *p == '\t')
		p++;

	if (*p == CNULL)
		return NULL;

	while (*p != ' ' && *p != '\t' && *p != CNULL)
		*buf++ = *p++;

	*buf = CNULL;
	return p;
}

static void
getdirlist(arg)
	char	*	arg;
{
	DIRLIST **	head;
	char	*	s;
	char		buf[PATHSIZ+1];

	if (arg[1] == 'a')
		head = &first.adir;
	else
		head = &first.idir;
	
	s = arg;
	s += 2;
	while (s = getword(s, buf))
	{
		if (arg[1] == 'a' && buf[0] != '/')
			fatal("directory list must be absolute: %s", arg);

		nodedl(buf, head); 
	}
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

/* Search the source list for the given file name:
 *
 *   1. flag = EXACT		Look for exact match.
 *				Return TRUE if found, else FALSE
 *
 *   2. flag = FORCE		Look for exact match.
 *				If not found, allocate a new node.
 *				Always return TRUE.
 *
 *   3. flag = DERIVED		Return TRUE if a match can be found by applying
 *				transfer rules to the file name suffixes in the
 *			 	source list, else FALSE.
 */
static int
findso(nm, flag)
	NAME		nm;
	int		flag;
{
	SOURCE	*	node,
		*	prev;

	for (prev = node = first.so; node; prev = node, node = node->next)
		if (node->pf == nm.pf
		  && (node->sf == nm.sf || DERIVED && derived(nm.sf, node->sf)))
			return TRUE;

	if (flag == FORCE)
	{
		node = (SOURCE *) alloc(sizeof(SOURCE), line);

		node->pf = nm.pf;
		node->sf = nm.sf;

		if (prev)
			prev->next = node;
		else
			first.so = node;

		return TRUE;
	}
	else
		return FALSE;
}

/* If dsf is same as sf, or can be derived from it by applying
 * a number of transfer rules, return TRUE, else FALSE
 */
static int
derived(dsf, sf)
	SUF	*	dsf,
		*	sf;
{
	int		i;

	if (dsf == sf)
		return TRUE;

	if (sf->rule != EQU)
		return FALSE;

	for (i = 0; i < sf->top; i++)
		if (derived(dsf, sf->out[i]))
			return TRUE;

	return FALSE;
}

/* Append directory to search list.
 */
static void
nodedl(path, head)
	char	*	path;
	DIRLIST **	head;
{
	DIRLIST *	node,
		*	prev;

	for (prev = node = *head; node; prev = node, node = node->next)
		if (strcmp(path, node->path) == 0)
			return;

	node = (DIRLIST *) alloc(sizeof(DIRLIST), path);
	strcpy(node->path, path);

	if (prev)
		prev->next = node;
	else
		*head = node;

	return;
}


/* Allocate a zero initialized node.
 */
static char *
alloc(size, obj)
	char	*	obj;
{
	char	*	node;

	if ((node = calloc(1, size)) == NULL)
		fatal("can't alloc %s", obj);

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
	register	unsigned sum;
	register	c;

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
