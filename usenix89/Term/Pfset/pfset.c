/*
 * pfset - set programable function keys
 *
 * Pfset reads the file .pf${TERM} in a users HOME directory for
 * definitions on setting the function keys to the user's preference.
 * This was written to work on Teletype 4425 terminals but should be
 * easily modifiable for other terminal types.
 *
 * Compile with:
 *	cc -O -o pfset pfset.c
 *
 * AUTHOR
 *	Edward C. Bennett (edward@ukecc)
 *
 * Copyright 1985 by Edward C. Bennett
 *
 * Permission is given to alter this code as needed to adapt it to forign
 * systems provided that this header is included and that the original
 * author's name is preserved.
 */
#include <stdio.h>

#define	SYS_PF	0
#define	USER_PF	1

#define	KEYED	0
#define	LOCAL	1
#define	SEND	2

struct	pfent {
	char	*pf_key;
	char	*pf_action;
	char	*pf_label;
	char	*pf_command;
} pfent;

char	buf[BUFSIZ];

/*ARGSUSED*/
main(argc, argv)
int argc;
char **argv;
{
	register	char	*p;
	FILE	*fp;
	char	*getenv(), *pfskip(), *strcat();
	int	i;
	void	exit();

	if ((p = getenv("HOME")) == NULL) {
		fprintf(stderr, "%s: HOME variable not found\n", argv[0]);
		exit(1);
	}
	sprintf(buf, "%s/.pf", p);

	if ((p = getenv("TERM")) == NULL || *p == NULL) {
		fprintf(stderr, "%s: TERM variable not found\n", argv[0]);
		exit(1);
	}
	strcat(buf, p);

	if ((fp = fopen(buf, "r")) == NULL)
		exit(0);

	while (fgets(buf, BUFSIZ, fp)) {
		p = buf;

		pfent.pf_key = p;
		p = pfskip(p);
		pfent.pf_action = p;
		p = pfskip(p);
		pfent.pf_label = p;
		p = pfskip(p);
		pfent.pf_command = p;
		while (*p && *p != '\n')
			p++;
		*p = '\0';

		De_escape(pfent.pf_command);

		/*
		 * Print the key number
		 */
		printf("\033[%d;", *(pfent.pf_key+1)-'0');

		/*
		 * Print the command length
		 */
		printf("%d;", strlen(pfent.pf_command));

		/*
		 * Print the key action
		 */
		if (!strcmp(pfent.pf_action, "keyed"))
			i = KEYED;
		else if (!strcmp(pfent.pf_action, "local"))
			i = LOCAL;
		else
			i = SEND;
		printf("%d;", i);

		/*
		 * Print the key type
		 */
		printf("%dq", (*p == 's' ? SYS_PF : USER_PF));

		/*
		 * Print the label
		 */
		printf("   f%d   %-8.8s", *(pfent.pf_key+1)-'0', pfent.pf_label);

		/*
		 * Print the command
		 */
		printf("%s", pfent.pf_command);

		i++;
	}
	/*
	 * Show the USER PF labels
	 */
	printf("\033}");

	return(0);
}

/*
 * pfskip - isolate and skip a field
 *
 * q = pfskip(p);
 *	p	is a pointer to a character string
 *	q	is a ponter to the next field in p
 *
 * pfskip advances along the given string watching for NULLs and ':'s.
 * If it sees a ':', it changes it to a NULL and returns the NEXT
 * character position. If it finds a NULL, it stops.
 */
char *
pfskip(p)
register char *p;
{
	while (*p && *p != ':')
		++p;

	if (*p)
		*p++ = '\0';

	return(p);
}

/*
 *
 * De_escape - reduce things like '\n'
 *
 * De_escape(p)
 *	p	is a pointer to the string to be worked on
 *
 * Escapes like '\n' are converted to their real characters.
 */
De_escape(s)
char *s;
{
	char	*t;

	t = s;
	do {
		if (*s == '\\') {
			switch (*++s) {
			case 'n':
				*s = '\015';
				break;
			}
		}
	} while (*t++ = *s++);
}
