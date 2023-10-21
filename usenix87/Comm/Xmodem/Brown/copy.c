#
#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>


/*
**	copy - copy a  subtree to a new directory
**	preserving ownership and dates (access and modification only)
**
**	Keith Thompson	- Feb 7, 1981
**	revised		- May 1, 1981	(added options)
**
**	Options:	(can only be used by superuser)
**
**		-o login_name	make all files owned by 'login_name'
**		-u login_name	make all user ownership 'login_name'
**		-g group_name	make all group ownership 'group_name'
*/

#define BSIZE 4096

#define DEST (argc-1)
#define SRC  (argc-2)

#define EXISTS 0
#define READABLE 4
#define WRITEABLE 2
#define EXECUTABLE 1

#define MODE 0666
#define READ 0

#define strncat strcatn
extern char *strcat();
extern char *strcpy();

int superuser;
int user;
int group;

main(argc,argv)
int argc;
char **argv;
{
	register i,j,badarg;
	register struct passwd *pw;
	extern struct passwd *getpwnam();

	signal(SIGINT,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	signal(SIGPIPE,SIG_IGN);

	user = getuid();
	superuser = user == 0;
	group = superuser ? 0 : getgid();

	badarg = 0;

	if(argc < 3) {
		fprintf(stderr,"usage: %s [ options ] src_dir dst_dir\n",
				argv[0]);
		exit(2);
	}

	if(superuser) for(i=1 ; i<SRC; i = j) {
		register char *ptr;

		ptr = argv[i];
		j = i+1;
		if(j >= SRC) {
			fprintf(stderr,"Invalid argument: %s\n",argv[i]);
			badarg++;
			break;
		}

		if(*ptr++ != '-') {
			fprintf(stderr,"Invalid argument: %s\n",argv[i]);
			badarg++;
			continue;
		}

		pw = getpwnam(argv[j]);
		j++;

		switch(*ptr) {

		case 'o':
			user = pw->pw_uid;
			group = pw->pw_gid;
			break;

		case 'u':
			user = pw->pw_uid;
			break;

		case 'g':
			group = pw->pw_gid;
			break;

		default:
			fprintf(stderr,"Bad option: %c\n",*(ptr-1));
			badarg++;
		}
	}

	if(badarg) exit(4);


	if(!access(argv[DEST],EXISTS)) {
		fprintf(stderr,"%s already exists\n",argv[DEST]);
		exit(2);
	}

	if(access(argv[SRC],READABLE)) {
		fprintf(stderr,"%s not readable\n",argv[SRC]);
		exit(2);
	}


	copy(argv[SRC],argv[DEST]);
}

copy(src,dst)
char *src,*dst;
{
	struct direct dirbuf;
	union {
		char *c;
		struct direct *d;
	} u_dir;
	struct stat statbuf;
	struct stat srcstat;
	char src_file[256],dst_file[256];
	int src_fd;

	u_dir.d = &dirbuf;

	if(stat(src,&srcstat)) {
		perror(src);
		exit(4);
	}

	if((srcstat.st_mode&S_IFMT) != S_IFDIR) {
		fprintf(stderr,"copy: %s is not a directory\n",src);
		exit(4);
	}

	/*
	**	src exists and is readable
	**	now make the target directory
	*/

	if(mkdir(dst)) {
		exit(4);
	}

	change(dst, &srcstat);

	if((src_fd = open(src,0)) < 0) {
		perror(src);
		exit(4);
	}

	while(read(src_fd,u_dir.c,sizeof(dirbuf)) == sizeof(dirbuf)) {

		/*
		**	test to see if the directory slot is in use
		*/

		if(!dirbuf.d_ino)  {
			continue;
		}

		/*
		**	skip "." & "..", mkdir takes care of them
		*/

		if(!strcmp(".",dirbuf.d_name)
		|| !strcmp("..",dirbuf.d_name)) {
			continue;
		}

		/*
		**	we have a file or directory to copy now
		*/

		mkname(src_file,src,dirbuf.d_name);
		mkname(dst_file,dst,dirbuf.d_name);

		if(stat(src_file,&statbuf)) {
			perror(src_file);
			exit(4);
		}

		if(!superuser && access(src_file,READABLE)) {
			fprintf(stdout,"copy: no access %s\n",src_file);
			continue;
		}

		/*
		**	do the copy depending on the file type
		**
		**	directory:	recursively copy the subtree
		**	regular:	copy the file
		**	special:	make a new node (super user only)
		*/

		switch((statbuf.st_mode)&S_IFMT) {

		case S_IFDIR:
			copy(src_file,dst_file);
			break;

		case S_IFCHR:
		case S_IFBLK:
			if(!superuser) break;

			fprintf(stdout,"making special file %s\n",dst_file);

			if(mknod(dst_file,(int) statbuf.st_mode,
				statbuf.st_rdev)) {
			}
			change(dst_file, &statbuf);
			utime(dst_file,&(statbuf.st_atime));
			if(chmod(dst_file,(int) statbuf.st_mode)) {
				perror(dst_file);
				exit(4);
			}
			break;

		case S_IFREG:
			filecopy(src_file,dst_file);
			change(dst_file, &statbuf);
			utime(dst_file,&(statbuf.st_atime));
			if(chmod(dst_file,(int) statbuf.st_mode)) {
				perror(dst_file);
				exit(4);
			}
			break;

		default:
			fprintf(stderr,"bad file type: %s\n",src_file);
		}

	}
	utime(dst,&(srcstat.st_atime));
	if(chmod(dst,(int) srcstat.st_mode)) {
		perror(dst_file);
		exit(4);
	}

	close(src_fd);
}


mkname(dst,str1,str2)
char *dst,*str1,*str2;
{
	dst[0] = '\0';
	strcat(dst,str1);
	strcat(dst,"/");
	strncat(dst,str2,14);

}

filecopy(src,dst)
char *src,*dst;
{
	register int src_fd,dst_fd;
	char block[BSIZE];
	register int size;

	if((src_fd = open(src,READ)) < 0) {
		perror(src);
		exit(4);
	}

	if((dst_fd = creat(dst,MODE)) < 0) {
		perror(dst);
		exit(4);
	}

	while((size = read(src_fd,block,BSIZE)) > 0) {
		if(write(dst_fd,block,size) != size) {
			perror(dst);
			exit(4);
		}
	}

	if(size < 0) {
		perror(src);
		exit(4);
	}

	if(close(src_fd)) {
		perror(src);
		exit(4);
	}
	if(close(dst_fd)) {
		perror(src);
		exit(4);
	}

}

change(file,status)
char *file;
struct stat *status;
{
	int u,g;

	u = user;
	g = group;

	if(superuser) {
		if(!user) {
			u = status->st_uid;
		}
		if(!group) {
			g = status->st_gid;
		}
	}
	if(chown(file, u, g)) {
		perror(file);
		exit(4);
	}
}


mkdir(d)
register char *d;
{
	char pname[128], dname[128];
	register i, slash = 0;

	pname[0] = '\0';
	for(i = 0; d[i]; ++i)
		if(d[i] == '/')
			slash = i + 1;
	if(slash)
		strncpy(pname, d, slash);
	strcpy(pname+slash, ".");
	if (access(pname, WRITEABLE)) {
		fprintf(stderr,"mkdir: cannot access %s\n", pname);
		return(-1);
	}

	if ((mknod(d, 040777, 0)) < 0) {
		fprintf(stderr,"mkdir: cannot make directory %s\n", d);
		return(-1);
	}
	strcpy(dname, d);
	strcat(dname, "/.");

	if((link(d, dname)) < 0) {
		fprintf(stderr, "mkdir: cannot link %s\n", dname);
		unlink(d);
		return(-1);
	}
	strcat(dname, ".");
	if((link(pname, dname)) < 0) {
		fprintf(stderr, "mkdir: cannot link %s\n",dname);
		dname[strlen(dname)] = '\0';
		unlink(dname);
		unlink(d);
		return(-1);
	}

	return(0);
}
