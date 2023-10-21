#include <puts.c>
#include <equal.c>
#define PASSWD "/etc/u-seek"
#define UFILE "/etc/user-file"
#define DL "/usr2/sys-usage/disk-limit"
int dlflag 0;
int fd, fd2, fd3;
main(argc,argv)
char **argv;
{
	register i, uid;
	if (argc == 1){
		puts("Usage: ",*argv," [-d] logname logname ...",0);
		exit(1);
	}

	if ((fd=open(PASSWD,0)) < 0){
		puts("Can't open password file",0);
		exit(0);
	}

	if((fd2=open(UFILE,0)) < 0){
		puts ("Can't open user-file",0);
		exit(0);
	}

	if((fd3=open(DL,0)) < 0){
		puts ("Can't open disk-limit file",0);
		exit(0);
	}

	while (--argc){
		argv++;
		if (**argv == '-')
			if ((*argv)[1] == 'd'){
				dlflag++;
				continue;
			}
		for (i=0;(*argv)[i];i++)
			if (i >= 8){
				(*argv)[i] = 0;
				break;
			}

		if ((uid=pfind(*argv)) >= 0){
			ufind(uid);
			if (dlflag)
				dl(*argv);
		}
	}
}
pfind(s)
char *s;
{

	register char *p, *ptr;
	register uid;

	sseek(fd,0,0);
	uid = -1;
	while ((ptr = read64(fd)) != -1){
		uid++;
		for (p=ptr;*p;p++)
			if (*p == ':'){
				*p = '\0';
				break;
			}
		if (equal(s,ptr)){
			*p = ':';
			puts(ptr,0);
			return(uid);
		}
	}
	puts(s,": not found in password file",0);
	return(-1);
}
ufind(uid)
{
/*  could check this, but too much time & code when
they are probably in here (cuz were in PASWWD file).

Unless messed up, but happens so rarely, & is ok , cuz if falls there,
will be picked up by getline <= 0 & prnts not in file.
this way of coding will save time (a branch test for every line)
for those normal in the passwd file */
	register char *ptr;
	sseek (fd2,0,0);

/* root not in user-file */
	if (uid == 0){
		puts("root	GOD	System Staff",0);
		return;
	}

	while (--uid > 0){
		getstr(fd2);
	}
	if ((ptr= getstr(fd2)) > 0)
		puts(ptr,0);
	else
		puts("Not in user-file", 0);
}
dl(s)
char *s;
{

	register char *ptr, *p, *p2;
	int found;

	sseek (fd3,0,0);
/* root not in disk-limit file, either */

	if (equal (s,"root")){
		puts ("root gets what he wants",0);
		return;
	}

	found = 0;
	while ((p2=ptr=getstr(fd3)) != -1){

/*would have been nice to  do this with a while (n--)
where n = 2, but need a modification to language by which
you can say break(2); to break out of 2 levels of a while!! */
	for (p = p2; *p != '\0' && *p != '/' ;p++);
		if (*p == '\0' || *++p == '\0')
			continue;

	for (p2 = p;*p != '\0' && *p != '/' ;p++);
		if (*p == '\0' || *++p == '\0')
			continue;

		for (p2=p;*p2 != '\0';p2++);
		*p2 = '\0';
		if (equal(p,s)){
			puts(ptr,0);
			found++;
		}
	}
	if (!found)
	puts ("Not in disk-limit file",0);
}
#define BUFSIZE 512		/* char buffer (read) size */
#define EOF     -1		/* eof or error */
static char buff[BUFSIZE], *place;
static nchars 0;
getchar(fd)
{
/*  returns a character; reads buffered */

	if (nchars <= 0)
		if ((nchars=read(fd,buff,BUFSIZE)) > 0)
			place = buff;
		else
			return(EOF);
	nchars--;
	return(*place++);
}
sseek(fd,i,j)
{
	seek (fd,i,j);
	nchars = 0;
}
read64(fd)
{

static char buf[1024];
static nblks;

	if (nchars <= 0){
		if ((nchars = read(fd,buf,1024)) <= 0)
			return(-1);
		nblks = (nchars =/ 64);
	}

	return(buf + (nblks-(nchars--)) * 64);
}
#define BUFLEN 128
getstr(fd)
{
/* returns a ptr to the next line (terminated by a \n or too long to 
fit in buffer ) in the input. 

it's arg is a file descriptor to the opened file.

returns -1 on an error or EOF.
*/
	static char lbuf[BUFLEN];
	register char *ptr, c;

	ptr = lbuf;
	while ((*ptr=getchar(fd)) > 0 && *ptr != '\n'){
		if (ptr >= lbuf + 127){
			while ((c=getchar(fd)) > 0
				&& c != '\n');
			break;
		}
		ptr++;
	}
	if (*ptr <= 0)
		return(-1);

	*ptr = '\0';
	return(lbuf);
}
