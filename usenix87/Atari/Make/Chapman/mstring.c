/* mstring.c */

/* The purpose of this file is to provide subroutines for handling
   strings whose space is allocated with malloc - in this way we remove
   all limitations on length of strings */

#include "define.h"
#include "osbind.h"
#include "tosdefs.h"
#include "mstring.h"
#include "ctype.h"
#include "stdio.h"

#define DLEN 80

int linenumber = 0;

char *malloc();

char *mrealloc(p,newsize)
char *p;
int newsize;
{	char *pnew;

	pnew = malloc(newsize);
	if (pnew)
		strcpy(pnew,p);
	free(p);
	return pnew;
}


	char
lastchar(p)
	char *p;{
	char c=0;
	while (*p) c = *p++;
	return c;
	}

	char *
endptr(p)
	char *p;{
	while (*p) p++;
	return p;
	}


	mstring
mfgets (stream)
	FILE *stream;{
	mstring p; int plen;
	char *endptr();
	char *fgets();

	if (feof(stream)) return NULL;
	p = talloc(plen = DLEN);
	p[0] = '\0';
	while (1) {
		if (strlen(p) + DLEN > plen) {
			p = mrealloc(p, plen += DLEN);
			if (p==NULL) puts("no more memory (mfgets)"), exit(1);
			}
		{ char *tp, *tp2;
		tp2= endptr(p);
		tp= fgets(((BYTE *) tp2),((WORD )DLEN),stream);
		if ( (tp==NULL) || (feof(stream) ) ) {
			if (*p) return p;
			else {
				free(p);
				return NULL;
				}
		};
		}
		if (lastchar(p) != '\n') continue;
		linenumber++;
		endptr(p)[-1] = 0;
		if (lastchar(p) == '\\') {
			endptr(p)[-1] = 0;
			continue;
			}
		break;
		}
p = mrealloc (p,strlen(p)+1) ;
if (p==NULL) puts("no more memory (mfgets)"), exit(1);
return p;

}

	mstring
msubstr(p,i,l)  /* creates a string from p[i],p[i+1],...,p[i+l-1] */
	mstring p;{
	mstring q;
	q = talloc(l+1);
	strncpy(q,p+i,l);
	q[l] = '\0';
	return q;
	}

	mstring
mstrcat(p,q)
	mstring p,q;{
	mstring r = talloc (strlen(p) + strlen(q) + 1);
	strcpy(r,p);
	strcat(r,q);
	return r;
	}

	mstring
strperm(s)
	char *s;{       /* allocate space for s, return new pointer */
	char *t;
	t = (char *) talloc(strlen(s)+1);
	strcpy(t,s);
	return ((mstring ) t);
	}

passpace(p)
	char **p;{
	while (isspace (**p)) (*p)++;
	}

passnonsp(p)
	char **p;{
	while (**p && !isspace(**p)) (*p)++;
	}

password(p)
	char **p;{
	while (isalnum(**p)) (*p)++;
	}

error (errmsg,a,b,c,d,e,f,g,h)
char *errmsg;long a,b,c,d,e,f,g,h;
{
    /* unfortunately, this assumes only one file is being used */
    if (linenumber) fprintf(stderr,"at line %d : ",linenumber);
    fprintf(stderr,errmsg,a,b,c,d,e,f,g,h);
    exit (1);
}

	mstring
talloc(i)
	int i;{
	char *p;
	p = malloc(i);
	if (p==NULL) error ("no more memory");
	return p;
	}
