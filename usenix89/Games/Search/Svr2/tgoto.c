#define	CTRL(c)	('c' & 037)

#include "defines.h"
#include "structs.h"
char *_branchto();

/*
 * Routine to perform parameter substitution.
 * p->CM is a string containing printf type escapes.
 * The whole thing uses a stack, much like an HP 35.
 * The following escapes are defined for substituting row/column:
 *
 *	%d	print pop() as in printf
 *	%[0]2d	print pop() like %[0]2d
 *	%[0]3d	print pop() like %[0]3d
 *	%c	print pop() like %c
 *	%s	print pop() like %s
 *	%l	pop() a string and push its length.
 *
 *	%p[1-0]	push ith parm
 *	%P[a-z] set variable
 *	%g[a-z] get variable
 *	%'c'	char constant c
 *	%{nn}	integer constant nn
 *
 *	%+ %- %* %/ %m		arithmetic (%m is mod): push(pop() op pop())
 *	%& %| %^		bit operations:		push(pop() op pop())
 *	%= %> %<		logical operations:	push(pop() op pop())
 *	%! %~			unary operations	push(op pop())
 *	%b			unary BCD conversion
 *	%d			unary Delta Data conversion
 *
 *	%? expr %t thenpart %e elsepart %;
 *				if-then-else, %e elsepart is optional.
 *				else-if's are possible ala Algol 68:
 *				%? c1 %t %e c2 %t %e c3 %t %e c4 %t %e %;
 *
 * all other characters are ``self-inserting''.  %% gets % output.
 */

#define push(i) (stack[++top] = (i))
#define pop()	(stack[top--])

char *
tgoto (p, p2, p1)
	t_player *p;
	int p2, p1;
{
	static char result[32];
	static char added[10];
	int vars[26];
	int stack[10], top = 0;
	register char *cp = p->CM;
	register char *outp = result;
	register int c;
	register int op;
	int sign;
	int onrow = 0;
	int leadzero = 0;	/* not having leading zero is unimplemented */
	char *xp;

	if (cp == 0)
		return ("OOPS: NULL ARG");
	added[0] = 0;

	while (c = *cp++) {
		if (c != '%') {
			*outp++ = c;
			continue;
		}
		op = stack[top];
		if (*cp == '0') {
			leadzero = 1;
			cp++;
		}
		switch (c = *cp++) {

		/* PRINTING CASES */
		case 'd':
			if (op < 10)
				goto one;
			if (op < 100)
				goto two;
			/* fall into... */

		case '3':
three:
			if (c == '3' && *cp++ != 'd')
				return ("OOPS: BAD CHAR AFTER %3");
			*outp++ = (op / 100) | '0';
			op %= 100;
			/* fall into... */

		case '2':
			if (op >= 100)
				goto three;
			if (c == '2' && *cp++ != 'd')
				return ("OOPS: BAD CHAR AFTER %2");
two:	
			*outp++ = op / 10 | '0';
one:
			*outp++ = op % 10 | '0';
			(void) pop();
			continue;

		case 'c':
			/*
			 * Weird things can happen to nulls, EOT's, tabs, and
			 * newlines by the tty driver, arpanet, etc.
			 * So we instead we used to alter the place being
			 * addessed and then move the cursor locally using UP
			 * or RIGHT, before Tore made som changes.
			 */
			switch (op) {
			case 0:
				op = 0200;
				break;
			}

			*outp++ = op;
			(void) pop();
			break;

		case 'l':
			xp = (char *) pop();
			push(strlen(xp));
			break;

		case 's':
			xp = (char *) pop();
			while (*xp)
				*outp++ = *xp++;
			break;

		case '%':
			*outp++ = c;
			break;

		/*
		 * %i: shorthand for increment first two parms.
		 * Useful for terminals that start numbering from
		 * one instead of zero (like ANSI terminals).
		 */
		case 'i':
			p1++; p2++;
			break;

		/* %pi: push the ith parameter */
		case 'p':
			switch (c = *cp++) {
			case '1': push(p1); break;
			case '2': push(p2); break;
			default: return ("OOPS: BAD PARM NUMBER");
			}
			onrow = (c == '1');
			break;
		
		/* %Pi: pop from stack into variable i (a-z) */
		case 'P':
			vars[*cp++ - 'a'] = pop();
			break;
		
		/* %gi: push variable i (a-z) */
		case 'g':
			push(vars[*cp++ - 'a']);
			break;
		
		/* %'c' : character constant */
		case '\'':
			push(*cp++);
			if (*cp++ != '\'')
				return ("OOPS: MISSING CLOSING QUOTE");
			break;
		
		/* %{nn} : integer constant.  */
		case '{':
			op = 0; sign = 1;
			if (*cp == '-') {
				sign = -1;
				cp++;
			} else if (*cp == '+')
				cp++;
			while ((c = *cp++) >= '0' && c <= '9') {
				op = 10*op + c - '0';
			}
			if (c != '}')
				return ("OOPS: MISSING CLOSING BRACE");
			push(sign * op);
			break;
		
		/* binary operators */
		case '+': c=pop(); op=pop(); push(op + c); break;
		case '-': c=pop(); op=pop(); push(op - c); break;
		case '*': c=pop(); op=pop(); push(op * c); break;
		case '/': c=pop(); op=pop(); push(op / c); break;
		case 'm': c=pop(); op=pop(); push(op % c); break; /* %m: mod */
		case '&': c=pop(); op=pop(); push(op & c); break;
		case '|': c=pop(); op=pop(); push(op | c); break;
		case '^': c=pop(); op=pop(); push(op ^ c); break;
		case '=': c=pop(); op=pop(); push(op = c); break;
		case '>': c=pop(); op=pop(); push(op > c); break;
		case '<': c=pop(); op=pop(); push(op < c); break;

		/* Unary operators. */
		case '!': stack[top] = !stack[top]; break;
		case '~': stack[top] = ~stack[top]; break;
		/* Sorry, no unary minus, because minus is binary. */

		/*
		 * If-then-else.  Implemented by a low level hack of
		 * skipping forward until the match is found, counting
		 * nested if-then-elses.
		 */
		case '?':	/* IF - just a marker */
			break;

		case 't':	/* THEN - branch if false */
			if (!pop())
				cp = _branchto(cp, 'e');
			break;

		case 'e':	/* ELSE - branch to ENDIF */
			cp = _branchto(cp, ';');
			break;

		case ';':	/* ENDIF - just a marker */
			break;

		default:
			return ("OOPS: BAD % SEQUENCE");
		}
	}
	strcpy(outp, added);
	return (result);
}


char *
_branchto(cp, to)
register char *cp;
char to;
{
	register int level = 0;
	register char c;

	while (c = *cp++) {
		if (c == '%') {
			if ((c = *cp++) == to || c == ';') {
				if (level == 0) {
					return cp;
				}
			}
			if (c == '?')
				level++;
			if (c == ';')
				level--;
		}
	}
	return ("OOPS: NO MATCHING ENDIF");
}
