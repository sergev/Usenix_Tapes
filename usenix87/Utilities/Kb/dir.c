#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<sys/stat.h>

/* dir - to execute a function for each file in a directory
**
** int dir(dirname,f,recursive,ignore)
**
** char *dirname;
** int (*f)();
** int recursive;
** int ignore;
**
** For each file in dirname (except '.' and '..') the function f will
** be called with the name of the file as argument (eg. f(name))
** When recursive = 0 then only 1 level of directory is done, 
** all other values of recursive will make dir() do a recursive decent.
** 
** When (*f)(name) returns to dir() with return code != 0 then dir
** aborts immediately with the return code from  (*f)(name).
** Setting ignore != 0 will make dir() ignore errors from subdirectories
** in the case that recursive != 0.
**
** Return code values for dir() are:
**	0	all okay no error
**	1	directory does not exist
**	2	argument is not a directory
**	3	argument length too long to handle
**	4	cannot read directory file
**	*	return code from (*f)(name)
**
** The definition of f should be:
**
** int f(name)
** char *name;
**
** f should ALWAYS return an error code or 0 if all okay.
*/

#define	BUFSIZE	512

int dir(dname,f,recursive,ignore)
char	*dname;	/* name of directory to work on */
int	(*f)();	/* function to call for every file in dir */
int	recursive; /* if != 0 then recursive decent */
int	ignore;	/* if !=0 then ignore subdir in error */
{
	struct	direct	dirbuf;
	struct	stat	stbuf;
	char 	*nbp;
	char	*nep;
	register int i;
	int	fd;
	char	fname[BUFSIZE];
	int	code;

	/* check if dname  is a dir */
	if (stat(dname, &stbuf) == -1) {
#ifdef DEBUG
		fprintf(stderr,"dir(): can't find '%s'\n",dname);
#endif
		return(1);
	}
	if ((stbuf.st_mode & S_IFMT) != S_IFDIR) {
#ifdef DEBUG
		fprintf(stderr,"dir(): '%s' is not a directory\n",dname);
#endif
		return(2);
	}

	/* check for length overflow: name = dname / fname */
	if (strlen(dname) + 2 + DIRSIZ > BUFSIZE) {
#ifdef DEBUG
		fprintf(stderr,"dir(): '%s' string too long\n",dname);
#endif
		return(3);
	}

	if ((fd = open(dname,0)) == -1) { /* cannot open */
#ifdef DEBUG
		fprintf(stderr,"dir(): cannot open '%s'\n",dname);
#endif
		return(4);
	}

	while (read(fd, (char *)&dirbuf, sizeof(dirbuf)) > 0) {
		if (dirbuf.d_ino == 0) { /* slot not in use */
			continue;
		}
		if (strcmp(dirbuf.d_name, ".")  == 0 ||
		    strcmp(dirbuf.d_name, "..") == 0) {/* skip . and .. */
		    	continue;
		}

		/* construct fname */
		strcpy(fname,dname);
		strcat(fname,"/");
		strcat(fname,dirbuf.d_name);

		/* call function and check result */
		code = (*f)(fname);
		if (code != 0) goto end;

		/* check if we need recursion */
		if ( recursive == 0 ) continue;

		/* else see if this is a dir. */
		if (stat(fname, &stbuf) == -1) {
			return(1);
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			code = dir(fname,f,recursive,ignore);
			if (ignore != 0 ) continue;
			if (code != 0) goto end;
		}
	}/*while*/
	code = 0;
end:
	close(fd);
	return(code);
}/*dir*/
