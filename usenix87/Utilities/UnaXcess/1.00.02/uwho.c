/*
 *	@(#)uwho.c	1.1 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)uwho.c	1.1 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <utmp.h>

#ifndef NOAUTOPATH
struct passwd *getpwuid();
#endif  NOAUTOPATH

#define TTYLIST "/etc/utmp"

main(argc, argv)
char **argv; {
	int cnt, flg, users;
	struct utmp user;
	char tty[16];

	if ((users = open(TTYLIST, 0)) < 0) {
		fprintf(stderr, "%s: can't open %s\n", argv[0], TTYLIST);
		exit(1);
	}
	flg = 0;
	while (read(users, &user, sizeof user) > 0) {
#ifdef SYS5
		if (user.ut_type != USER_PROCESS)
			continue;
#else  SYS5
		if (user.ut_name[0] == '\0')
			continue;
#endif SYS5
		sprintf(tty, "/dev/%.*s", sizeof user.ut_line, user.ut_line);
		if (argc < 2)
			showme(&tty[5]);
		else
			for (cnt = 1; argv[cnt] != NULL; cnt++)
				if (strcmp(&tty[5], argv[cnt]) == 0) {
					showme(&tty[5]);
					break;
				}
	}
	exit(0);
}

showme(ttyf)
char *ttyf;
{
	FILE *fp;
	char line[1024];
#ifndef NOAUTOPATH
	static char home[1024] = "";
#endif NOAUTOPATH

#ifdef NOAUTOPATH
	sprintf(line, "%s/%s", NOAUTOPATH, ttyf);
#else
	if (home[0] == '\0')
		strcpy(home, getpwuid(geteuid())->pw_dir);
	sprintf(line, "%s/%s", home, ttyf);
#endif NOAUTOPATH
	if ((fp = fopen(line, "r")) == NULL)	/* not in use on this tty */
		return;
	fgets(line, 1024, fp);
	printf("%s: %s", ttyf, line);		/* line already has newline */
	fclose(fp);
}
