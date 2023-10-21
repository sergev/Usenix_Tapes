
#include "hd.h"
#include "strings.h"
#include "command.h"

/* Interface to general execute */

#define	REXEC	0

xecute(f)
char **f;
{
	static char inline[STRMAX];
	char stmp[STRMAX];
	register char *s, *t, *u;
	int ret;
	register int argcnt, i;
	char **v;
	char lastc;
	char c1, c2;

	tty_push(COOKEDMODE);
	clearmsg(2);
	ewin();
	lastc = 1;
	ret = NOREPLOT;
	if (f != REXEC) {
		v = f;
		if (*v != CNULL) {
			for (s = inline; *v; v++) {
				t = *v;
				while (*s++ = *t++);
				s[-1] = ' ';
			}
			*s = 0;
		}
		else {
again:
			printf("\r");
			hilite("Command:");
			printf(" ");
			c1 = inline[0];
			c2 = inline[1];
			getline(inline);
			if (*inline == 0) {
				lastc = 0;
				inline[0] = c1;
				inline[1] = c2;
			}
		}
	}
	else if (*inline == 0) {
		printf("\r");
		hilite("No command");
	}
	if (lastc && *inline) {
		if (VSHMODE == SELECTMODE && selecttype == DIRCMD) {
			s = stmp;
			t = inline;
			u = 0;
			argcnt = 0;
			while (*t) {
				if (*t == '$') {
					if (t[1] == ' ' || t[1] == 0) {
						i = 0;
						if (*selectname) {
							u = selectname;
							while (*u)
								*s++ = *u++;
							t++;
							i++;
						}
						else if (!NOARG) {
							if (argcnt == 0)
								hilite("(%s)\n", inline);
							hilite("Arg%d:", ++argcnt);
							putch(' ');
							i = xgetline(stdin, s, (&stmp[STRMAX]-s));
							if (i) {
								s += i;
								s[0] = ' ';
								t++;
							}
						}
						if (i == 0) {
							hilite("(Missing argument)\r\n");
							t++;
						}
						continue;
					}
				}
				*s++ = *t++;
			}
			*s = 0;
			s = t = stmp;
			if (*s == ';')
				s++;
			if (u || f == REXEC) {
				printf("\r");
				hilite("Command: %s\r\n", s);
			}
			system(s);
		}
		else {
			s = t = inline;
			if (*s == ';')
				s++;
    			if (f == REXEC) {
				printf("\r");
				hilite("Command: %s\r\n", s);
			}
			system(s);
		}
		if (*t != ';') {
			f = (char **)!REXEC;
			ret = REPLOT;
			goto again;
		}
		tty_pop();
		return REPLOT;
	}
	else {
		vwin();
		sleep(1);
		clearmsg(1);
		tty_pop();
	}
	return ret;
}

rexecute()
{
	return xecute(REXEC);
}

