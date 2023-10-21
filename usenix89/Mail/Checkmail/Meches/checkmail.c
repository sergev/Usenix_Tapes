#ifndef lint
static char *sccsid = "@(#)checkmail.c	1.4 (Scott Mesches) 7/18/86";
#endif

/*
 * checkmail  -- Mail checker for me and other folks.
 * 		Returns a status of one for new mail, zero otherwise.
 * 		-q option supresses output and only affects status.
 * Usage: checkmail [ -q ] [ name... ]
 */

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	MAILDIR	"/usr/spool/mail"
#define	TRUE	1
#define	FALSE	0

main (argc, argv)
int     argc;
char   *argv[];
{
	int     i, qflag = FALSE;
	struct passwd  *getpwuid (), *pwd;

	if (argc == 1) {
		if ((pwd = getpwuid (getuid ())) == NULL) {
			perror (*argv);
			exit (1);
		}
		printf ("You have ");
		exit (status (pwd -> pw_name,qflag));
	}
	else {
		if (*argv[1] == '-') {
			if (!strcmp("-q",argv[1])) {
				qflag = TRUE;
				if ((pwd = getpwuid (getuid ())) == NULL) {
					perror (*argv);
					exit (1);
				}
				(void)status (pwd -> pw_name,qflag);
			}
			else
				fprintf(stderr,"Usage: %s [ -q ] [ user ... ]\n", argv[0]);
		}
		else
			for (i = 1; i < argc; i++) {
				printf ("%s has ", argv[i]);
				(void)status (argv[i],qflag);
		}
	}
}

status (name,flag)
char	*name;
int		flag;
{
    char    path[1024];
    struct stat st;

    (void) sprintf (path, "%s/%s", MAILDIR, name);
    if (stat (path, &st)) {
		if (flag == FALSE)
			printf ("no mailbox.\n");
		return (0);
    }
    if (st.st_size == 0) {
		if (flag == FALSE)
			printf ("no mail.\n");
	}
    else
		if (st.st_mtime > st.st_atime) {
			if (flag == FALSE)
				printf ("new mail.\n");
			return (1);
		}
		else
			if (flag == FALSE)
				printf ("mail.\n");
	return(0);
}
