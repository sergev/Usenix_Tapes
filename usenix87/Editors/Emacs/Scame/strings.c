/*	SCAME strings.c			*/

/*	Revision 1.0.0  1985-02-09	*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

# include "scame.h"

Bool strneq(s1,s2,i,swedish)		/* Are s1 and s2 equal? */
register char *s1,*s2;
register int i;
Bool swedish;
{
	while(i-- > 0
	      && upcase(*s1,swedish)==upcase(*s2,swedish)) { s1++; s2++; }
	return (i == -1);
}

int strsub(s1,s2,swedish,anywhere)	/* Is s2 a substring of s1? */
char *s1,*s2;
Bool swedish, anywhere;
{
register char *t1, *t2, *tt1, *tt2;

	tt1 = s1;
	tt2 = s2;
	do {
		t1=tt1;
		t2=tt2;
		while(upcase(*t1,swedish) == upcase(*t2,swedish)) {
			if (*t1 == '\0') return(tt1-s1);
			*t1++; *t2++;
			}
		if (*t2 == '\0') return(tt1-s1);
	} while (anywhere && *(++tt1) != '\0');
	return (-1);
}

Bool isblank(c)
char c;
{
return (c == ' ' || c == '\t');
}

int strsize(str)
char *str;
{
register int i=0;
	while (*str != '\0') {
		i = i + 1 + (*str < ' ' || *str > '~');
		str++;
	}
	return (i);
}

Bool endofquantp(q,p)
enum quantity_t q;
char *p;
/* Is *p outside of quantity? */
{
char c;
	c = *p;
	switch (q) {
	case WORD:
		switch (cb.majormode) {
		case LISP:
			return((Bool) (c < '!'
					|| c > '~'
					|| index("()[]';", c)));
		case SWEDISH:
			return((Bool) (c < '0'
					|| (c > '9' && c < 'A')
					|| (c > ']' && c < 'a')
					|| c > '}'));
		default:
			return((Bool) (c < '0'
					|| (c > '9' && c < 'A')
					|| (c > 'Z' && c < 'a')
					|| c > 'z'));
		} break;
	case LINE:
		return((Bool) (p > buf && *(p-1) != '\n')); 
	default:
		return(FALSE);
	}
}

basename(file,base,removext)
char *file, *base;
Bool removext;
{
char *cp;
char tstr[FILENAMESIZE];
	strcpy(tstr,(cp= rindex(file,'/')) == NIL? file: &cp[1]);
	if (removext && (cp = rindex(tstr,'.')) != NIL && cp != tstr)
 		*cp = '\0';
	strcpy(base,tstr);
}

#ifndef BSD42
char *index(str, c)
char *str, c;
{
	while(*str && *str != c) str++;
	if (*str) return(str);
	else return(NIL);
}

char *rindex(str, c)
char *str, c;
{
register char *tp;
	if (*str) {
		tp = str + strlen(str) - 1;
		while (tp >= str && *tp != c) tp--;
		if (tp >= str) return(tp);
		else return(NIL);	
	}
	else return(NIL);
}
#endif

initupcase()
{
register int i;
	for (i=0; i < 512; i++) {
		upcasearr[i] = nupcasarr[i] = ((i & 0177) >= 'a'
					&& (i & 0177) <= 'z') ? (i & 0737) : i;
#ifdef COMMENT
		locasearr[i] = ((i & 0177) >= 'A'
					&& (i & 0177) <= 'Z') ? (i | 0040) : i;
#endif
	}
}
