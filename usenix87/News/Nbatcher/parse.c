/*
 *
 *	parse.c - nbatcher line parser for the control file
 *
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include "nbatcher.h"

#define MAX_BYTES	1000000L	/* max allowable bytes */

parse_entry (line)
char *line;
{
	register char	*p;
	short	num, upper;
	short	lower, dash;
	long	l_num;

	upper = 23;	/* upper hour limit */
	lower = 0;	/* lower hour limit */
	dash = 0;

	clear_entry (&ep);	/* zero out the structure */

	p = (char *) ep.site;

	/* get the site name and copy
	   it to the structure */

	while (*line && *line != COLON)
		*p++ = *line++;
	*p = '\0';
	if (*++line == '\n' || *line == '\0')
		xerror ("illegal number of fields\n");

	/* check that its valid */

	if (ep.site[0] == '\0')
		xerror ("null site name in control file\n");

	/* now, parse the hour string and check
	   for valid syntax */

	p = (char *) ep.hour;
	while (*line && *line != COLON)
		*p++ = *line++;

	*p = '\0';
	if (*++line == '\n' || *line == '\0')
		xerror ("illegal number of fields\n");

	if (ep.hour[0] == '\0')
		xerror ("null hour string in control file\n");

	/* now re-scan the hour in structure and
	   weed out the badies */

	if (ep.hour[0] == '*' && ep.hour[1] != '\0')
		xerror ("invalid hour string syntax: %s\n", ep.hour);
	else if (ep.hour[0] == '*')
		goto h_skip;

	if (strcmp(ep.hour, "off", 3) == 0 && ep.hour[3] != '\0')
		xerror ("invalid hour string syntax: %s\n", ep.hour);
	else if (strncmp(ep.hour, "off", 3) == 0)
		goto h_skip;

	p = (char *) ep.hour;
	if (!isdigit(*p))
		xerror ("non-numeric char in hour string: %c\n", *p);

	while (*p) {
		num = 0;
		do {
			num = num*10 + (*p - '0');
		} while (isdigit(*++p));

		if (num < lower || num > upper)
			xerror ("illegal hour: %d\n", num);

		if (!*p)
			break;

		if (*p == '-' && dash)
			xerror ("syntax error in hour field\n");
		else if (*p == '-')
			dash = TRUE;

		if (*p != ',' && *p != '-')
			xerror ("non-numeric char in hour string: %c\n", *p);
		else if (!isdigit(*++p))
			xerror ("syntax error in hour field\n");

	}

	/* now that thats over with, let do the compression
	   field.  Only 9-16 is allowed, except a null field
	   indicates no compression for this site. */

h_skip:
	num = 0;
	while (*line && *line != COLON) {
		if (!isdigit(*line))
			xerror ("non-numeric in compression field\n");
		num = num*10 + (*line++ - '0');
	}
	if (*++line == '\n' || *line == '\0')
		xerror ("illegal number of fields\n");

	if (num != 0 && (num < 9 || num > 16))
		xerror ("illegal compression bits: %d\n", num);

	ep.c_bits = num;

	/* now check the max. bytes for UUCP queue.
	   Note: There is a max. allowable # of bytes
		 here, set at 1MB.  Change it at your
		 own risk.
	*/

	l_num = 0;
	while (*line && *line != COLON) {
		if (!isdigit(*line))
			xerror ("non-numeric in max. bytes field\n");

		l_num = l_num*10 + (*line++ - '0');
	}

	if (l_num > MAX_BYTES)
		xerror ("%ld max. bytes exceeds allowable maximun\n", l_num);

	if (l_num != 0)
		ep.m_bytes = l_num;
	else
		ep.m_bytes = DFL_BYTES;

	/* and finally the command line (if there is one) */

	p = (char *) ep.command;

	if (*++line != '\n' && *line != '\0') {
		while (*line && *line != '\n')
			*p++ = *line++;

		*p = '\0';
	}
}

#ifdef USE_PORT_CODE
xerror (fmt, a1, a2)
char *fmt;
char *a1, *a2;
{
	char	buf[BUFSIZ];

	sprintf (buf, fmt, a1, a2);
	printf ("nbatcher: %s\n", fmt);
	exit (99);
}

#else
xerror (fmt, argp)
char *fmt;
int argp;
{
	char	buf[BUFSIZ];
	char	fbuf[BUFSIZ];
	FILE	prwbuf;
	register char	*cp;
	
	prwbuf._flag = _IOWRT;
	prwbuf._file = _NFILE;
	prwbuf._cnt = 32767;
	prwbuf._ptr = (unsigned char *)buf;
	prwbuf._base = (unsigned char *)buf;
	sprintf (fbuf, "%s: %s", "nbatcher", fmt);
	_doprnt (fbuf, (char *)&argp, &prwbuf);
	putc ('\0', &prwbuf);
	for (cp = buf; *cp != '\0'; cp++)
		putchar (*cp);

	exit (99);
}
#endif	/* USE_PORT_CODE */

clear_entry (s)
char *s;
{
	register int i;

	for (i=0; i<sizeof(struct file_entry); *s++ = '\0', i++)
				;

}
