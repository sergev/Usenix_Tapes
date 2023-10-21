
/*
 * bexec: execute a non-binary file for csh, with #! recognition
 * (usage: alias shell /usr/local/bin/bexec)
 */

#ifndef SH
#define SH		"/bin/sh"
#endif
#ifndef SYSBLK
#define SYSBLK		1024	/* the fastest disk read size */
#endif
#ifdef BSD
#define strrchr rindex
#else
char *calloc(), *basename(), *strrchr();
#endif
#ifdef RMC
char *dirname(), *strrchr();
#endif

main(argc, argv, envp)
char **argv, **envp; {
	register int cmd, oc;
	char kcmd[SYSBLK];
	register char *cp, *ap;
	register char **newargv;
	
	if ((cmd = open(argv[1], 0)) < 0)
		_exit(1);
	oc = read(cmd, kcmd, sizeof kcmd);
	close(cmd);
	if (kcmd[0] != '#' || kcmd[1] != '!') {
#ifdef RMC
		for (cp = kcmd; cp < &kcmd[oc]; cp++)
			if (*cp == 0 || *cp > '\177') {
				if (chdir(dirname(argv[1])) < 0) {
					extern int errno;

					write(1, dirname(argv[1]), strlen(dirname(argv[1])));
					_exit(errno);
				}
				argv[0] = "runcobol";
				execve(RMC, argv, envp);
				_exit(-115);
			}
#endif
		execve(SH, argv, envp);
		_exit(100);
	}
#ifndef BSD
	for (cp = kcmd + 2; *cp == ' '; cp++)
		;
	for (ap = cp; *ap != ' ' && *ap != '\n'; ap++)
		;
	if (*ap == ' ')
		*ap++ = '\0';
	for (cmd = 0; ap[cmd] != '\n'; cmd++)
		;
	ap[cmd] = '\0';
	newargv = (char **) calloc((unsigned) argc + 2, sizeof (char *));
	oc = 0;
	newargv[oc++] = basename(cp);
	argv++;
	if (*ap != '\0')
		newargv[oc++] = ap;
	for (; *argv != 0; argv++)
		newargv[oc++] = *argv;
	newargv[oc] = 0;
	execve(cp, newargv, envp);
#endif
	_exit(100);
}

#ifndef BSD

char *basename(path)
char *path; {
	register char *cp;

	if ((cp = strrchr(path, '/')) == (char *) 0)
		return path;
	return cp + 1;
}

#endif

#ifdef RMC

char *dirname(path)
char *path; {
	static char dir[1024];
	register char *cp;

	strcpy(dir, path);
	if ((cp = strrchr(dir, '/')) == (char *) 0)
		return ".";
	*cp = '\0';
	if (dir[0] == '\0')
		return "/";
	return dir;
}

#endif
