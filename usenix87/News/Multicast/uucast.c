/*
 *	uucast - Cast a UUX command to multiple systems.
 *
 *	86/07/12.	2.1	Shane P. McCarron (MECC).
 *
 *	Copyright (C) Shane McCarron, 1987.
 *
 *	Note:	this program must run suid uucp.  It will create the
 *		files in the uucp directories owned by user uucp, with
 *		mask MASK.
 */

#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include "uucast.h"
#ifndef	NODE
#include <sys/utsname.h>
#endif	NODE

#define	NAMELEN	15		/* length of file names */

char	sccsid[] = "@(#)uucast.c	1.5";

#ifdef	NODE
char	node[] = NODE,
#else	NODE
char	node[8],		/* variable for node name from uname */
#endif	NODE
	user[] = USER,		/* the user that is doing the sending */
	comname[NAMELEN],	/* name of command file */
	remname[NAMELEN],	/* name of remote command file */
	nremname[NAMELEN],	/* name of remote data file */
	locname[NAMELEN];	/* name of local data file */
extern	errno;


main(argc, argv)
int	argc;
char	**argv;
{
	void	makename();
	void	usage();

	char	file[128],		/* file name to transmit */
		path[128],		/* temporary path name */
		*command = argv[2],	/* command to UUX */
		*sysname[MAXSYS];	/* array of system names */
	FILE	*fptr;			/* file descriptor */
	int	i,			/* looping variable */
		result,			/* result of link call */
		seq,			/* sequence number */
		syscount = 0;		/* number of systems to send to */
	struct	passwd	*uucp;		/* struct for user uucp information */
	struct  passwd  *getpwnam();

	if (argc < 4)
		usage(argv[0]);

	uucp = getpwnam("uucp");
	setuid(uucp->pw_uid);
	setgid(uucp->pw_gid);

	strcpy(file, argv[1]);		/* get the data file name */

	for (i = 3; i < argc; i++)	/* get the system names */
	{
		sysname[i-3] = argv[i];
		syscount++;
	}
#ifndef	NODE
	(void) getnode(node);		/* get the node name */
#endif	NODE
	(void) umask(MASK);		/* clear the mask bits */
	seq = getpid();			/* get the process id */
	i = 0;
	while (i < syscount)
	{
		/*
		 * Cleaned up logic a bit - MHC
		 */
		do {
			makename(sysname[i], seq++);
#ifdef HDB
			(void) sprintf(path, "%s/%s/%s", XDIR, sysname[i], remname);
#else
			(void) sprintf(path, "%s/%s", XDIR, remname);
#endif HDB
		} while (!access(path, 0));

#ifdef HDB
		(void) sprintf(path, "%s/%s/%s", DDIR, sysname[i], comname);
#else
		(void) sprintf(path, "%s/%s", DDIR, comname);
#endif HDB

		if ((result = link(file, path)) == -1 && (errno == EXDEV))
		{
			/*
			 * Link failed due to target directoring being on
			 * a different file system.  Try doing a copy.
			 * If copy works then move path to file so that it
			 * can be used as a source to link the rest of the
			 * files.
			 */

			/* changed to do strcpy only if copy works - MHC */

			if (result = copy(file, path) == 0)
				strcpy(file, path);	    
		}  /* end if */
		else if (result != 0)
		{
			fprintf(stderr, "uucast: Link failed from %s to %s: %d\n", 
				file, path, errno);
			exit(errno);
		}  /* end else */
#ifdef HDB
		(void) sprintf(path, "%s/%s/%s", XDIR, sysname[i], remname);
#else
		(void) sprintf(path, "%s/%s", XDIR, remname);
#endif HDB
		fptr = fopen(path, "w");
#ifdef	WANTZ
		fprintf(fptr, "U %s %s\nN\nZ\nR %s\nF %s\nI %s\nC %s\n",
			user, node, user, comname, comname, command);
#else
		fprintf(fptr, "U %s %s\nF %s\nI %s\nC %s\n", user, node,
			comname, comname, command);
#endif	WANTZ
		fclose(fptr);
#ifdef HDB
		(void) sprintf(path, "%s/%s/%s", CDIR, sysname[i], locname);
#else
		(void) sprintf(path, "%s/%s", CDIR, locname);
#endif HDB
		fptr = fopen(path, "w");
#ifdef HDB
		fprintf(fptr, "S %s %s %s - %s 0666 %s\nS %s %s %s - %s 0666 %s\n",
			comname, comname, user, comname, user, remname, nremname,
			user, remname, user);
#else
		fprintf(fptr, "S %s %s %s - %s 0666\nS %s %s %s - %s 0666\n",
			comname, comname, user, comname, remname, nremname,
			user, remname);
#endif HDB
		fclose(fptr);
		i++;
	} /* end for */
}


int	copy(file1, file2)
char	*file1, *file2;			/* source and destination name */
{
	char	cmd[BUFSIZ];		/* make a command string */
	
	(void) sprintf(cmd, "cp %s %s", file1, file2);
	return(system(cmd));
}

#ifndef	NODE
int	getnode(name)
char	*name;
{
	struct	utsname	buffer;

	(void) uname(&buffer);
	strcpy(name, buffer.nodename);
}
#endif	NODE

/*	makename - make the name files for casting.
**
**	remname and nremname use grade2 because on some systems (Ultrix)
**	the remote command file must have an X in its name.  On normal
**	systems, just make GRADE and GRADE2 the same.
**
*/

void	makename(sysnam, num)		/* make file names */
char	*sysnam;			/* system name */
int	num;				/* sequence number */
{
	while (num > (10000-1))
		num = num - 10000;
#ifdef HDB
	(void) sprintf(comname, "D.%.5s%s%04d", node, GRADE, num);
	(void) sprintf(remname, "D.%.5s%04da12c", sysnam, num);
	(void) sprintf(nremname, "X.%.7s%s%04d", sysnam, GRADE, num);
	(void) sprintf(locname, "C.%.7s%s%04d", sysnam, GRADE, num);
#else
#ifndef	GRADE2
	(void) sprintf(comname, "D.%s%s%04d", sysnam, GRADE, num);
	(void) sprintf(remname, "D.%s%s%04d", node, GRADE, num);
	(void) sprintf(nremname, "X.%s%s%04d", node, GRADE, num);
	(void) sprintf(locname, "C.%s%s%04d", sysnam, GRADE, num);
#else
	(void) sprintf(comname, "D.%s%s%04d", sysnam, GRADE, num);
	(void) sprintf(remname, "D.%s%s%04d", node, GRADE2, num);
	(void) sprintf(nremname, "X.%s%s%04d", node, GRADE2, num);
	(void) sprintf(locname, "C.%s%s%04d", sysnam, GRADE, num);
#endif	!GRADE2
#endif	HDB
}

void	usage(command)
char	*command;
{
	fprintf(stderr, "usage: %s file command system(s)\n", command);
	exit(1);
}

