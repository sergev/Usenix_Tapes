/*
 * regcomp(), regexec(), and xread() were posted to mod.sources by
 * henry spencer, and are too large to be included here.
 *
 * This file contains strcspn, strchr, strpbrk, and getopt.
 */

#include <stdio.h>
/*
 * strcspn - find length of initial segment of s1 consisting entirely
 * of characters not from s2
 */

int strcspn(s1, s2)
char *s1;
char *s2;
{
	register char *scan1;
	register char *scan2;
	register int count;

	count = 0;
	for (scan1 = s1; *scan1 != '\0'; scan1++) {
		for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
			if (*scan1 == *scan2++)
				return(count);
		count++;
	}
	return(count);
}

char *strchr(a, b)
char *a, b;
    {
    char *index();
    return (index(a, b));
    }

/* strpbrk - Returns a pointer to the first character of source that is any
 *  of the specified keys, or NULL if none of the keys are present
 *  in the source string.
 */
char *strpbrk(source, keys)
char *source, *keys;
{
        register int loc = 0, key_index = 0;
 
        while (source[loc] != '\0') {
          key_index = 0;
          while (keys[key_index] != '\0')
            if (keys[key_index++] == source[loc])
              return((char *) (source + loc));
          loc++;
        }
        
        return(NULL);
}
 

/*
 * getopt - get option letter from argument vector
 */
int	opterr = 1,		/* useless, never set or used */
	optind = 1,		/* index into parent argv vector */
	optopt;			/* character checked for validity */
char	*optarg;		/* argument associated with option */

#define BADCH	(int)'?'
#define EMSG	""
#define tell(s)	fputs(*nargv,stderr);fputs(s,stderr); \
		fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);

getopt(nargc,nargv,ostr)
int	nargc;
char	**nargv,
	*ostr;
{
	static char	*place = EMSG;	/* option letter processing */
	register char	*oli;		/* option letter list index */
	char	*index();

	if(!*place) {			/* update scanning pointer */
		if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) return(EOF);
		if (*place == '-') {	/* found "--" */
			++optind;
			return(EOF);
		}
	}				/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' || !(oli = index(ostr,optopt))) {
		if(!*place) ++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ':') {		/* don't need argument */
		optarg = NULL;
		if (!*place) ++optind;
	}
	else {				/* need an argument */
		if (*place) optarg = place;	/* no white space */
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		}
	 	else optarg = nargv[optind];	/* white space */
		place = EMSG;
		++optind;
	}
	return(optopt);			/* dump back option letter */
}

