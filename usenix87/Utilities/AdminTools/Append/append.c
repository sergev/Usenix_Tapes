/* 4.0: */ 
#line 48 append.web

/* 4.4: */ 
#line 253 append.web

/*
 * append -- a command to append a file to a directory to which 
 *	one does not have write permission, using setgrp & ln. 
 *	Placing append in a directory simulates the "sa a *.*" 
 *	access control command of Multics. See also append.web. 
 */

/* :4.4 */

#line 261 append.web


/* 4.2: */ 
#line 187 append.web

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

/* :4.2 */

#line 193 append.web


/* 4.1: */ 
#line 104 append.web

#define ERR (-1)

/* :4.1 */

#line 107 append.web
/* 4.2: */ 
#line 197 append.web

#define DIRECTORY 0040000
#define SPECIAL (0020000 | 0060000)

/* :4.2 */

#line 201 append.web


/* 4.1: */ 
#line 61 append.web

 void
main(argc,argv) int argc; char *argv[]; {
	/* 4.2: */ 
#line 193 append.web
	
	struct stat s;	/* stat buffer */
	int	m; 	/* file access mode */
	
	/* :4.2 */
	
#line 197 append.web
	/* 4.3: */ 
#line 246 append.web
	
	extern int errno;
	extern char *sys_errlist[];
	int	rc;
	
	/* :4.3 */
	
#line 251 append.web


	char	*progName, *fromName, *toName,
		*segmentPart(/* char * */);

	progName = argv[0];
	if (argc < 2) {
		/* no parms, must be a request for information */
		/* 4.4: */ 
#line 261 append.web
		
		fprintf(stderr,"%s -- add (via link) a file to this directory, %s\n",
			progName, "even if you lack permission.");
		fprintf(stderr,"Usage: %s filename [newname]\n",progName);
		
		/* :4.4 */
		
#line 266 append.web


		exit(0);
	}
	else if (argc == 2) {
		/*  one parm, make names the same */
		fromName = argv[1];
		toName = segmentPart(argv[1]);
	}
	else if (argc == 3) {
		/* two parms, make second one the new name */
		fromName = argv[1];
		if (strcmp(argv[2],".")==0) {
			/* the directory */
			toName = segmentPart(fromName);
		}
		else {
			toName = segmentPart(argv[2]);
		}
	}
/* :4.1 */

#line 90 append.web
/* 4.1: */ 
#line 96 append.web


	/* 4.2: */ 
#line 139 append.web
	
	if (stat(progName,&s) == ERR || s.st_mode & (DIRECTORY|SPECIAL)
	   || (s.st_uid != geteuid() && s.st_gid != getegid())) {
		/* someone's trying to trick me by putting append in his path */
		fprintf(stderr,"%s: Can't append to current directory, %s\n",
			progName, "\"cd\" to target directory first");
		exit(1);
	}
	
	
	if (stat(fromName,&s) == ERR) {
		switch (errno) {
		case ENOTDIR: 
			fprintf(stderr,"%s: Can't access %s, %s.\n", progName,
				fromName, "part of the path is a non-directory");
			break;
		case ENOENT:
			fprintf(stderr,"%s: File %s doesn't exist.\n",
				progName,fromName);
			break;
		case EACCES:
			fprintf(stderr,"%s: Can't search a directory in %s.\n",
				progName,fromName);
			break;
		default:
			fprintf(stderr,"%s: Can't link, error is \"%s\".\n", 
				progName,sys_errlist[errno]);
		}
		exit(1);
	}
	else if ((m=s.st_mode) & SPECIAL) {
		fprintf(stderr,"%s: Can't append a special file.\n",progName);
		exit(1);
	}
	else if (m & DIRECTORY) {
		fprintf(stderr,"%s: Can't append a directory.\n",progName);
		exit(1);
	}
	
	if (stat(toName,&s) != ERR) {
		fprintf(stderr,"%s: Can't replace an existing file\n",
			progName);
		exit(1);
	}
	/* :4.2 */
	
#line 183 append.web

 
	rc = link(fromName,toName);
	/* 4.3: */ 
#line 207 append.web
	
	if (rc == ERR) {
		switch (errno) {
		case EACCES: 
			fprintf(stderr,"%s: Cannot write to this directory (%s).\n",
				progName, "Can't happen, send mail to the owner");
			exit(3);
		case EXDEV:
			fprintf(stderr,"%s: Can't do a cross-device link.\n",
				progName);
			exit(1);
		case EROFS:
			fprintf(stderr,"%s: Can't link to a r/o file system.\n",
				progName);
			exit(1);
		case EMLINK:
			fprintf(stderr,"%s: Can't link, too many already exist.\n",
				progName);
			exit(1);
		case ENOSPC:
			fprintf(stderr,"%s: Can't link, directory full.\n",
				progName);
			exit(1);
		case ENOTDIR:
		case ENOENT:
		case EEXIST:
		case EPERM:
			fprintf(stderr,"%s: Can't link, impossible error \"%s\".\n",
				progName,sys_errlist[errno]);
			exit(3);
		default:
			fprintf(stderr,"%s: Can't link, error is \"%s\".\n", 
				progName,sys_errlist[errno]);
			exit(2);
		}
	}
	/* :4.3 */
	
#line 243 append.web


	exit(0);
}

/* :4.1 */

#line 104 append.web


/* 5.0: */ 
#line 272 append.web

 char *
segmentPart(s) char *s; {
	char	*p, *strrchr(/* char *, char */);

	if (s && (p=strrchr(s,'/'))) {
		return ++p;
	}
	return s;
}

/* :5.0 */

#line 282 append.web



/* :4.0 */

#line 55 append.web

