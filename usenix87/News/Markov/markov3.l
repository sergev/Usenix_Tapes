%{
/*
 * Copyright (c) 1986, 1987 by Joe Buck
 *
 * Permission is granted to use, redistribute, or modify this program,
 * as long as you don't pretend you wrote it.  Send improvements or
 * bug reports to {ihnp4,hplabs,ames,sun}!oliveb!epimass!jbuck.
 *
 * The program generates simulated Usenet articles, given Usenet articles
 * as input.
 *
 * This program constructs a table of frequencies for a token appearing,
 * given the two preceding tokens.  A "token" is a sequence of non-blank
 * characters.  An entirely blank line is also treated as a token, as is
 * the beginning and end of an article.
 *
 * The program is designed to process news articles, rejecting text from
 * the header, signature, and included text, together with cruft inserted
 * by rn and notes.  A paragraph of included text is treated like a token.
 *
 * After the table is built (and it can be big), articles are generated
 * on the standard output.
 */
#ifndef lint
static char *sccs_id = "@(#)markov3.l	1.1 3/6/87 epimass!jbuck";
#endif
#include <sys/types.h>		/* for time_t */
int in_included_text = 0;
%}
%Start HDR BODY SIG
%%
<HDR>^[^ \t]+:.*\n	;	/* Header line, e.g. "From: foo@bar.UUCP" */
<HDR>^[ \t]+[^ \t].*\n	;	/* Continuation of header line */
<HDR>^[ \t]*$		BEGIN BODY;
<BODY>^"-- "$		BEGIN SIG;
<BODY>^[><)|#}].*\n	{ /* 50% rule gets people to change ">"
			     to other characters; this gets most of them */
			  if (!in_included_text) {
    			      in_included_text = 1;
			      process_token ("\n> ...\n\n");
			  }
			}
<BODY>"]".*\n		{ /* should have been included in the above.  My
			     lex generates bad C code if I say [[><...]
			     even though ed(1) says that's a valid regular
			     expression. */
			  if (!in_included_text) {
			      in_included_text = 1;
			      process_token ("\n> ...\n\n");
			  }
			}
<BODY>^"In article".*\n	;	/* Reject rn crud */
<BODY>^"/* Written".*"*/"\n	;		/* Also NOTES crud */
<BODY>^"/* End of text from".*"*/"\n	;		/* NOTES */
<BODY>^"/* ----------".*"----------*/"\n	;		/* NOTES */
<BODY>[ \t]+		;	/* Skip white space */
<BODY>\n[ \t\n]*\n	{ process_token ("\n"); /* Paragraph break */}
<BODY>[^ \t\n]+		{ in_included_text = 0; process_token (yytext); }
<HDR>.			;	/* Eat anything that escaped */
<HDR>\n			;
<BODY>\n		;
<SIG>.			;
<SIG>\n			;
%%
void perror(), exit();
char *strcpy(), *malloc();
extern int optind;
extern char *optarg;

/*
 * hashtab is a hash table storing all the tokens we encounter.
 */
struct htentry {
    char *htext;
    struct htentry *hlink;
};

#define HSIZE 3557		/* Should be prime */
#define Fprintf (void)fprintf
#define Printf (void)printf

struct htentry hashtab[HSIZE];

/*
 * node and succnode are portions of the big structure we're going to build.
 * node represents something like ("was", "a") in a binary tree.
 * a linked list of succnodes contain tokens that may follow ("was", "a")
 */
struct node {
    char *text;
    char *text2;
    int ocount;
    struct node *lc, *rc;
    struct succnode *succ;
};

struct succnode {
    struct node *scnod;
    int    count;
    struct succnode *link;
};


struct node *prev_code = NULL;
char *prev_token = NULL, **Argv;
int init_state = HDR;
int verbose = 0;
struct node *root = NULL, *tknptr;
struct succnode *start = NULL;
int n_pairs = 0, n_tokens = 0, n_files = 0, n_total = 0;

struct node *insert_token();
char *savetoken();

process_token (txt)
char *txt;
{
     struct node *code;
     char *token = savetoken (txt);
/* We have a new token.  Say the previous two tokens were "one" "way"
 * and the current token is "to".  Then prev_code points to a node
 * for ("one", "way") and token is "to".  This function adds ("way", "to") as a
 * successor to ("one","way") and makes prev_code point to ("way","to").
 */
     code = insert_token (prev_token, token);
     insert_pair (prev_code, code);
     prev_code = code;
     prev_token = token;
     return;
}

/*
 * here it is, the main function.
 */
main (argc, argv)
int argc;
char  **argv;
{
    int     i, c, n_articles = 10, sflag = 0;
    char *dumpfile = NULL;
    extern int  optind;
    extern char *optarg;

    while ((c = getopt (argc, argv, "pxvn:d:s:")) != EOF) {
	switch (c) {
	    case 'v':
		verbose = 1;
		break;
	    case 'p':		/* Input is plain text, not Usenet stuff */
		init_state = BODY;
		break;
	    case 'n': 		/* # articles to generate */
		n_articles = atoi (optarg);
		break;
	    case 'd':		/* where to dump the data structure */
		dumpfile = optarg;
		break;
	    case 's':		/* Set the seed for rand; fall through */
		srand (atoi (optarg));
	    case 'x':		/* set flag to prevent srand */
		sflag++;
		break;
	    default:
		Fprintf (stderr,
		 "Usage: markov3 [-pvx] [-s seed] [-n n_art] [-d dump] files\n");
		exit (1);
	}
    }
    BEGIN init_state;		/* initial state of lexical analyzer */
    if (!sflag)			/* set random number generator */
	srand ((int)time ((time_t *)0));
/* Note: if optind == argc, there are no file arguments.  yyin is left
 * initialized to stdin.
 */
    if (optind < argc) {
/* yyin is lex input stream.  Point to first file. */
	if ((yyin = fopen (argv[optind], "r")) == NULL) {
	    perror (argv[optind]);
	    exit (1);
	}
	optind++;		/* skip to next file */
    }
    Argv = argv;		/* make it global so yywrap can access it */
    n_files = 1;
/* yylex puts all the input files through the lexical analyzer and builds
 * the database.
 */
    (void) yylex ();
    if (dumpfile)
	dump_database (dumpfile);
    if (verbose)
	Fprintf (stderr,
	 "Total of %d tokens (%d different), %d different pairs, %d files\n",
		n_total, n_tokens, n_pairs, n_files);
/* Generate the articles, separated by form feeds */
    for (i = 0; i < n_articles; i++) {
	if (i > 0) output_word ("\n\f\n");
	generate_article ();
    }
    return 0;
}

/*
 * Lex calls this when EOF is reached.  It opens the next file if there
 * is one.  Lex interprets a return value of 1 to mean "all done" and 0
 * to mean "keep going".
 */
yywrap () {
    (void) fclose (yyin);
    insert_pair (prev_code, (struct node *)0);
    prev_code = NULL;
    if (Argv[optind] == NULL) return 1;
    else if ((yyin = fopen (Argv[optind], "r")) == NULL) {
	perror (Argv[optind]);
	exit (1);
    }
    optind++;
    in_included_text = 0;
    if (verbose && n_files % 10 == 0)
	Fprintf (stderr, "%d files\n", n_files);
    n_files++;
    BEGIN init_state;
    return 0;
}

/*
 * This function saves a token in the hash table (if it isn't there
 * already) and returns a pointer to the stored copy.
 */
char *
savetoken (txt)
char *txt;
{
    int h;
    char *p;
    struct htentry *hp;

    n_total++;
    for (p = txt, h = 0; *p; h += *p++);
    hp = hashtab + (h % HSIZE);
    while (hp->hlink) {
	if (strcmp (hp->htext, txt) == 0) {
	    return hp->htext;
	}
	hp = hp->hlink;
    }
/* OK, it's a new token.  Make hp->hlink point to a new,
 * null block and make hp->htext point to the text.
 */
    hp->hlink = (struct htentry *) malloc (sizeof *hp);
    hp->htext = malloc ((unsigned)(strlen (txt) + 1));
    (void) strcpy (hp->htext, txt);
    hp->hlink->hlink = NULL;
    hp->hlink->htext = NULL;
    n_tokens++;
    return hp->htext;
}

/*
 * This recursive function inserts a token pair into the tree.
 */
struct node *
insert_in_tree (p, txt, txt2)
struct node *p;
char *txt, *txt2;
{
    int cmp;
    if (p == NULL) {
/* Create a new node. */
	p = (struct node *) malloc (sizeof *p);
	p->text = txt;
	p->text2 = txt2;
	p->lc = p->rc = NULL;
	p->succ = NULL;
	p->ocount = 1;
	tknptr = p;
	n_pairs++;
	if (verbose && n_pairs % 1000 == 0)
	    Fprintf (stderr, "%d pairs\n", n_pairs);
	return p;
    }
    cmp = my_strcmp (p->text, txt);
    if (cmp == 0) cmp = my_strcmp (p->text2, txt2);
    if (cmp == 0) {
/* It's a match.  Increment the count. */
        tknptr = p;
	p->ocount += 1;
    }
/* Look in the subtrees. */
    else if (cmp < 0) p->lc = insert_in_tree (p->lc, txt, txt2);
    else p->rc = insert_in_tree (p->rc, txt, txt2);
    return p;
}

/*
 * This just calls insert_in_tree starting at the root
 */
struct node *
insert_token (txt, txt2)
char *txt,*txt2;
{
    root = insert_in_tree (root, txt, txt2);
    return tknptr;
}

/*
 * This function adds a successor.
 */
struct succnode *
insert_in_succ_chain (sp, np)
struct succnode *sp;
struct node *np;
{
    if (sp == NULL) {
	sp = (struct succnode *) malloc (sizeof *sp);
	sp->scnod = np;
	sp->count = 1;
	sp->link = NULL;
    }
    else if (sp->scnod == np)
	sp->count += 1;
    else sp->link = insert_in_succ_chain (sp->link, np);
    return sp;
}

/*
 * This calls insert_in_succ_chain starting at the right place.
 */
insert_pair (p1, p2)
struct node *p1, *p2;
{
    if (p1) p1->succ = insert_in_succ_chain (p1->succ, p2);
    else start = insert_in_succ_chain (start, p2);
}

/*
 * This function dumps the stored data structure onto a file.
 * Now if only I had a function to read it back in.
 */
char *
pr_token (txt)
char *txt;
{
    if (txt[0] != '\n')
	return txt;
    return txt[1] ? "<INCL>" : "<LF>";
}

treedump (tree, fp)
struct node *tree;
FILE *fp;
{
    if (tree) {
	treedump (tree->rc, fp);
	Fprintf (fp, "( %s %s ) %d", pr_token (tree->text),
			pr_token (tree->text2), tree->ocount);
	chaindump (tree->succ, fp);
	treedump (tree->lc, fp);
    }
}

/*
 * Subroutine of treedump; it does one row.
 */
chaindump (p, fp)
struct succnode *p;
FILE *fp;
{
    char   *text;
    while (p) {
	if (p->scnod == NULL)
	    text = "<EOF>";
	else text = pr_token (p->scnod->text2);
	Fprintf (fp, " %s %d", text, p->count);
	p = p->link;
    }
    putc ('\n', fp);
}

/*
 * This routine generates the dump file (-d option)
 */
dump_database (file)
char *file;
{
    FILE *fp = fopen (file, "w");
    if (fp == NULL) {
	Fprintf (stderr, "markov: can't open ");
	perror (file);
	exit (1);
    }
    Fprintf (fp, "START:");
    chaindump (start, fp);
    treedump (root, fp);
}

/* roll (n) generates a uniformly distributed rv between 0 and n-1.
 * This code is stolen from "hack" and should be portable.  If you
 * change this, remember that different systems have rand functions
 * with different ranges, and the bottom bits are often no good.
 */
#define roll(n) ((rand() >> 3) % n)

/*
 * This function generates an article by traversing the
 * structure we've built.
 */
generate_article () {
    struct succnode *p = start;
    int ncounts = n_files;
    int n, accum;
    char *tp;

    while (1) {
/* Roll the dice to find out the next token.  The code below selects the
 * next token, and the new state, with a probability corresponding to the
 * frequency in the input.
 */
	n = roll (ncounts);
	accum = p->count;
	while (accum <= n && p->link) {
	    p = p->link;
	    accum += p->count;
	}
	if (p->scnod == NULL)
	    break;
	tp = p->scnod->text2;
/* Check for "end of story" */
	if (tp == NULL)
	    break;
	output_word (tp);
	ncounts = p->scnod->ocount;
	p = p->scnod->succ;
    }
    output_word ("\n");	/* This will flush the buffer as well. */
    return;
}

/*
 * This version handles null strings *
 */
my_strcmp (a, b)
register char *a, *b;
{
    if (a == NULL) return b ? -1 : 0;
    if (b == NULL) return 1;
    return strcmp (a, b);
}

#define LEN 75
output_word (word)
char *word;
{
    static char line[LEN+1];
    static int room = LEN;
    int l;

    if (word == NULL) return;
    l = strlen (word);
/* If word won't fit, or starts with \n, dump the current line */
    if ((l >= room || word[0] == '\n') && line[0]) {
	Printf ("%s\n", line);
	line[0] = 0;
	room = LEN;
    }
/* If word won't fit in the buffer or starts with \n, print it now */
    if (l >= LEN)
	Printf ("%s\n", word);
    else if (word[0] == '\n')
	Printf ("%s", word);
/* Otherwise fill it in */
    else {
	(void)strcat (line, word);
	(void)strcat (line, " ");
	room -= (l + 1);
    }
    return;
}
