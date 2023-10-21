
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * The output function for babble. Well, there has to be a special control
 * character for newlines, spaces, and tabs... This is %. %n, %t, %s.
 */

#include <stdio.h>

extern int nflag;

#define vowel(c)	((c == 'a') || (c == 'e') || (c == 'i') || (c == 'o') \
				    || (c == 'u') || (c == 'A') || (c == 'E') \
				    || (c == 'I') || (c == 'O') || (c == 'U'))
#define wspace(c)	((c == ' ') || (c == '\n') || (c == '\t'))

int lpos = 0;

kkoutput(string)
char *string;
{

	static char text[BUFSIZ];
	static int tpos = 0;
	int a, b;
	int vow, x;
	char c;

	/* Save up text until it is time to write out the stuff. */

	if (*string != '@') {
		while (*string)
			text[tpos++] = *(string++);
		text[tpos++] = ' ';
		return;
	} else {
		a = 0;
		if (text[0] >= 'a' && text[0] <= 'z')
			text[0] = text[0] - 'a' + 'A';
		if (lpos > 64) {
			putchar('\n');
			lpos = 0;
		}
loop:
		if (a >= tpos) {
			tpos = 0;
			return;
		}
		if (text[a] != ' ' && text[a] != '\t' && text[a] != '!' 
		    && text[a] != '\n' && text[a] != ',' && text[a] != '.'
		    && text[a] != '?') {
			if ((text[a] == 'a') && ((a == 0) || wspace(text[a - 1]))
			    && a < tpos && wspace(text[a + 1])) {
				for (x = a + 1; x < tpos; x++)
					if (!wspace(text[x]))
						break;
				if ((x != tpos) && vowel(text[x])) {
					putchar('a');
					putchar('n');
					a++;
					lpos += 2;
					goto loop;
				}
			}
			if (text[a] == '#') {
				a++;
				goto loop;
			}
			if (text[a] == '%') {
				switch(text[++a]) {
					case 'n': {
						putchar('\n');
						lpos = 0;
						a++;
						break;
					}
					case 's': {
						putchar(' ');
						lpos++;
						a++;
						break;
					}
					case 't': {
						
						putchar('\t');
						lpos = (lpos / 8 + 1) * 8;
						a++;
						break;
					}
					default: {
						putchar(text[a++]);
						lpos++;
						break;
					}
				}
				goto loop;
			}
			putchar(text[a]);
			lpos++;
			a++;
			goto loop;
		}
		if (text[a] == '.' || text[a] == ',' || text[a] == '!' 
		    || text[a] == '?') {
			putchar(text[a++]);
			if (*(string + 1) == 'P') {
				putchar('\n');
				putchar('\t');
				lpos = 8;
				goto loop;
			}
			while (text[a] == ' ' || text[a] == '\t' || 
			    text[a] == '\n')
				a++;
			goto space;
		}
		for (b = a; b < tpos; b++) {
			if (text[b] != ' ' && text[b] != '\t' && 
			    text[b] != '\n') 
				break;
		}
		if (text[b] == '.' || text[b] == ',' || text[b] == '!'
		    || text[b] == '?') {
			a = b;
			goto loop;
		}
space:
		for (x = a + 1; x < tpos; x++) 
			if (!wspace(text[x])) {
				if (text[x] == '#') {
					a = x + 1;
					goto loop;
				}
				break;
			}
		if ((lpos > 64) && !nflag) {
			if (tpos > a) {
				putchar('\n');
				lpos = 0;
			} else {
				/*
				tpos = 1;
				lpos = 0;
				text[0] = '\n';
				return;
				*/
			}
		} else {
			putchar(' ');
			lpos++;
		}
		while (text[a] == ' ' || text[a] == '\t' || text[a] == '\n')
			a++;
		goto loop;
	}
}

