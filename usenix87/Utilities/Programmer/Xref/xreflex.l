%{
#define INFINITE_LOOP  while (1)
int gargc;		/* global argc duplicate */
char **gargv;		/* global argv duplicate */
int n = 1;		/* line counter */
char *fname = "-";	/* current file name */

%}
startid [A-Za-z_]
moreid [A-Za-z_0-9]
%%
{startid}{moreid}*	printf(" %s\t%s\t%d\n", yytext, fname, n);
\n			n++;
.			;
%%

yywrap()
{
    /* reset line count & proceed to next file, or quit */
    n = 1;
    if (next_open())   return(0);
    else   exit(0);
}

next_open()
{
    (void) fclose(stdin);
    while (--gargc > 0) {
	fname = *++gargv;
	if (fopen(fname,"r") != NULL)
	    return(1);
	fprintf(stderr, "xref: can't open %s\n", fname);
    }
    return(0);
}

main(argc,argv,env)
int argc;   char **argv;   char *env[];
{
    gargc = argc;   gargv=argv;
    if (gargc==1 || next_open())
	INFINITE_LOOP (void) yylex();
    exit(2);
}
