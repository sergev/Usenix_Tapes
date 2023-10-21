#

/*
 * This program is a program to join a user on to the UNIX system. This program
 * is invoked by using the join command in the format-join user [group]. This
 * was written by B. Rappaport and W. Shannon.
 */
#include "stdio.h"

#define max(a,b) ((a) > (b)?(a) : (b))

#define	LENGTH	400	/* max length of line in group file */

char	line[LENGTH];

main (argc, argv)
int     argc;
char   *argv[];
{
	int     gid, num, i, b, grp, ngrp, pswd, uid, check, owner, n;
	char	*ap, *p;

	if (argc < 2) {
		printf ("usage: join user [group]\n");
		exit (1);
	}

	/* if group not specified, default is "other" */
	if (argc == 2) {
		argv[2] = "other";
	}
	gid = 10;	/* in case can't find it in group file */

	if ((grp = fopen ("/etc/group", "r")) < 0)
		fatal ("/etc/group: cannot open\n");
	if ((ngrp = fopen ("/etc/newgroup", "w")) < 0)
		fatal ("/etc/newgroup: cannot open\n");

	/*
	 * search the group file searching for the group name
	 * specified (or default "other")
	 */

	while (fgets (line, LENGTH, grp)) {
		for (p = line; *p != ':'; p++);
		*p = '\0';

		/* 
		 * compares group names and places the user`s
		 * name in the appropriate group section
		 */

		b = strcmp (argv[2], line);
		*p = ':';
		if (b == 0) {
			gid = getid (line);
			for (p = line; *p; p++);
			p--;
			if (*(p - 1) != ':')
				*p++ = ',';
			ap = argv[1];
			while (*p++ = *ap++);
			*(p - 1) = '\n';
			*p = '\0';
		}
		fputs (line, ngrp);
	}

	fclose (grp);
	fclose (ngrp);
	if ((pswd = fopen ("/etc/passwd", "r")) < 0)
		fatal ("/etc/passwd: cannot open\n");
	uid = 0;
	while (fgets (line, LENGTH, grp)) {
		i = getid (line);
		if (i < 99)
			uid = max (uid, i);
	}
	uid = uid + 1;

	/* 
	 * The user is added to the password file.
	 */

	if ((pswd = fopen ("/etc/passwd", "a")) < 0)
		fatal ("/etc/passwd: cannot open for append\n");
	if (unlink ("/etc/group") == -1)
		fatal ("/etc/group: cannot unlink\n");
	if (link ("/etc/newgroup", "/etc/group") == -1)
		fatal ("/etc/newgroup: cannot link\n");
	if (unlink ("/etc/newgroup") == -1)
		fatal ("/etc/newgroup: cannot unlink\n");
	sprintf (line, "%s::%d:%d::/case/usr/%.14s:\n", argv[1], uid, gid, argv[1]);
	fputs (line, pswd);
	fclose(pswd);

	/* 
	 * A directory is made for the user. The user is given a .mail file. The 
	 * .mail file of the owner and group is changed.
	 */

	owner = (gid << 8) | uid;
	if ((chdir ("/case/usr")) < 0)
		fatal ("/case/usr: cannot chdir\n");
	if ((mknod (argv[1], 040755, 0)) < 0)
		fatal ("cannot make user directory\n");
	sprintf(line, "%s/.", argv[1]);
	link(argv[1], line);
	sprintf(line, "%s/..", argv[1]);
	link(".", line);
	if ((chown (argv[1], owner)) < 0)
		fatal ("cannot chown\n");
	chdir(argv[1]);
	if ((close (creat (".mail", 0622))) < 0)
		fatal ("cannot create .mail file\n");
	if ((chown (".mail", owner)) < 0)
		fatal (".mail: cannot chown\n");

	printf("User %s, group %s joined as uid %d, gid %d\n",
		argv[1], argv[2], uid, gid);
}

/*
 *	getid - convert number in second field of passwd
 *		or group file line to binary
 */

getid (line) {
	char   *p;
	int     i, num;

	p = line;
	for (i = 0; i < 2; i++) {
		while (*p++ != ':');
		num = 0;
		while (isdigit (*p)) {
			num =* 10;
			num =+ (*p - '0');
			p++;
		}
	}
	return (num);
}


fatal (s)
char   *s;
{
	fprintf (stderr, s);
	unlink("/etc/newgroup");
	exit (1);
}
