/* the copy command
	copy [-a] [-ad] [-l] [-n] [-o] [-r] [-v] sources destination
	written by Keith Davis Feb 13, 1976
	modified by Joseph Pallas May 1, 1980 to prevent unlinking of
		old file when copy fails
*/

int parent;     /* the parent process id for the mkdir forks */

struct
{       int     dev;    /* the device type */
	int     inumber;/* the inode number */
	int     flags;  /* the flags */
	char    nlinks; /* number of links to the file */
	char    uid;    /* the uid of the file */
	char    gid;    /* the group id of the file */
	char    size0;  /* high byte of the 24 bit size */
	int     size1;  /* low word of the 24 bit size */
	int     addr;   /* first address */
	int     jnk[11];/* other info */
}       stat1, stat2;

int     type1, type2;

char name1[256],name2[272]; /* buffers for source and destination names */
char *name1l, *name2l;  /* points to the last char '\0' in name1 and name2 */

int     direntry[9];    /* buffer for directory entries */

int     askflag;        /* ask before doing anything */
int     askdirflag;     /* ask before doing anything to directories */
int     linkflag;       /* if on then links are used where possible */
int     newonlyflag;    /* if on then override will not be done */
int     ownerflag;      /* if on and super user then will change owner on created files etc */
int     recursiveflag;  /* if on then will branch down directory structure */
int     verboseflag;    /* prints lots of information */
int     suflag;         /* 0 if super user otherwise 1 */
int	moveflag;	/* if on do copy followed by unlink */
char    *name;

main(argc, argv)
int argc;
char **argv;
{       register int i;
	register char *s, *t;
	char *name2ptr; /* remembers loc of last arg */
	int k;  k = 0;
	name = argv[0];
	suflag = getuid()&0377; /* set suflag id */
	for(i=2;i<16;i++) close(i); /* close unneeded files */
	if(equal(name,"move")) linkflag = moveflag = 1;
	for(i=1;i<argc;i++)
	{       if(argv[i][0] == '-') switch(argv[i][1])
		{       case 'a': if (argv[i][2] == 'd') askdirflag = 1;
				  else askflag = askdirflag = 1;
				  break;
			case 'l': linkflag = 1;  break;
			case 'n': newonlyflag = 1;  break;
			case 'o': if (suflag == 0) ownerflag = 1;
				  else printf("only super user may set -o flag\n");
				  break;
			case 'r': recursiveflag = 1;  break;
			case 'v': verboseflag = 1;  break;
			case 'm': linkflag = moveflag = 1;  break;
		}
		else name2ptr = argv[k++] = argv[i]; /* copy the arg */
	}
	if(--k < 1)  /* k now is the number of sources */
	{       if (k)
		{       printf("Error: arg count\n");
			exit(0);
		}
		k++; /* if only one source, use working directory */
		name2ptr = ".";
	}
	for(i=0;i<k;i++)   /* this code solves the multiple sources problem */
	{
		/* get a source */
		s = name1;  t = argv[i];
		while(*s++ = *t++);
		name1l = --s; /* point to the '\0' in char string */
		if (*--s == '/') { *s = 0; name1l = s; } /* extra / */

		/* get the destination */
		s = name2;  t = name2ptr; /* always the same file */
		while(*s++ = *t++);
		name2l = --s;
		if (*--s == '/') { *s = 0; name2l = s; } /* extra / */

		getstat();
		if (type1 == 2 && type2 != 0) cd(); /* do directory trans */
		else    /* do the cp command */
		{       /* make entry in the direntry for the cp command */
			s = &direntry[1];  t = name1;
			while (*s = *t++) if(*s++ == '/') s = &direntry[1];
			cp(); /* do the cp command */
		}
	}
}

getstat() /* gets the status on the two files */
{       if (stat(name1,&stat1) == -1) type1 = 4;
	else    type1 = ((stat1.flags)>>13)&3;
	if (stat(name2,&stat2) == -1) type2 = 4;
	else    type2 = ((stat2.flags)>>13)&3;
}

cd() /* copy a directory */
{       register int dirfd; /* source-directory file descriptor */
	int pid,status,mode,modeflags,name1s,name2s;
	mode = type2;  modeflags = stat1.flags; /* save for later use */
	if (type2 == 4) /* does not exist so create the directory */
	{       do if((pid=fork()) == 0)
		{       execl("/bin/mkdir","mkdir",name2,0);
			execl("/usr/bin/mkdir","mkdir",name2,0);
			printf("Error: could not exec mkdir\n");
			kill(9,parent);
			exit(0);
		}  while(pid == -1);
		while(pid != wait(&status));
		getstat();
	}
	if (type2 != 2) /* if two then directory otherwise error */
	{       if (type2 == 4) printf("Error: could not mkdir %s\n",name2);
		else            printf("Error: %s is not a directory\n",name2);
		return;
	}
	if ((dirfd = open(name1,0)) == -1)
	{       printf("Error: could not open directory %s\n",name1);
		return;
	}
	name1s = name1l;  name2s = name2l; /* save pointers */
	getentry(dirfd); /* skip first two entries */
	getentry(dirfd); /* because they are the "." & ".." entries */
	while(getentry(dirfd))  /* get an entry */
	{       push_name();    /* and push it onto the name1 & name2 */
		getstat();
		switch (type1)
		{       case 0: /* just a file */
				if (ask(askflag,"copy file",name1)) cp();
				break;
			case 1: /* character-type special file */
			case 3: /* block-type special file */
				if (ask(askdirflag,"copy special file",name1)) sf();
				break;
			case 2: /* directory */
				if (recursiveflag) /* ignore if not */
					if (ask(askdirflag,"examine directory",name1))
						cd(); /* copy the directory */
				break;
			default:/* error */
				printf("Error: %s does not exist",name1);
				return;
		}
		name1l = name1s;  *name1l = 0; /* pop name1 */
		name2l = name2s;  *name2l = 0; /* pop name2 */
	}
	close(dirfd); /* close the file source-directory */
	if (mode == 4) /* if directory was created then change */
	{       chmod(name2,modeflags);
		if (ownerflag) chown(name2,(stat1.gid<<8)|(stat1.uid));
	}
	return;
}


sf() /* copy a special file */
{       if (suflag)
	{       printf("Error: Only super user can copy special files.\n");
		return;
	}
	mknod(name2,stat1.flags,stat1.addr); /* make the node */
	if (ownerflag) chown(name2,(stat1.gid<<8)|stat1.uid);
	getstat();
	if(type1        != type2        ||
	   stat1.flags  != stat2.flags  ||
	   stat1.addr   != stat2.addr   )
	{       printf("Error: could not create special file %s\n",name2);
		unlink(name2);
	}
}

getentry(df)
int df; /* the directory file descriptor */
{       do
	{       if(read(df,direntry,16) != 16) return(0);
	} while (direntry[0] == 0);
	return(1);
}

push_name()
{       register char *s, *t;

	/* add entry onto name1 */
	if (name1l+17 > &name1[256])
	{       printf("Error file name too long %s\n",name1);
		exit(1);
	}
	s = name1l;  t = &direntry[1];  *s++ = '/';
	while (*s++ = *t++);
	name1l = --s;

	/* add entry onto name2 */
	if (name2l+17 > &name2[256])
	{       printf("Error file name too long %s\n",name2);
		exit(1);
	}
	s = name2l;  t = &direntry[1];  *s++ = '/';
	while (*s++ = *t++);
	name2l = --s;
}


cp()    /* works just like the cp command */
{
	static int buf[256];
	int fold, fnew;
	register int n;
	register char *s, *t;

	/* is target a directory? */
	if (type2 == 2) /* if so then pad name2 with direntry */
	{       s = name2l;        /* set to last char in destination name */
		*s++ = '/';        /* push a '/' */
		t = &direntry[1];  /* points to begin of source string */
		while(*s++ = *t++);/* add last part of name2 */
		getstat();
	}
	if (type2 != 4)
	{       if (newonlyflag) /* cannot process if flag is on and old */
		{       printf("Error: cannot overwrite %s\n",name2);
			return;
		}
		if (stat1.dev==stat2.dev)
		{       /* done if same device and inode */
			if (stat1.inumber==stat2.inumber)
				return;
			if (linkflag && unlink(name2) == -1)
			{       printf("Error: cannot remove %s\n",name2);
				return;
			}
		}
	}
	if (linkflag == 0 || link(name1,name2) == -1)
	{       if((fold = open(name1,0)) == -1)
		{       printf("Error: cannot open %s\n",name1);
			return;
		}
		if ((fnew = creat(name2,stat1.flags)) < 0)
		{       printf("Error: cannot create %s\n",name2);
			close(fold); return;
		}
		if (ownerflag && type2 == 4)
			chown(name2,(stat1.gid<<8)|(stat1.uid));
		while(n = read(fold,buf,512))
		{       if(n < 0)
			{       printf("Error: read error on %s\n",name1);
				unlink(name2);
				close(fold); close(fnew);
				return;
			}
			else if(write(fnew, buf, n) != n)
			{       printf("Error: write error on %s\n",name2);
				unlink(name2);
				close(fold); close(fnew);
				printf("%s aborting\n",name);
				exit(-1);
			}
		}
		close(fold);
		close(fnew);
	}
	if(moveflag) unlink(name1);   /* move option */
	return;
}

ask(f,s,t) int f;
{       char ch;
	register flg;
	if (f)  /* if flag true then ask question */
	{       printf("%s %s? ",s,t);
		if (read(0,&ch,1) != 1) return(0);
		flg = ch == 'y';
		while (ch != '\n' && read(0,&ch,1) == 1) ;
		return(flg);
	}
	else if (verboseflag) printf("%s %s\n",s,t);
	return(1);
}
