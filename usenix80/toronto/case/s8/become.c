/*
 *	become -- set real and effective user-id to the argument
 *		(also group ids)
 *
 *	e.g.
 *		become [-l] hasp [directory]
 *		commands ...
 *	The "commands" will be executed (via a shell) as
 *	 if the user "hasp" had executed it.
 *	If directory is specifier a chdir is executed.
 *	If the -l option is present a chdir is executed
 *	 to the new users login directory.
 *
 *	Only the super-user can use "become".
 */

char directory[50];
int	uid,
	gid -1;

# define SHELL	"/bin/sh"

main(argc,argv)
int argc;
char *argv[];
{
int lflag 0;

	if(argc < 2){
		printf("Usage: become [-l] user_name|uid [directory]\n");
		printf("commands to be executed...\n");
		exit(9);
	}
	while(argc > 1){
		if(*argv[1] == '-')
			switch(argv[1][1]){
			case 'l':
				lflag = 1;
				break;
			default:
				printf("bad option\n");
				exit(9);
			}
		else{
			if(isnum(argv[1])){
				if(((uid = atoi(argv[1])) < 0) | (uid > 99)){
					printf("invalid user id\n");
					exit(9);
				}
			}else{
				if(getnum(argv[1]) < 0){
					printf("invalid user name\n");
					exit(9);
				}
			}
			getbyuid();
			if(argc == 3){
				if(chdir(argv[2]) < 0)
					printf("warning - chdir failed\n");
			}else if(lflag)
				if(chdir(directory) < 0)
					printf("warning - chdir failed\n");
			break;
		}
		argc--;
		argv++;
	}
	setgid(gid << 8 | gid);
	setuid(uid << 8 | uid);
	execl(SHELL, "sh", 0);
	printf("become failure\n");
	exit(9);
}

# include "stdio.h"

getnum(s)
char *s;
{
int i;
char str[20];
register char *sp, c;
FILE *fd;

	i = -1;
	fd = fopen("/etc/passwd", "r");
	c = '\n';	// prime with a CR
	do{
		if(c == '\n'){
			sp = str;
			while((*sp = getc(fd)) != ':')
				if(*sp++ == EOF)
					goto RET;
			*sp = '\0';
			if(strcmp(str,s) == 0){
				while((c = getc(fd)) != ':')	// password
					if(c == EOF)
						goto RET;
				sp = str;
				while((*sp = getc(fd)) != ':')sp++;
				*sp = '\0';
				uid = i = atoi(str);
				sp = str;
				while((*sp++ = getc(fd)) != ':');	// GID
				*(--sp) = '\0';
				gid = atoi(str);
				while(getc(fd) != ':');	// GCOS
				sp = directory;
				while((*sp++ = getc(fd)) != ':');
				*(--sp) = '\0';
				goto RET;
			}
		}
	}while(c = getc(fd));
RET:
	fclose(fd);
	return(i);
}

isnum(s)
register char *s;
{

	while(*s){
		if(! isdigit(*s))
			return(0);
		else
			s++;
	}
	return(1);
}

getbyuid()
{
register char *p, *pd;
char buffer[256];
register i;
char gstring[10];
register char *pg;
	getpw(uid, buffer);
	if(gid == -1){
		p = buffer;
		for(i=0;i<3;i++)
			while(*p++ != ':');
		for(pg=gstring;(*pg = *p) != ':';pg++,p++);
		*pg = '\0';
		gid = atoi(gstring);
	}
	if(*directory == '\0'){
		while(*p++ != ':');
		pd = directory;
		while((*pd++ = *p++) != ':');
		*(--pd) = '\0';
	}
}
