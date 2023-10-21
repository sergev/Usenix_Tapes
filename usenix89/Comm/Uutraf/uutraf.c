/*
 *	uutraf - (formerly uxfst) a program for accumulating bytes
 *	         snet/received over the UUCP network.
 *
 *	compile:  cc -O -s uutraf.c -o uutraf
 *
 *
 *	Uutraf will accept one argument, a file containing xferstat
 *	formatted info, otherwise uutraf uses /usr/spool/uucp/.Admin/xferstat
 *	as its input.
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>

#define SYSLOG		"/usr/spool/uucp/.Admin/xferstats"

FILE	*fp;

char	*tfile,
	*mktemp();

main (argc, argv)
char **argv;
{
	char	buf[BUFSIZ], last_sys[20];
	char	xf_file[80];
	float	t_secs;
	long	in_cnt, out_cnt;
	long	num;
	register char	*p, *p1;
	int	s_ret;


	if (argc == 2)
		strcpy (xf_file, argv[1]);
	else
		strcpy (xf_file, SYSLOG);

	tfile = mktemp("/tmp/ustXXXXXX");
	sprintf (buf, "sort %s -o %s", xf_file, tfile);
	s_ret = system(buf);

	if (s_ret != 0) {
		fprintf (stderr, "system(%s) failed\n",buf);
		exit (1);
	}

	if ((fp=fopen(tfile, "r")) == NULL) {
		fprintf (stderr, "can't fopen %s\n", tfile);
		exit (1);
	}

	if ((fgets(buf, sizeof(buf), fp)) == NULL)
		exit (0);

	p = (char *) buf;
	p1 = (char *) last_sys;

	while (*p && *p != '!')
		*p1++ = *p++;
	*p1 = '\0';
	
	rewind (fp);

	in_cnt = out_cnt = 0;
	while ((fgets(buf, sizeof(buf), fp)) != NULL) {
		if (strncmp(buf, last_sys, strlen(last_sys))) {
			dump (last_sys, in_cnt, out_cnt);
			p = (char *) buf;
			p1 = (char *) last_sys;
			while (*p && *p != '!')
				*p1++ = *p++;
			*p1 = '\0';
			in_cnt = out_cnt = 0;
		}
		p = (char *) buf;
		while (*p) {
			if (*p == '>') {
				for(s_ret=0; s_ret<2; s_ret++, p++)
						;
				num = 0;
				do {
					num = num*10 + (*p - '0');
				} while (isdigit(*++p));
				out_cnt += num;
				continue;
			} else if (*p == '<') {
				for (s_ret=0; s_ret<3; s_ret++, p++)
						;
				num = 0;
				do {
					num = num*10 + (*p - '0');
				} while (isdigit(*++p));
				in_cnt += num;
				continue;
			}
			p++;
		}
	}
	dump (last_sys, in_cnt, out_cnt);
	fclose (fp);
	unlink (tfile);
	exit (0);
}

dump (s, in, out)
char *s;
long in, out;
{
	static short first = 0;

	if (!first) {
		printf ("%-12s%12s%16s\n","System","Bytes Sent","Bytes Received");
		printf ("%-12s%12s%16s\n","------","----------","--------------");
		first = 1;
	}

	printf ("%-12s%12ld%16ld\n", s, out, in);
}
