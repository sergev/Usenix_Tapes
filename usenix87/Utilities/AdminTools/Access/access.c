#include <stdio.h>
#include "regexp.h"
/*
 * access (userid) [command]
 *
 * Scans /etc/accesstab. Accesstab format
 *
 *	regex1:regex2
 *
 * If the login name of the invoker matches regex1, then a regsub is
 * performed on regex2, and if the requested "(userid)" matches the
 * resulting string, then it does a setgid to the gid of the requested
 * "(userid)", a setuid to the appropriate userid, then exec's /bin/sh.
 * It also changes directory to the home directory of the requested userid.
 * [command], if specified, is passed to sh(1).
 */

char *progname;
static char **shargv;
static char *nope = "permission denied";
static char *tablename = "/etc/accesstab";

#ifndef ERR
#define ERR (-1)
#endif

main(argc, argv)
int argc;
char **argv;
{
	char *source,
		 *dest,
		 *getlogin();

	progname = argv[0];

	if (argc < 2)
		syntax();

	dest = argv[1];
	shargv = argv+1;

	if ((source = getlogin()) == NULL)
		error("Who are you?!");

	if (check(source, dest))
		doit(dest);
	else
		error(nope);
}

syntax()
{
	fprintf(stderr, "syntax: %s username [command]\n", progname);
	exit(1);
}

error(s)
char *s;
{
	fprintf(stderr, "%s: %s\n", progname, s);
	exit(1);
}

FILE *efopen(name, mode)
char *name, *mode;
{
	FILE *fp, *fopen();
	if (fp = fopen(name, mode))
		return(fp);

	fprintf(stderr, "%s: cannot open %s\n", progname, name);
	exit(1);
}

check(s, d)
char *s, *d;
{
	FILE *fp, *efopen();
	char buffer[256], *ptr, destbuff[256], *strchr();
	regexp *r_src, *r_dest, *regcomp();

	fp = efopen(tablename, "r");
	while (fgets(buffer, 256, fp)) {
		if (strlen(buffer) < 3 || *buffer == '#' || !strchr(buffer, ':'))
			continue;
		while ((ptr=strchr(buffer, '\n')) || (ptr=strchr(buffer, '\r')))
			*ptr = '\0';
		if (!(ptr=strchr(buffer, ':')))
			error("parse error in accesstab");
		*ptr++ = '\0';
		r_src = regcomp(buffer);
		if (!regexec(r_src, s))
			continue;
		if (!*ptr)
			error("bad dest expression in accesstab");
		regsub(r_src, ptr, destbuff);
		r_dest = regcomp(destbuff);
		if (regexec(r_dest, d)) {
			fclose(fp);
			return(1);
		}
	}
	fclose(fp);
	return(0);
}

#include <pwd.h>

doit(name)
char *name;
{
	struct passwd *ptr, *getpwnam();

	if (!(ptr=getpwnam(name)))
		error("no such userid");
	endpwent();

	if (strcmp(name, ptr->pw_name) != 0)
		error("wrong passwd entry returned");

	if (chdir(ptr->pw_dir) == ERR)
		error("cannot chdir");

	if (setgid(ptr->pw_gid) == ERR)
		error("cannot setgid");

	if (setuid(ptr->pw_uid) == ERR)
		error("cannot setuid");

	shargv[0] = "sh";

	execv("/bin/sh", shargv);
	error("exec failed");	
}
