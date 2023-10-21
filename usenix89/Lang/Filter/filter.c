
#include <ctype.h>

#define NORMAL		0
#define STR		1
#define STRESC		2
#define SLASH		3
#define COMM		4
#define STAR		5
#define NUM		6
#define ZERO		7
#define WORD		8
#define BIN		9

#define BUFSIZE	100
int	buf[BUFSIZE];
int	bufp		= 0;

getch()
{
    return( (bufp > 0) ? buf[--bufp] : getchar() );
}

ungetch(c)
int	c;
{
    if (bufp >= BUFSIZE)
	fprintf(stderr, "ungetch: too many chars\n");
    else
	buf[bufp++] = c;
}

main()
{
    int	c,
	oc,
	state,
	num;

    state = NORMAL;
    while ((c=getch())!=EOF) {
	switch(state) {
	    case NORMAL:if (isalpha(c) || c=='_') {
			    putchar(c);
			    state = WORD;
			} else if ( c=='0' ) {
			    putchar(c);
			    num = 0;
			    state = ZERO;
			} else if (isdigit(c) || c=='.') {
			    putchar(c);
			    state = NUM;
			} else if ( c=='\'' || c=='"' ) {
			    putchar(c);
			    state = STR;
			    oc=c;
			} else if ( c=='/' ) {
			    putchar(c);
			    state = SLASH;
			} else
			    putchar(c);
			break;
	    case ZERO:	if (isdigit(c) || c=='.') {
			    putchar(c);
			    state = NUM;
			} else if ( c == 'x' || c == 'X' ) {
			    scanf("%x",&num);
			    printf("x%x",num);
			    state = NORMAL;
			} else if ( c == 'b' || c == 'B' )
			    state = BIN;
			else {
			    ungetch(c);
			    state = NORMAL;
			}
			break;
	    case NUM:	if (isdigit(c) || c=='.')
			    putchar(c);
			else {
			    ungetch(c);
			    state = NORMAL;
			}
			break;
	    case BIN:	if (isdigit(c))
			    num = (num << 1) + ( c - '0' );
			else {
			    printf("%o",num);
			    ungetch(c);
			    state = NORMAL;
			}
			break;
	    case STR:	putchar(c);
			switch(c) {
			    case '"':
			    case '\'':	if (oc == c)
					    state = NORMAL;
					else
					    state = STR;
					break;
			    case '\\':	state = STRESC;
			    default:
					break;
			}
			break;
	    case STRESC:putchar(c);
			state = STR;
			break;
	    case SLASH: putchar(c);
			if (c=='*')
			    state = COMM;
			else
			    state = NORMAL;
			break;
	    case COMM:	putchar(c);
			if (c=='*')
			    state = STAR;
			break;
	    case STAR:	putchar(c);
			if (c=='/')
			    state = NORMAL;
			else
			    state = COMM;
			break;
	    case WORD:	if (isalpha(c) || isdigit(c) || c=='_')
			    putchar(c);
			else {
			    ungetch(c);
			    state = NORMAL;
			}
			break;
	    default:
			break;
	}
    }
    return(0);
}
