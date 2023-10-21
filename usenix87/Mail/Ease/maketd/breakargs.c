/* breakargs - break a string into a string vector for execv.
 * Returns NULL if can't malloc space for vector, else vector.
 * Note, when done with the vector, mearly "free" the vector.
 * Written by Stephen Uitti, PUCC, Nov '85 for the new version
 * of "popen" - "nshpopen", that doesn't use a shell.
 * (used here for the as filters, a newer option).
 *
 * breakargs is copyright (C) Purdue University, 1985
 *
 * put in a fix for cmds lines with "string string" in them
 * Mon Aug 25 13:34:27 EST 1986 (ksb)
 *
 * Permission is hereby given for its free reproduction and
 * modification for non-commercial purposes, provided that this
 * notice and all embedded copyright notices be retained.
 * Commercial organisations may give away copies as part of their
 * systems provided that they do so without charge, and that they
 * acknowledge the source of the software.
 */
#ifdef BSD2_9
#include <sys/types.h>
#endif
#include <stdio.h>			/* for nothing, really */
#define SPC '\040'			/* ascii space */

char *
mynext(pch)
register char *pch;
{
	register int fQuote;

	for (fQuote = 0; (*pch != '\000' && *pch != SPC && *pch != '\t')||fQuote; ++pch) {
		if ('\\' == *pch) {
			continue;
		}
		switch (fQuote) {
		default:
		case 0:
			if ('"' == *pch) {
				fQuote = 1;
			} else if ('\'' == *pch) {
				fQuote = 2;
			}
			break;
		case 1:
			if ('"' == *pch)
				fQuote = 0;
			break;
		case 2:
			if ('\'' == *pch)
				fQuote = 0;
			break;
		}
	}
	return pch;
}

char **
breakargs(cmd)
char *cmd;
{
	register char *p;		/* tmp				*/
	register char **v;		/* vector of commands returned	*/
	register unsigned sum;		/* bytes for malloc		*/
	register int i;			/* number of args		*/
	register char *s;		/* save old position		*/
	char *malloc(), *strcpy();

	p = cmd;
	while (*p == SPC || *p == '\t')
		p++;
	cmd = p;			/* no leading spaces		*/
	sum = sizeof(char *);
	i = 0;
	while (*p != '\0') {		/* space for argv[];		*/
		++i;
		s = p;
		p = mynext(p);
		sum += sizeof(char *) + 1 + (unsigned)(p - s);
		while (*p == SPC || *p == '\t')
			p++;
	}
	++i;
	/* vector starts at v, copy of string follows NULL pointer */
	v = (char **)malloc(sum+1);
	if (v == NULL)
		return (char **)NULL;
	p = (char *)v + i * sizeof(char *); /* after NULL pointer */
	i = 0;				/* word count, vector index */
	while (*cmd != '\0') {
		v[i++] = p;
		s = cmd;
		cmd = mynext(cmd);
		if (*cmd != '\0')
			*cmd++ = '\0';
		strcpy(p, s);
		p += strlen(p);
		++p;
		while (*cmd == SPC || *cmd == '\t')
			++cmd;
	}
	v[i] = (char *)NULL;
	return v;
}
