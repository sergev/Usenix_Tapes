/* the delete command
	delete [-a] [-ad] [-r] [-v] [-d] [-f] files
	written by Keith Davis Mar 10, 1976
	as modified by Lawrence Martin Nov 1, 1977


function:
        The delete command enables a  UNIX  user  to  remove  the
        contents  of  directories  or  files.  It  is possible to
        delete whole file systems with the recursive flag.  If  a
        file  named  is not a directory then delete works exactly
        the same as rm. If the file is a directory then the files
        in that directory are removed.

        Unless the  -f  flag  is  set,  delete  will  ask  before
        deleting  a  nondirectory file if that file does not have
        write permission.

	This program should be compiled by the command:
		cc -s -n -O delete.c


parameters:
	-a      Ask before removing a file.  Set askdirflag (ad).

	-r      Recursively examine directories.

	-ad     Ask whether a directory is to be examined when it
		is discovered.

	-v      Print messages to tell what the program is doing.

	-d      Remove a directory after all its files have been
		deleted.  But if the askdirflag had been set ask
		before removing the directory.

	-f      Do not print error messages or other messages.


files:  /bin/rmdir to remove empty directories.


history:
	version 1.0:    Keith Davis  Mar 10, 1976.
	This version only had flags -a, -r, -ad, and -v.
        It did not check whether  nondirectory  files  had  write
        permission.   The  flags  -a  and  -ad  did  not  cause a
        question to be asked for  the  initial  files  themselves
	that are given as arguments to the command.

	version 1.1:    Lawrence Martin  Nov 1, 1977.
	Flags -d and -f were added.
        If the argument file is a nondirectory file  delete  will
        work exactly like rm, except that we now have an askflag.
        Flags -a and -ad cause a question to  be  asked  for  the
        initial   argument   files   (besides   any  lower  level
	directories and files).

	version 1.2:    Lawrence Martin  Nov 22, 1977.
        If you type 'delete' by itself a short description of the
	command will be given.
*/

char id[] "~|^`delete.c\t1.2\n";

int parent;     /* the parent process id for the rmdir forks */

struct
{       
	int     dev;    /* the device type */
	int     inumber;/* the inode number */
	int     flags;  /* the flags */
	char    nlinks; /* number of links to the file */
	char    uid;    /* the uid of the file */
	char    gid;    /* the group id of the file */
	char    size0;  /* high byte of the 24 bit size */
	int     size1;  /* low word of the 24 bit size */
	int     addr;   /* first address */
	int     jnk[11];/* other info */
}       
stat1;

int     type1;

char name1[256]; /* buffers for source and destination names */
char *name1l; /*points to the last char '\0' in name1 and name2*/

int     direntry[9];    /* buffer for directory entries */

int     askflag;        /* ask before doing anything */
int     askdirflag;     /* ask before doing anything to
			   directories */
int     recursiveflag;  /* if on then will branch down directory
			   structure */
int     verboseflag;    /* print lots of information */
int     directoryflag;  /* remove empty directories;
			   if askdirflag is set then ask first */
int     fflag;          /* don't ask or write anything */

main(argc, argv)
int argc;
char **argv;
{       
	register int i;
	register char *s, *t;
	int k;  
	k = 0;
	for(i=2;i<16;i++) close(i); /* close unneeded files */
	for(i=1;i<argc;i++)
	{       
		if(argv[i][0] == '-') switch(argv[i][1])
		{       
		case 'a': 
			if (argv[i][2] == 'd') askdirflag = 1;
			else askflag = askdirflag = 1;
			break;
		case 'r': 
			recursiveflag = 1;  
			break;
		case 'v': 
			verboseflag = 1;  
			break;
		case 'd': 
			directoryflag = 1;  
			break;
		case 'f': 
			fflag = 1;  
			break;
		}
		else argv[k++] = argv[i]; /* copy the arg */
	}
	if(fflag&&(askflag||askdirflag||verboseflag))
	{
		printf("Error: inconsistent flags\n");
		exit(1);
	}
	if(k < 1)  /* k now is the number of sources */
	{       
		help();
		exit(1);
	}
	for(i=0;i<k;i++)   /* this code solves the multiple
			      sources problem */
	{
		/* get a source */
		s = name1;  
		t = argv[i];
		while(*s++ = *t++);
		name1l = --s;/*point to the '\0' in char string*/
		if (*--s == '/') { 
			*s = 0; 
			name1l = s; 
		} /* extra / */

		getstat();
		if (type1 == 4) {
			if (!fflag) printf("file %s doesn't exist\n",name1);
			continue;
		}
		if (type1 == 2) {
			if (ask(askdirflag,"examine directory",name1))
				rd(); /* do remove dir bit */
		}
		else if(ask(askflag,"remove file",name1))
			rm();    /* do remove file bit */
	}
}

help()
{
	printf("To use delete type 'delete [-a] [-r] [-ad] [-v] ");
	printf("[-d] [-f] files'.\n");
	printf("'files' may be one or more directories or nondirectory files.\n");
	printf("For a nondirectory file delete removes the ");
	printf("file the same as rm does.\n");
	printf("If an argument is a directory then delete removes");
	printf(" all of the files \nin that directory.\n\n");
	printf("parameters:\n");
	printf("-a      Ask before removing a file.  Set ad flag.\n");
	printf("-r      Recursively examine directories.\n");
	printf("-ad     Ask before examining a directory.\n");
	printf("-v      Print messages to say what the program is doing.\n");
	printf("-d      Remove a directory after all its files");
	printf(" have been deleted.\n        But if the askdirflag");
	printf(" had been set ask before removing the directory.\n");
	printf("-f      Do not print error messages or ");
	printf("any other messages.\n");
	return;
}

getstat() /* gets the status on the file */
{       
	if (stat(name1,&stat1) == -1) type1 = 4;
	else    type1 = ((stat1.flags)>>13)&3;
}

rd() /* remove a directory */
{       
	register int dirfd; /* source-directory file descriptor */
	register int flag;  /* set if directory cannot be removed */
	int pid,status,name1s;

	flag = 0; /* set directory to be removed */

	if ((dirfd = open(name1,0)) == -1)
	{       
		if (!fflag) printf("Error: could not open directory %s\n",name1);
		return(1);
	}

	name1s = name1l; /* save pointers */
	getentry(dirfd); /* skip first two entries */
	getentry(dirfd); /* because they are the "." & ".." entries */
	while(getentry(dirfd))  /* get an entry */
	{       
		push_name();    /* and push it onto the name1 & name2 */
		getstat();
		switch (type1)
		{       
		case 0: /* just a file */
			if (ask(askflag,"remove file",name1))
				flag =+ rm();
			else flag++;
			break;
		case 1: /* character-type special file */
		case 3: /* block-type special file */
			if (ask(askdirflag,"remove special file",name1))
				flag =+ rm();
			else flag++;
			break;
		case 2: /* directory */
			if (recursiveflag) /* ignore if not */
			{       
				if (ask(askdirflag,"examine directory",name1))
					flag =+ rd();
				else flag++;
			}
			else flag++;
			break;
		default:/* error */
			if (!fflag) printf("Error: %s does not exist",name1);
			flag++; /* cannot remove directory */
			return(1);
		}
		name1l = name1s;  
		*name1l = 0; /* pop name1 */
	}
	close(dirfd); /* close the file source-directory */

	if (flag&&verboseflag)
	{       
		printf("directory %s not empty\n",name1);
		return(1);
	}
	if (flag) return(1);
	else if (directoryflag&&ask(askdirflag,"rmdir",name1))
		/* remove the directory */
	{       
		do if((pid=pfork()) == 0)
		{       
			execl("/bin/rmdir","rmdir",name1,0);
			if (!fflag) printf("Error: could not exec rmdir\n");
			kill(9,parent);
			exit();
		}  
		while(pid == -1);
		while(pid != wait(&status));

		getstat();

		if (type1 != 4) /* if not four then error */
		{       
			if(!fflag) printf("Error: could not rmdir %s\n",name1);
			return(1);
		}

		if (verboseflag) printf("directory %s has been deleted\n",name1);
		return(0);
	}
	else return(1);
}

getentry(df)
int df; /* the directory file descriptor */
{       
	do
	    {       
		if(read(df,direntry,16) != 16) return(0);
	} 
	while (direntry[0] == 0);
	return(1);
}

push_name()
{       
	register char *s, *t;

	/* add entry onto name1 */
	if (name1l+17 > &name1[256])
	{       
		if (!fflag) printf("Error file name too long %s\n",name1);
		exit(1);
	}
	s = name1l;  
	t = &direntry[1];  
	*s++ = '/';
	while (*s++ = *t++);
	name1l = --s;
}


rm()    /* works just like the rm command */
{
	static int buf[256];
	register int n;
	register char *s, *t;
	int i, b;

	if(!fflag) {
		if((getuid()&0377) == (stat1.uid&0377)) /* uid >127 */
			b = 0200;
		else if((getgid()&0377) == (stat1.gid&0377))
			b = 020;
			else    b = 2;
		if((stat1.flags & b) == 0 && ttyn(0) != 'x') {
			printf("%s: %o mode ",name1,stat1.flags);
			i = b = getchar();
			while(b != '\n' && b != '\0')
				b = getchar();
			if(i=='x' || i=='X') exit(0);
			if(i!='y' && i!='Y') return(1);
		}
	}
	if (unlink(name1) == -1)
	{       
		if (!fflag) printf("could not remove %s\n",name1);
		return(1);
	}
	if(verboseflag) printf("file %s has been removed\n",name1);
	return(0);
}

ask(f,s,t) int f;
{       
	char ch;
	register flg;
	if (f)  /* if flag true then ask question */
	{       
		printf("%s %s? ",s,t);
		if (read(0,&ch,1) != 1) return(0);
		flg = ch == 'y';
		while (ch != '\n' && read(0,&ch,1) == 1) ;
		return(flg);
	}
	return(1);
}

