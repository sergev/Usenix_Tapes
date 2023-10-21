
/*
 * cptree -- copy a subtree of the directory hierarchy to another
 *	     place.
 */ 

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>

#define cycle	for(;;)
#define streq(a,b)	(strcmp((a), (b)) == 0)

#define READ	0
#define ABSOLUTE 0

#define OK	0
#define ERROR	-1

#define NAMELEN	512
char from[NAMELEN];
char to[NAMELEN];

#define ITABLESIZE 256
struct{
	ino_t oldinum;
	char *newname;
} itable[ITABLESIZE];
#define EMPTY	0
int full;
#define STRSIZE	2048
char strspace[STRSIZE];
char *strp;

int errs;
int dflg, lflg;
long int startime;

main(argc, argv)
int argc;
char *argv[];
{
	initialize(argc, argv);
	copytree();
	exit((errs == 0)?0:1);
}

initialize(argc, argv)
int argc;
char *argv[];
{
	dflg = lflg = 0;
	while((argc > 1) && (*argv[1] == '-')){
		if(streq(argv[1], "-d"))
			dflg++;
		else if(streq(argv[1], "-l"))
			lflg++;
		else
			usage();
		argv++;
		--argc;
	}
	time(&startime);
	if(argc != 3)
		usage();
	strcpy(from, argv[1]);
	strcpy(to, argv[2]);
	errs = 0;
	inititable();
}

inititable()
{
	int i;

	for(i = 0; i < ITABLESIZE; i++)
		itable[i].oldinum = EMPTY;
	full = 0;
	strp = strspace;
}

copytree()
{
	struct stat statbuf;
	int dirfd, index, n;
	unsigned short mode;
	long int lseekptr;
	char *endfrom, *endto, *lname;
	struct direct dirbuf;

	if(stat(from, &statbuf) == ERROR){
		printf("Can't find %s\n", from);
		errs++;
		return;
	}
	mode = statbuf.st_mode & S_IFMT;
	switch(mode){
	case S_IFDIR:
		if(statbuf.st_mtime > startime){
			printf("Skipping directory %s\n", from);
			errs++;
			break;
		}
		if((makedir(to, &statbuf) == ERROR)
		  && (stat(to, &statbuf) == ERROR)){
			printf("Can't find or create directory %s\n", to);
			errs++;
			return;
		}
		lseekptr = 2 * sizeof dirbuf;
		cycle{
			if((dirfd = open(from, READ)) == ERROR){
				printf("Can't read directory %s\n", from);
				errs++;
				return;
			}
			lseek(dirfd, lseekptr, ABSOLUTE);
			lseekptr += sizeof dirbuf;
			n = read(dirfd, (char *) &dirbuf, sizeof dirbuf);
			close(dirfd);
			if(n != sizeof dirbuf)
				break;
			if(dirbuf.d_ino == 0)
				continue;
			endfrom = from+strlen(from);
			endto = to+strlen(to);
			strcat(from, "/");
			strncat(from, dirbuf.d_name, DIRSIZ);
			strcat(to, "/");
			strncat(to, dirbuf.d_name, DIRSIZ);
			copytree();
			*endfrom = '\0';
			*endto = '\0';
		}
		break;

	default:
		if(dflg)
			break;
		if(lflg){
			if(makelink(from, to) == ERROR){
				printf("Can't link %s to %s\n", from, to);
				errs++;
			}
			break;
		}
		if(mode != S_IFREG){
			printf("Skipping special file %s\n", from);
			break;
		}
		if(statbuf.st_nlink <= 1)
			copyfile(from, to, &statbuf);
		else if((index = lookup(statbuf.st_ino)) != ERROR){
			lname = itable[index].newname;
			if(makelink(lname, to) == ERROR){
				printf("Can't link %s to %s\n", to, lname);
				errs++;
			}
		}
		else if((copyfile(from, to, &statbuf) == OK)
		  && (insert(statbuf.st_ino, to) == ERROR)){
			printf("Table overflow -- can't record links for %s\n", from);
			errs++;
		}
		break;

	}
}

makedir(dir, statp)
char *dir;
struct stat *statp;
{
	int status, i;

	i = fork();
	if(i == ERROR){
		printf("Can't fork to exec mkdir for %s\n", dir);
		return(ERROR);
	}
	if(i == 0){
		fclose(stdout);
		execl("/bin/mkdir", "mkdir", dir, 0);
		execl("/usr/bin/mkdir", "mkdir", dir, 0);
		fprintf(stderr, "Can't exec mkdir -- boy, are you in trouble.\n");
		exit(1);
	}
	wait(&status);
	status >>= 8;
	if(status == 0){
		chmod(dir, statp->st_mode);
		chown(dir, statp->st_uid, statp->st_gid);
		return(OK);
	}
	else
		return(ERROR);
}

#define BUFSIZE	512

copyfile(fromfile, tofile, statp)
char *fromfile, *tofile;
struct stat *statp;
{
	int fromfd, tofd, n, rc;
	char buf[BUFSIZE];

	rc = OK;
	if((fromfd = open(fromfile, READ)) == ERROR){
		printf("Can't open file %s\n", fromfile);
		errs++;
		return(ERROR);
	}
	if((tofd = creat(tofile, statp->st_mode)) == ERROR){
		printf("Can't create file %s\n", tofile);
		errs++;
		close(fromfd);
		return(ERROR);
	}
	chown(tofile, statp->st_uid, statp->st_gid);
	while((n = read(fromfd, buf, BUFSIZE)) > 0)
		if(write(tofd, buf, n) != n){
			printf("Can't write file %s\n", tofile);
			errs++;
			rc = ERROR;
			break;
		}
	if(n == ERROR){
		printf("Can't read from file %s\n", fromfile);
		errs++;
		rc = ERROR;
	}
	close(fromfd);
	close(tofd);
	return(rc);
}

makelink(name1, name2)
char *name1, *name2;
{
	if(link(name1, name2) == OK)
		return(OK);
	if(unlink(name2) == OK)
		return(link(name1, name2));
	else
		return(ERROR);
}

lookup(num)
ino_t num;
{
	int index, startindex, value;

	index = startindex = num%ITABLESIZE;
	do{
		value = itable[index].oldinum;
		if(value == num)
			return(index);
		else if(value == EMPTY)
			return(ERROR);
		index++;
		if(index >= ITABLESIZE)
			index = 0;
	} while(index != startindex);
	return(ERROR);
}

insert(num, file)
ino_t num;
char *file;
{
	int index, count;

	if(full)
		return(ERROR);
	index = num%ITABLESIZE;
	count = 0;
	while(itable[index].oldinum != EMPTY){
		count++;
		if(count > ITABLESIZE){
			full++;
			return(ERROR);
		}
		index++;
		if(index >= ITABLESIZE)
			index = 0;
	}
	if((strp+strlen(file)) >= strspace+STRSIZE)
		return(ERROR);
	itable[index].oldinum = num;
	strcpy(strp, file);
	itable[index].newname = strp;
	strp += strlen(file)+1;
	return(index);
}

usage()
{
	fprintf(stderr, "Usage: cptree [ -d ] [ -l ] fromdir todir\n");
	exit(1);
}
