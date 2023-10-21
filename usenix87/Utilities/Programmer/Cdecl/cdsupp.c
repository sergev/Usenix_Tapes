#include <stdio.h>
char *malloc();

main()
{
	yyparse();
}

unsupp(s)
char *s;
{
	printf("Warning: Unsupported in C -- %s\n",s);
}

unport(s)
char *s;
{
	printf("Warning: Non-portable construction -- %s\n",s);
}

yyerror(s)
char *s;
{
	printf("%s\n",s);
}

yywrap()
{
	return 1;
}

/*
 * Support for dynamic strings:
 * cat creates a string from three input strings.
 * The input strings are freed by cat (so they better have been malloced).
 * ds makes a malloced string from one that's not.
 */

char *
cat(s1,s2,s3)
char *s1,*s2,*s3;
{
	register char *newstr;
	register unsigned len = 0;

	if (s1 != NULL) len = strlen(s1) + 1;
	if (s2 != NULL) len += strlen(s2);
	if (s3 != NULL) len += strlen(s3);
	newstr = malloc(len);
	if (s1 != NULL) {
		strcpy(newstr,s1);
		free(s1);
	}
	if (s2 != NULL) {
		strcat(newstr,s2);
		free(s2);
	}
	if (s3 != NULL) {
		strcat(newstr,s3);
		free(s3);
	}
	return newstr;
}

char *
ds(s)
char *s;
{
	register char *p;

	p = malloc((unsigned)(strlen(s)+1));
	strcpy(p,s);
	return p;
}

static char *helptext[] = {
	"[] means optional; {} means 1 or more; <> means defined elsewhere\n",
	"command:\n",
	"  declare <name> as <english>\n",
	"  cast <name> into <english>\n",
	"  explain <gibberish>\n",
	"english:\n",
	"  function [( <name> )] returning <english>\n",
	"  array [<number>] of <english>\n",
	"  pointer to <english>\n",
	"  <type>\n",
	"type:\n",
	"  [{<modifier>}] <C-type>\n",
	"  {<modifier>} [<C-type>]\n",
	"  <sue> <name>\n",
	"name is a C identifier\n",
	"gibberish is a C declaration\n",
	"C-type is int, char, double or float\n",
	"modifier is short, long or unsigned\n",
	"sue is struct, union or enum\n",
	NULL
};

help()
{
	register char **p;

	for (p=helptext; *p!=NULL; p++)
			printf("\t%s",*p);
}
