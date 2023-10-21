/* This program is designed to change the default login universe
 * of a user.  It requires to run setuid and handles checking of
 * user permissions (hopefully).
 *
 *    Only a user or root can change a the login universe
 *					F. Crawford - 31 Jul 84
 *	SCCS @(#)chuni.c	1.3	9/26/84
 */
#include <stdio.h>
#include <pwd.h>
#include <universe.h>
#include <sys/file.h>

#define UNIVERSE	"/etc/u_universe"	/* Universe file */
#define NOUNIV	(sizeof(univ_name) / sizeof(char *))
						/* Number of known universes */
#define UCB	2				/* Universe number of UCB */

char *getlogin();

void release (file)
    FILE *file;
    {
    flock(fileno(file), LOCK_UN);
    fclose(file);
    }

int main(argc, argv)
    int argc;
    char **argv;
    {
    register FILE *univ;
    register char *loginid;
    register int i;
    char buf[20], name[20];
    int len;
    struct passwd *pwdent;

    setuniverse(UCB);
    if (--argc < 1)
	{
	fprintf(stderr, "Usage: %s user [universe]\n", *argv);
	exit(1);
	}
    else
	++argv;
    if (!(loginid = getlogin()))
	{
	if (!(pwdent = getpwuid(getuid())))
	    {
	    fprintf(stderr, "You don't exist\n");
	    exit(1);
	    }
	loginid = pwdent->pw_name;
	}
    if (!(univ = fopen(UNIVERSE, "r+")))
	{
	perror(UNIVERSE);
	exit(1);
	}
    setbuf(univ, NULL);
    if (flock(fileno(univ), LOCK_SH | LOCK_NB) < 0)
	{
	fprintf(stderr, "File is busy\n");
	fclose(univ);
	exit(1);
	}
    strcpy(name, *argv);
    strcat(name, ":");
    len = strlen(name);
    while (fgets(buf, sizeof(buf), univ) && strncmp(name, buf, len))
	;
    if (strncmp(name, buf, len))
	{
	fprintf(stderr, "Name not found\n");
	release(univ);
	exit(1);
	}
    if (argc == 1)
	{
	printf("%s", &buf[len]);
	release(univ);
	exit(0);
	}
    else
	{
	if (getuid() && strcmp(*argv, loginid))
	    fprintf(stderr, "Permission denied\n");
	else
	    {
	    for (i = 0; i < NOUNIV; i++)
		if (!strcmp(argv[1], univ_name[i]))
		    {
		    flock(fileno(univ), LOCK_EX);
		    fseek(univ, (long) (-strlen(buf)), 1);
		    fprintf(univ, "%s:%s\n", argv[0], argv[1]);
		    release(univ);
		    exit(0);
		    }
	    fprintf(stderr, "Unknown universe\n");
	    }
	}
    release(univ);
    exit(1);
    }
