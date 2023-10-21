/*
 * tele - Search for telephone numbers and the like
 * g - Alternative GREP program with context and multiple patterns
 *	Daniel Ts'o, Rockefeller Univ.
 */

#include <stdio.h>
#include <pwd.h>

#define	PHONE1LIST "/phone"
#define	PHONE2LIST "/Phone"
#define	PHONEPUB "/usr/pub/phone"
#define	PHONELOCAL "/usr/pub/phone.local"
#define	PHONEMISC "/usr/pub/phone.misc"
#define	LINESIZE 512
#define	MAXPATTERNS 100

#define	CARAT	(0200+'^')
#define	DOT	(0200+'.')
#define	DOLLAR	(0200+'$')
#define	SPECIAL	0200

int aflag = 0;
int bflag = 0;
int cflag = 0;
int sflag = 0;
int tflag = 1;
int vflag = 0;
int pflag = 0;
int Cflag = 0;
int lflag = 0;
long hflag = 0;
long hits = 0;
int fileflag = 0;
int numberflag = 0;
long lineno = 0;
int fold = 1;
char **cv;
char *ealloc();
char *nodename;
char *getenv();
long getnum();

main(c, v)
char **v;
{
	register char *s;
	register int f, f1;
	int f2;
	char pname[LINESIZE];
	char obuf[BUFSIZ];
	char *logdir();
	char **files;
	char *PHONE;
	char **pp;
	char *patterns[MAXPATTERNS+2];

	/* Act like grep */
	nodename = v[0];
	if (*nodename == 'g')
		fold = tflag = 0;
	PHONE = getenv("PHONE");
	if (*PHONE == 0)
		PHONE = NULL;
	pp = patterns;
	while (c > 1 && v[1][0] == '-' && v[1][1]) {
		c--;
		v++;
		s = *v;
		for (;;) {
			if (*++s == 0)
				break;
			switch(*s) {
			/* After match */
			case 'a':
				aflag++;
			/* Before match */
			case 'b':
			case 'c':
				if (*s == 'b')
					bflag++;
				cflag = getnum(s);
				break;
			case 'h':
				hflag = getnum(s);
				break;
			case 'g':
				fold = tflag = 0;
				break;
			case 'l':
				lflag++;
				break;
			case 'C':
				Cflag++;
				break;
			case 'p':
				pflag++;
				break;
			case 's':
				sflag++;
				break;
			case 't':
				fold = tflag = 1;
				break;
			case 'F':
				fold = 0;
				break;
			case 'f':
				fold = 1;
				break;
			case 'n':
				numberflag = 1;
				break;
			case 'N':
				fileflag = 1;
				break;
			case 'v':
				vflag = 1;
				break;
			case 'e':
				if (pp >= &patterns[MAXPATTERNS]) {
					fprintf(stderr,
						"%s: Too many patterns\n",
						nodename);
					exit(-1);
				}
				if (s[1]) {
					*pp++ = s + 1;
					*s = 0;
					s--;
					break;
				}
				else if (c > 1) {
					c--;
					v++;
					*pp++ = *v;
					break;
				}	
			default:
				fprintf(stderr, "%s: %s: Bad option\n", *v,
					nodename);
				exit(-1);
			}
		}
	}
	if (c < 2) {
		if (tflag)
			fprintf(stderr, "Usage: %s names_and_patterns\n",
				nodename);
		else
			fprintf(stderr, "Usage: %s pattern [files]\n",
				nodename);
		exit(-1);
	}

	v[c] = 0;
	c--;
	if (!tflag) {
		files = v + 2;
		*pp++ = v[1];
		*pp = 0;
		v = patterns;
		if (c > 2)
			fileflag++;
		c = pp - patterns;
	}
	else
		v++;
	while (c-- > 0)
		special(v[c]);
	/* Set-up array of line buffers for context */
	if (cflag) {
		cv = (char **) ealloc(cflag*(sizeof (char *)));
		for (f = 0; f < cflag; f++)
			cv[f] = (char *)ealloc(LINESIZE);
	}

	setbuf(stdout, obuf);
	if (tflag) {
		strcpy(pname, logdir());
		strcat(pname, PHONE1LIST);
		if (PHONE != NULL) {
			f = f1 = phone(v, PHONE);
			f2 = phone(v, pname);
			if (f2 >= 0) {
				f += f2;
				f1 = 0;
			}
		}
		else
			f = f1 = phone(v, pname);
		strcpy(pname, logdir());
		strcat(pname, PHONE2LIST);
		f2 = phone(v, pname);
		if (f2 >= 0) {
			f += f2;
			f1 = 0;
		}
		f2 = phone(v, PHONELOCAL);
		if (f2 >= 0) {
			f += f2;
			f1 = 0;
		}
		f2 = phone(v, PHONEMISC);
		if (f2 >= 0) {
			f += f2;
			f1 = 0;
		}
		f2 = phone(v, PHONEPUB);
		if (f2 >= 0) {
			f += f2;
			f1 = 0;
		}
		if (f1 && !sflag)
			fprintf(stderr, "No phone list\n");
	}
	else if (*files == 0)
		f = phone(v, 0);
	else {
		f = 0;
		while (*files)
			f += phone(v, *files++);
	}
	if (Cflag)
		printf("%ld\n", hits);
	exit(f > 0 ? 0 : -1);
}

/*
 * Do case-folding of pattern and map SPECIALS
 *	SPECIALS have 0200 bit set. Conscious decision - don't strip 0200 off
 *	pattern (maybe someone will want it)
 */
special(pat)
register char *pat;
{
	register char *s;

	s = pat;
	while (*pat) {
		if (fold)
			if (*pat >= 'A' && *pat <= 'Z')
				*pat += 'a' - 'A';
		switch (*pat) {
		case '\\':
			if (pat[1])
				pat++;
			*s++ = *pat++;
			break;
		case '^':
			*s++ = CARAT;
			pat++;
			break;
		case '.':
			*s++ = DOT;
			pat++;
			break;
		case '$':
			*s++ = DOLLAR;
			pat++;
			break;
		default:
			*s++ = *pat++;
		}
	}
	*s = 0;
}

long getnum(s)
register char *s;
{
	long i;

	s++;
	if ('0' <= *s && *s <= '9') {
		i = atol(s);
		*s = 0;
		if (i <= 0)
			i = 1;
	}
	else
		i = 1;
	return i;
}

phone(v, fname)
char **v;
char *fname;
{
	register char **vv, *s, *t;
	register int i;
	int f;
	long ln;
	char buf[LINESIZE], sbuf[LINESIZE];
	FILE *pd;
	int ccnt, cindex, nomatch;

	f = 0;
	if (fname && strcmp("-", fname)) {
		pd = fopen(fname, "r");
		if (pd == NULL) {
			if (!tflag && !sflag)
				fprintf(stderr, "%s: %s: Cannot open\n",
					nodename, fname);
			return -1;
		}
	}
	else
		pd = stdin;
	cindex = 0;
	ccnt = 0;
	lineno = 0;
	if (cflag)
		for (i = 0; i < cflag; *cv[i++] = 0);
	while (fgets(buf, LINESIZE, pd) != NULL) {
		for (i = 0, s = buf, t = sbuf; *t++ = *s; i++, s++)
			if (fold)
				if (*s >= 'A' && *s <= 'Z')
					*s = *s - 'A' + 'a';
		lineno++;
		nomatch = 1;
		for (vv = v; *vv; vv++)
			if (match(*vv, buf, i) ^ vflag) {
				if (lflag) {
					printf("%s\n",
						(fname ? fname : "(stdin)"));
					fflush(stdout);
					if (pd != stdin)
						fclose(pd);
					return 1;
				}
				f++;
				hits++;
				nomatch = 0;
				if (cflag) {
					ccnt = cindex;
					ln = lineno - cflag;
					if (!aflag) do {
						output(fname, ln++, cv[cindex]);
						*cv[cindex] = 0;
						if (++cindex >= cflag)
							cindex = 0;
					} while (cindex != ccnt);
					ccnt = cflag + 1;
				}
				output(fname, lineno, sbuf);
				if (tflag)
					fflush(stdout);
				break;
			}
		if (cflag) {
			if (nomatch && ccnt && !bflag) {
				output(fname, lineno, sbuf);
				if (tflag)
					fflush(stdout);
				*cv[cindex] = 0;
			}
			else if (!nomatch)
				*cv[cindex] = 0;
			else {
				t = cv[cindex];
				s = sbuf;
				while (*t++ = *s++);
			}
			if (++cindex >= cflag)
				cindex = 0;
		}
		if (ccnt) {
			if (pflag && ccnt == 1 && *sbuf != '\n')
				ccnt = 2;
			ccnt--;
		}
		if (hflag && ccnt == 0 && hits >= hflag) {
			fflush(stdout);
			exit(0);
		}
	}
	if (pd != stdin)
		fclose(pd);
	fflush(stdout);
	return f;
}

output(fname, ln, s)
char *fname;
long ln;
char *s;
{
	if (s == 0 || *s == 0)
		return;
	if (Cflag)
		return;
	if (fileflag && fname)
		printf("%s:", fname);
	if (numberflag)
		printf("%ld:", ln);
	if (fputs(s, stdout) == EOF) {
		fprintf(stderr, "%s: Output error\n", nodename);
		exit(-1);
	}
}

match(a, b, m)
char *a, *b;
register int m;
{
	register char *s, *t;
	int n;
	int carat;

	if ((*a&0377) == CARAT) {
		a++;
		if (*a == 0)
			return 1;
		carat = 1;
	}
	else
		carat = 0;
	n = 0;
	s = a;
	if (*s == 0)
		return 0;
	while (*s++)
		n++;
	for (;;) {
		s = a;
		t = b;
		if ((*s&0377) != DOT)
			for (;;) {
				if (*t == 0)
					break;
				if (*t++ == *s) {
					t--;
					break;
				}
				m--;
			}
		if (carat && t != b)
			break;
		b = t;
		if (n > m)
			break;
		for (;;) {
			if (*t == '\n') {
				if ((*s&0377) == DOLLAR)
					return 1;
				break;
			}
			if ((*s&0377) == DOT) {
				s++;
				t++;
			}
			else if (*s++ != *t++)
				break;
			if (*s == 0)
				return 1;
		}
		if (carat)
			break;
		m--;
		b++;
	}
	return 0;
}

char *logdir()
{
	char *getenv();
	struct passwd *getpwuid();
	register char *hd;
	register struct passwd *p;

	hd = getenv("HOME");
	if (hd == NULL || *hd == 0) {
		p = getpwuid(getuid());
		hd = p->pw_dir;
	}
	if (hd == NULL || *hd == 0)
		hd = "/";
	return hd;
}

char *ealloc(n)
int n;
{
	register char *s;

	s = (char *)malloc(n);
	if (s == NULL) {
		fprintf(stderr, "malloc: %s: Out of memory\n", nodename);
		exit(-1);
	}
	return s;
}
