#include <stdio.h>

/*
 * stripped down version of deroff for use with edit spell checker
 * simply ignores nroff commands, but handles backslash sequences

All input is through the C macro; the most recently read character is in c.
*/

#define C ( (c=getchar()) == EOF ? exit(0) : c )
#define SKIP while(C != '\n') 

#define YES 1
#define NO 0

#define NOCHAR -2
#define SPECIAL 0
#define APOS 1
#define DIGIT 2
#define LETTER 3

char chars[128];  /* SPECIAL, APOS, DIGIT, or LETTER */

char line[512];
char *lp;

int c;
int ldelim	= NOCHAR;
int rdelim	= NOCHAR;





work()
	{
	register i;

	for(i='a'; i<='z' ; ++i)
		chars[i] = LETTER;
	for(i='A'; i<='Z'; ++i)
		chars[i] = LETTER;
	for(i='0'; i<='9'; ++i)
		chars[i] = DIGIT;
	chars['\''] = APOS;
	chars['&'] = APOS;
	
	
	for( ;; )
		{
		if(C == '.'  ||  c == '\'')
			comline();
		else
			regline(NO);
		}
	}




regline(macline)
	int macline;
	{
	line[0] = c;
	lp = line;
	for( ; ; )
		{
		if(c == '\\')
			{
			*lp = ' ';
			backsl();
			}
		if(c == '\n') break;
		*++lp = C;
		}
	
	*lp = '\0';
	
	if(line[0] == '\001')
		accept(&line[1]);
	else if(line[0] != '\0')
		putwords(macline);
	}




putwords(macline)	/* break into words for -w option */
int macline;
{
register char *p, *p1;
int i, nlet;
extern char word[];
char *wp;


for(p1 = line ; ;)
	{
	/* skip initial specials ampersands and apostrophes */
	while( chars[*p1] < DIGIT)
		if(*p1++ == '\0') return;
	nlet = 0;
	for(p = p1 ; (i=chars[*p]) != SPECIAL ; ++p)
		if(i == LETTER) ++nlet;

	if( (!macline && nlet>1)   /* MDM definition of word */
	   || (macline && nlet>2 && chars[ p1[0] ]==LETTER && chars[ p1[1] ]==LETTER) )
		{
		/* delete trailing ampersands and apostrophes */
		while(p[-1]=='\'' || p[-1]=='&')
			 --p;
/*
		while(p1 < p) putchar(*p1++);
		putchar('\n');
*/
		for(wp=word; p1<p; p1++, wp++)
			*wp = *p1;
		*wp='\0';
		check();
		}
	else
		p1 = p;
	}
}


comline()
	{
	SKIP;
	}



backsl()	/* skip over a complete backslash construction */
{
int bdelim;

sw:  switch(C)
	{
	case '"':
		SKIP;
		return;
	case 's':
		if(C == '\\') backsl();
		else	{
			while(C>='0' && c<='9') ;
			ungetc(c,stdin);
			c = '0';
			}
		--lp;
		return;

	case 'f':
	case 'n':
	case '*':
		if(C != '(')
			return;

	case '(':
		if(C != '\n') C;
		return;

	case '$':
		C;	/* discard argument number */
		return;

	case 'b':
	case 'x':
	case 'v':
	case 'h':
	case 'w':
	case 'o':
	case 'l':
	case 'L':
		if( (bdelim=C) == '\n')
			return;
		while(C!='\n' && c!=bdelim)
			if(c == '\\') backsl();
		return;

	case '\\':
/*
		if(inmacro)
			goto sw;
*/
	default:
		return;
	}
}
