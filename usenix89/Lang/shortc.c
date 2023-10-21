char ID[] = "@(#) shortc";
char Usage[] = "usage: shortc [-symlen] [-cp] file ... > Short.h\n";
/*
   Produce a set of preprocessor defines which guarantee that all identifiers
   in the files are unique in the first symlen (default 7) characters.
   Include the output into each file (or into a common header file).
   Since the symbols being redefined are ambiguous within symlen chars
   (that was the problem in the first place), the files must be compiled
   using a flexnames version of cpp.
   Lacking that, turn the output into a sed script and massage the source
   files.  In this case, you may need to specify -p to parse preprocessor
   lines, but watch for things like include-file names.
   Alternatively, you can specify -c as an option and instead of a list
   of defines the C source for a program to filter the names will be
   emitted.
   If using cpp, preprocessor symbols should be weeded out by hand; otherwise
   they will cause (innocuous) redefinition messages.
   To lock in names that you want unchanged (if possible) list the files
   containing them first on the processing list.  If symbols and the
   interfering names are in the same file then dummy up a file which
   is nought but a list of names and include it first.
 */

#include <ctype.h>
#include <stdio.h>

#define SYMLEN  7           /* symbols must be unique within ... chars */
#define MAXLEN  128         /* need a limit for tokenizing */
#define HASHSIZ 2048        /* power of 2; not an upper limit */
#define TRUE 1
#define FALSE 0
#define MAXSTRING 160

typedef struct Symbol symbol;
struct Symbol {
	symbol  *link;          /* hash chain */
	union {
	    long chcase_mask;  /* re-capitalize for mapped name if flag > SEEN */
	    symbol *xtrunc;    /* symbol which truncates to this one
				  if flag == TRUNC */
	} x;
	char    flag;
	char    inname[1];
};
#define chcase  x.chcase_mask
#define trunc   x.xtrunc
#define NOTSEEN 0           /* symbol never seen */
#define TRUNC   1           /* trunc points to symbol which truncates to
			       this one */
#define SEEN    2           /* there is no conflict with this symbol */
#define MULT    3	    /* re-capitalize to resolve conflict */

symbol  *symtab[HASHSIZ];

struct subsnames {
    char *from_name;
    char *to_name;
    struct subsnames *next;
    } *subsname_list = NULL;

int c_prog = FALSE;

int     symlen  = SYMLEN;
char    parsepp;            /* if set, parse preprocessor lines */

symbol  *lookup();
char    *token(), *truncname();
char    *myalloc();

extern  char *strcpy(), *strncpy();
extern  char *malloc();

main (argc, argv) register char **argv; register argc; /*: entry point */
{
	while( --argc > 0 )
	    doarg(*++argv);

	dump();
	exit(0);
}

doarg (arg) char *arg; /*: process one file or flag arg */
{
	register char *s;
	register symbol *y;

	if( *arg == '-' )
	{   arg++;
	    if( isdigit(*arg) )
		symlen = atoi(arg);
	    else if( *arg == 'p' )
		parsepp = 1;
	    else if( *arg == 'c' )
		c_prog = TRUE;
	    else fputs(Usage, stderr);
	    return;
	}

	if( freopen(arg, "r", stdin) == NULL )
	{   perror(arg);
	    return;
	}

	while( s = token() )
	    if( (y = lookup(s))->flag < SEEN )
		newname(y);
}

newname (y) register symbol *y; /*: pick a new non-colliding name */
{
	register symbol *a;

	/* repeat until no collision */
	for( ;; )
	{   /* pick another name */
	    nextname(y);
	    /* check its truncation for conflicts */
	    a = lookup(truncname(y));
	    if( a->flag == NOTSEEN )
		break;
	    /* if this is an original symbol and it collides with another
	     * (maybe modified) symbol, fix the other one instead of this one
		DCA - eliminated this because it makes it so that last
                truncation wins.  To lock in names unmodified it would be
                preferable that the first one wins so that we can
                include a file at the front which locks in the symbols
                we want unchanged.

	    if( a->flag == TRUNC && y->flag == SEEN )
	    {   newname(a->trunc);
		break;
	    }
	    */
	    /* if already a short name, ok */
	    if( a == y )
		return;
	}
	/* flag what this truncates to */
	a->trunc = y;
	a->flag = TRUNC;
}

nextname (y) register symbol *y; /*: find next possible name for this symbol */
{
	register char *s, *p;
	register n;

	switch( y->flag )
	{   case TRUNC:
		/* if another symbol truncates to this one, fix it not to */
		newname(y->trunc);
	    case NOTSEEN:
		/* this symbol's name is available, so use it */
		y->flag = SEEN;
		y->chcase = 0;
		return;
	}
	y->flag = MULT;
	y->chcase++;
}

char *truncname (y) register symbol *y; /*: return symbol name truncated to symlen chars */
{
	static char buf[MAXLEN+10];

	register long chcase_mask = y->chcase;
	register i;
	register char *str = y->inname, *str1 = buf, c;

        for(i = 0; i < symlen; i++) {
	    if (chcase_mask & 01) {
		c = *(str++);
		if (isupper(c)) c = tolower(c);
		else c = toupper(c);
		*(str1++) = c;
 	    }
	    else {
		*(str1++) = *(str++);
	    }
	    chcase_mask >>= 1;
	}
	return buf;
}

symbol *lookup(s) char *s; /*: find name in symbol table */
{
	register h;

	{   register char *p;
	    register c;

	    for( h = 0, p = s; (c = *p++); )
		h += h + c;
	}

	{   register symbol *y, **yy;

	    for( y = *(yy = &symtab[h & HASHSIZ-1]);; y = y->link )
	    {   if( !y )
		{   y = (symbol *)myalloc(sizeof *y + strlen(s));
		    strcpy(y->inname, s);
		    y->flag = NOTSEEN;
		    y->link = *yy;
		    *yy = y;
		    break;
		}
		if( strcmp(y->inname, s) == 0 )
		    break;
	    }
	    return y;
	}
}

dump () /*: output all mappings */
{
	register symbol *y;
	register n,i;
	register char c, *str;
	struct subsnames *new_name, *curr_name, *prev_name;
	FILE *template_file;
	char in_string[MAXSTRING];

    if (!c_prog) {
	for( n = HASHSIZ; --n >= 0; ) {
	    for( y = symtab[n]; y; y = y->link ) {
		if( y->flag == MULT ) {
		    str = y->inname;
		    i = y->chcase;
		    printf("#define %s ", y->inname);
		    while (c = *(str++)) {
			if (i & 01) {
			    if (isupper(c)) c = tolower(c);
			    else c = toupper(c);
			    putchar(c);
			}
			else {
			    putchar(c);
			}
			i >>= 1;
		    }
		    putchar('\n');
		}
	    }
	}
    }
    else {
	for( n = HASHSIZ; --n >= 0; ) {
	    for( y = symtab[n]; y; y = y->link ) {
		if( y->flag == MULT ) {
		    i = y->chcase;
		    new_name = (struct subsnames *) malloc(sizeof(*new_name));
		    new_name->from_name =(char *) malloc(strlen(y->inname) + 1);
		    new_name->to_name = (char *) malloc(strlen(y->inname) + 1);
		    strcpy(new_name->from_name, y->inname);
		    strcpy(new_name->to_name, y->inname); 
		    str = new_name->to_name;
		    while (c = *(str++)) {
			if (i & 01) {
			    if (isupper(c)) c = tolower(c);
			    else c = toupper(c);
			    *(str - 1) = c;
			}
			i >>= 1;
		    }
		    prev_name = NULL;
		    curr_name = subsname_list;
		    str = new_name->from_name;
		    while (curr_name && strcmp(curr_name->from_name,str) < 0) {
			prev_name = curr_name;
			curr_name = prev_name->next;
		    }
		    if (!prev_name) {
			new_name->next = subsname_list;
			subsname_list = new_name;
		    }
		    else {
			new_name->next = prev_name->next;
			prev_name->next = new_name;
		    }
		}
	    }
	}
	template_file = fopen("/usr/src/local/shortc/template.c","r");
	if (!template_file) {
	    fprintf(stderr, "unable to open template file\n");
	    exit(1);
	}
	i = TRUE;
	n = -1;
	while (fgets(in_string, MAXSTRING - 1, template_file)) {
	    if (!i && !strncmp(in_string,"/*E*/",5)) i = TRUE;
	    if (i) fputs(in_string, stdout);

	    if (!strncmp(in_string,"/*1*/",5)) {
		curr_name = subsname_list;
		while (curr_name) {
		    n++;
		    if (curr_name->next) {
			printf("	\"%s\",\n", curr_name->from_name);
		    }
		    else {
			printf("	\"%s\"\n", curr_name->from_name);
		    }
		    curr_name = curr_name->next;
		}
		i = FALSE;
	    }
	    else if (!strncmp(in_string,"/*2*/",5)) {
		curr_name = subsname_list;
		while (curr_name) {
		    if (curr_name->next) {
			printf("	\"%s\",\n", curr_name->to_name);
		    }
		    else {
			printf("	\"%s\"\n", curr_name->to_name);
		    }
		    curr_name = curr_name->next;
		}
		i = FALSE;
	    }
	    else if (!strncmp(in_string,"/*3*/",5)) {
		printf("	%d\n", n);
		i = FALSE;
	    }
	}
	close(template_file);
    }
}

char *token () /*: return next interesting identifier */
{
	register c, state = 0;
	register char *p;
	static char buf[MAXLEN+1];

	for( p = buf; (c = getchar()) != EOF; )
	{   if( state )
	    switch( state )
	    {   case '/':
		    if( c != '*' )
		    {   state = 0;
			break;
		    }
		    state = c;
		    continue;

		case 'S':
		    if( c == '/' )
			state = 0;
		    else
			state = '*';
		case '*':
		    if( c == state )
			state = 'S';
		    continue;

		default:
		    if( c == '\\' )
			(void) getchar();
		    else if( c == state )
			state = 0;
		    continue;
	    }
	    if( isalnum(c) || c == '_' )
	    {   if( p < &buf[sizeof buf - 1] )
		    *p++ = c;
		continue;
	    }
	    if( p > buf )
	    {   if( p-buf >= symlen && !isdigit(*buf) )
		{   *p = '\0';
		    ungetc(c, stdin);
		    return buf;
		}
		p = buf;
	    }
	    if( c == '"' || c == '\'' || c == '/' )
		state = c;
	    else if( c == '#' && !parsepp )
		state = '\n';
	}
	return NULL;
}

char *myalloc(n) /*: malloc with error detection */
{
	register char *p;

	if( !(p = malloc((unsigned)n)) )
	{   fprintf(stderr, "Out of space\n");
	    exit(1);
	}
	return p;
}



