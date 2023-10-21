#include "uutty.h"
/* 
** Start up a new program in the current process.  This is normally
** to start up a shell for a logged-in user.   If the command can't
** be executed, -1 is returned.	 Note that we can define a new search
** path here which will overwrite the one we were given.
*/
	Flag  forkfl = 0;	/* Should we fork or exec directly? */
	Flag  pathfl = 0;	/* True if PATH defined in environment */
/*
** The 'path' variable should be defined appropriately for your
** system.  If it is null, we will just pass on the path that
** was handed to us, which may be right on many systems.
*/
#ifdef Cadmus
	char *path = "PATH=:.:/bin:/bin.cadmus:/bin.sys5:/bin.sys3:/bin.ver7:/usr/ucb:/usr/bin:/etc:/usr/local:/usr/new:";
#define Path
#endif
#ifdef VME
	char *path = "PATH=:.:/bin:/usr/bin:/etc:";
#define Path
#endif
#ifndef Path
	char *path = 0;		/* No search path */
#endif

exec(cook,cmd,pp)
	int   cook;		/* Should we do cooked or raw I/O? */
	char *cmd;		/* Null-terminated command */
	struct passwd *pp;	/* The user's /etc/passwd entry, unpacked */
{	int   a, c, e, i, n, r;
	int   child;
	Flag  lognfl, shelfl, homefl;
	char *av[10];		/* This should be room enough */
	char *cp, *dp;
	char**ev;
	char  arg0[30];		/* For shell's visible name */
	char  logn[30];		/* For LOGNAME=<id> string */
	char  home[30];		/* For HOME=<dir> string */
	char  shll[30];		/* For SHELL=<prog> string */
	char *shell="/bin/sh";	/* default shell */
	extern long malloc();

	D5("exec(%d,\"%s\",%06lX)",cook,cmd,pp);
	r = -1;
	for (i=0; environ[i]; i++);	/* How big is environ[]? */
	n = (i+5) * sizeof(char**);	/* Space for copy + 5 entries */
	D5("exec: Get %d bytes for %d+5 environment entries.",n,i);
	ev = (char**)malloc(n);		/* Allocate a copy for us */
	if (ev == NULL) {
		E("Can't get %d bytes for environment table [exec].",n);
		Fail;
	}
	for (e=0; environ[e]; e++) {	/* Copy the environment vector */
		ev[e] = environ[e];
		D6("ev[%2d]=\"%s\"",e,ev[e]);
	}
	ev[e] = 0;
	D3("exec: There are %d environment entries.",e);
	pid = getpid();
	D4("exec: This is process %d.",pid);
	a = e = 0;			/* indices for av[] and ev[] */
	for (cp=cmd; *cp; ++cp) {	/* Scan the command for whitespace */
		c = *cp;
		if (c == ' ' || c == '\t') {
			av[a++] = "-sh";
			av[a++] = "-c";
			av[a++] = cmd;
			break;
		} else
		if (c == ':' || c == '\n') break;
	}
	if (a == 0) {				/* the command is just a program name */
		shell = cmd;			/* Note that it's our shell */
		dp = av[a++] = arg0;		/* Name to tell shell */
		*dp++ = '-';			/* Build arg0 with initial '-' */
		cp = lastfield(cmd,'/');	/* Find the last field of the name */
		while (*cp) *dp++ = *cp++;	/* Copy name to arg0 */
		*dp = 0;			/* Gotta have a null terminator */
		D4("exec: arg0=\"%s\"",arg0);
	}
	D3("av[%2d]=\"%s\"",a-1,av[a-1]);

	D7("exec:before sprintf(%06lX,\"%s\",%06lX)",logn,"LOGNAME=%s\0",pp->pw_name);
	sprintf(logn,"LOGNAME=%s\0",pp->pw_name);
	D6("exec: after sprintf() logn=\"%s\"",logn);

	D7("exec:before sprintf(%06lX,\"%s\",%06lX)",home,"HOME=%s\0"   ,pp->pw_dir);
	sprintf(home,"HOME=%s\0"   ,pp->pw_dir);
	D6("exec: after sprintf() home=\"%s\"",home);

	D7("exec:before sprintf(%06lX,\"%s\",%06lX)",shll,"SHELL=%s\0"  ,shell);
	sprintf(shll,"SHELL=%s\0"  ,shell);
	D6("exec: after sprintf() shll=\"%s\"",shll);

	if (debug >= 5) {
		av[a++] = "-VX";		/* This turns on shell debugging */
		D3("av[%2d]=\"%s\"",a-1,av[a-1]);
	}
	av[a] = 0;			/* Paranoia */
/*
** This is supposed to help, but seems not to:
**
	D4("exec: before setpgrp()");
	i = setpgrp();
	D4("exec: setpgrp()=%d",i);
*/
	lognfl = shelfl = homefl = 0;		/* Flags for noticing environment entries */
	for (e=0; ev[e]; e++) {			/* Examine the environment table */
		D3("exec: Examine ev[%2d]=%06lX=\"%s\"",e,ev[e],ev[e]);
		if (st_init("PATH=",ev[e]) > 0) {
			D7("exec: Matched \"PATH=\"; path=%06lX=\"%s\"",path,path);
			if (path) ev[e] = path;	/* Use special path */
			D6("ev[%2d]=\"%s\"",e,ev[e]);
			pathfl++;		/* Note we did it */
		}
		if (st_init("LOGNAME=",ev[e]) > 0) {
			ev[e] = logn;		/* Use special path */
			D6("ev[%2d]=\"%s\"",e,ev[e]);
			lognfl++;		/* Note we did it */
		}
		if (st_init("HOME=",ev[e]) > 0) {
			ev[e] = home;		/* Use special path */
			D6("ev[%2d]=\"%s\"",e,ev[e]);
			homefl++;		/* Note we did it */
		}
		if (st_init("SHELL=",ev[e]) > 0) {
			ev[e] = shll;		/* Use special path */
			D6("ev[%2d]=\"%s\"",e,ev[e]);
			shelfl++;		/* Note we did it */
		}
	}
	if (!homefl) ev[e++] = home;	/* Add the home directory */
	if (!lognfl) ev[e++] = logn;	/* Add the login id */
	if (!shelfl) ev[e++] = shll;	/* Add the shell's name */
	if (!pathfl) ev[e++] = path;	/* Add the search path (may be null) */
	D3("exec: There are %d environment entries.",e);
	ev[e] = 0;			/* Paranoia */
	if (debug >= 3) {		/* Display the shell's parameters */
		P("before execve(\"%s\",%lX,%lX)",shell,av,ev);
		for (i=0; av[i]; i++)
			P("arg[%2d]=\"%s\"",i,av[i]);
		for (i=0; ev[i]; i++)
			P("env[%2d]=\"%s\"",i,ev[i]);
	}
	if (debug) P("%s Start %s for u=%d=%s g=%d.",getime(),shell,pp->pw_uid,pp->pw_name,pp->pw_gid);
	if (cook) makesane(dev);
	if (dev != 0) {close(0); i = dup(dev); D3("exec: File %d=\"%s\"",i,device); }
	if (dev != 1) {close(1); i = dup(dev); D3("exec: File %d=\"%s\"",i,device); }
	child = -1;			/* Default is no child process */
	pid = getpid();			/* Note which process we are now */
	if (forkfl) {			/* Are we forking or execing directly? */
		D4("exec: Forking sub-process for shell.");
		if (lockfl) {		/* Should we create a lockfile? */
			lockfn = creat(lockfile,0);
			if (debug >= 2)
				P("%s: Create lockfile %d=\"%s\".",getime(),lockfn,lockfile);
			if (lockfn < 0) {	/* Either locked or directory isn't writable */
				E("Can't create lockfile \"%s\" [errno=%d]",lockfile,errno);
				die(errno);
			}
			locked = 1;		/* Note that there's a lockfile */
		}
		if ((child = fork()) > 0) {	/* Are we the parent? */
			D3("exec: %s: Shell process %d created.",getime(),child);	
			if (lsleep > 0) sleep(lsleep);
			for (;;) {		/* Loop until shell goes away */
				errno = 0;
				r = wait(0);		/* Sleep until a subprocess dies */
				D3("%s: wait(0)=%d",getime(),r);
				if (r == child)		/* Was it the shell process? */
					Done;		/* If so, all's fine */
				D3("%s: After wait(0)=%d [errno=%d]",getime(),r,errno);
				if (r > 0) {
					E("%s: Wrong child %d died; waiting for %d.",getime(),r,child);
					continue;
				}
				switch(errno) {
				case ECHILD:
					D3("exec: wait(0)=%d [errno=%d=No child]",r,errno);
					Done;
				default:
					E("Unknown errno=%d after wait(0)=%d",errno,r);
					Done;
				}
			}			/* Hang until the shell process dies */
done:			if (debug) sleep(1);
			D3("exec: %s: Child %d gone.",getime(),r);
			if (r > 0) r = 0;
			if (lockfl) unlock();
			if (debug) P("%s: \"%s\" done , die(%d)",getime(),shell,r);
			Fail;
		}
	}
/* 
** If we fall through to here, we are the process to exec a shell.
** We may be the original process or a subprocess.
*/
	if (debug > 1) {
		pid = getpid();
		P("%s: Shell process %d.",getime(),pid);
	}
	if (dev != 2) { close(2); i = dup(dev); }
	/**** Don't produce debug output after this! */
	errno = 0;
	if (dev>2) close(dev);	/* Don't give excess open files to the shell */
	r = execve(shell,av,ev);
	E("Can't exec \"%s\"\t[errno=%d]",shell,errno);
/*
** Note that we always terminate, regardless of how we get here.
** If we are being run from init(1), a new process will be started
** up.  If not, we just go away.  This isn't necessary, but seems
** in general to be a good idea.
*/
fail:				/* Both processes come here to terminate */
	if (lsleep > 0)
		sleep(lsleep);	/* Don't respawn too fast */
	die(r);			/* This will respawn if we're a daemon */
}
