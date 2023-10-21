
/* -*-c,save-*- */
/*------------------------------------------------------
 *	PTOC.C - Pascal to C pre-translator
 *	Robert Heller Fri Feb  1 09:55:14 1985
 *------------------------------------------------------*/
#define VMS /* VMS version */
/*#define CPM /* CP/M-68K version */
#ifdef VMS
#include <stdio.h>	/* standard I/O */
#include <ctype.h>	/* character macros */
#define FAST register	/* (not really needed under VMS...) */
#define ROM readonly static	/* (for Read Only stuff, if any) */
#define LOCAL static	/* for static stuff */
#define GLOBAL extern	/* for external stuff */
#endif
#ifdef CPM
#include <stdio.h>	/* standard I/O */
#include <ctype.h>	/* character macros */
#define FAST register	/* fast access vars */
#define ROM static	/* (for Read Only stuff, if any) */
#define LOCAL static	/* for static stuff */
#define GLOBAL extern	/* for external stuff */
#define CPMEOF 0x1a	/* ^Z - EndOfFile under CP/M */
#endif

/* main routine:  get words and special character and process them */
main()
{
    FAST char *letter;
    LOCAL word[100];
    FAST int wordlnth,c;

    letter = (&word[0]);
    wordlnth = 0;
    while ((c=getchar()) != EOF
#ifdef CPM
	|| (isatty(fileno(stdin)) && c != CPMEOF)
#endif
    ) {
	if (isalpha(c)) {
	    *letter++ = c;
	    wordlnth++;
	    }
	else {
	    if (wordlnth > 0) {
		*letter = '\0';
		wtest(word);
		wordlnth = 0;
		letter = (&word[0]);
		}
	    ctest(c);
	    }
	}
    if (wordlnth>0) {
	*letter = '\0';
	wtest(word);
	putchar('\n');
	}
    }
wtest(word)
FAST char *word;
{
    FAST char *swapword;

    swapword = word;
    switch (*word) {
    case 'w': case 'W':
	if (cf_strcmp(word,"writeln") == 0 ||
	    cf_strcmp(word,"write") == 0) swapword = "printf";
	break;
    case 'r': case 'R':
	if (cf_strcmp(word,"readln") == 0 ||
	    cf_strcmp(word,"read") == 0) swapword = "scanf";
	break;
    case 't': case 'T':
	if (cf_strcmp(word,"then") == 0) swapword = "\0";
	break;
    case 'a': case 'A':
	if (cf_strcmp(word,"and") == 0) swapword = "&&";
	break;
    case 'o': case 'O':
	if (cf_strcmp(word,"or") == 0) swapword = "||";
	break;
    case 'b': case 'B':
	if (cf_strcmp(word,"begin") == 0) swapword = "{";
	break;
    case 'e': case 'E':
	if (cf_strcmp(word,"end") == 0) swapword = ";}";
	break;
    default: break;
	}
    swap(swapword);
    }
ctest(c)
FAST c;
{
    switch (c) {
    case '\'' : putchar('"'); break;
    case '{' : swap("/*"); break;
    case '}' : swap("*/"); break;
    case '(' : swapif('(','*',"/*"); break;
    case '*' : swapif('*',')',"*/"); break;
    case ':' : swapif(':','=',"="); break;
    case '<' : swapif('<','>',"!="); break;
    case '>' : putchar(c); putchar(getchar()); break;
    case '=' : swap("=="); break;
    default: putchar(c);
	}
    }
swap(s)
FAST char *s;
{
    while(*s != '\0') putchar(*s++);
    }
swapif(first,second,replacement)
FAST char first,second,*replacement;
{
    FAST char c;

    if ((c=getchar()) == second) swap(replacement);
    else {
	putchar(first);
	ungetc(c,stdin);
	}
    }
cf_strcmp(s1,s2)
FAST char *s1,*s2;
{
    FAST int i;

    while(*s1 != '\0' && *s2 != '\0' && toupper(*s1) == toupper(*s2)) {
	s1++;
	s2++;
	}
    i = toupper(*s1) - toupper(*s2);
    if (i<0) return(-1);
    else if (i>0) return(1);
    else return(0);
    }
