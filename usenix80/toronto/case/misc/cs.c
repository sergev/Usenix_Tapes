#
/*
 * cs: current status
 * Greg Ordy, CWRU, Fall 1978
 *
 * this program shows a users effective and real
 * id and group.
 * it is similar to "ug", but not quite so cryptic.
 * if a file name is given on the calling line,
 * cs lists the owner and group of the file.
 *
*/
int 	pwfd;	/* password file file descriptor */
int	gpfd;	/* group file file descriptor */
struct	buf	/* buffer structure for buffered file input */
{
	int	fildes;
	int	nleft;
	char	*nextp;
	char	buff[512];
}
	buf;
struct	inode	/* inode buffer for stat */
{
	char	pad1[7];
	char	uid;
	char	gid;
	char	pad2[27];
}
	inode;
char	temp[512];

#define		PASSWD		"/etc/passwd"
#define		GROUP		"/etc/group"

main(argc,argv)
char	*argv[];
{
register int	user,group;
if((pwfd = open(PASSWD,0)) < 0) err("Cannot open password file");
if((gpfd = open(GROUP,0)) < 0)  err("Cannot open group file");
if(argc > 1)
{
	if(stat(argv[1],&inode) < 0)
	{
		printf("%s",argv[1]);
		err(" :cannot stat");
	}
	printf("File: %s\n",argv[1]);
	printf("Owner: %s",getname(inode.uid,pwfd));
	printf("Group: %s\n",getname(inode.gid,gpfd));
	exit(0);
}
user = getuid();
group = getgid();
printf("User:   real: %s",getname(user,pwfd));
printf("effective: %s\n",getname(user >> 8,pwfd));
printf("Group:  real: %s",getname(group,gpfd));
printf("effective: %s\n",getname(group >> 8,gpfd));
exit(0);
}
err(pointer)
char	*pointer;
{
printf("%s\n",pointer);
exit(-1);
}
getname(id,fd)
{
register char	*p1,*p2,c;
buf.fildes = fd;
id =& 0377;
buf.nleft = 0;
seek(fd,0,0);
while(1)
{
	p1 = temp;
	while((c = getc()) != '\n' && c != -1)
		*p1++ = c;
	if(c == -1)
		return("unknown   ");
	p1 = temp;
	while(*p1++ != ':');
	while(*p1++ != ':');
	p2 = p1;
	while(*p1++ != ':');
	*(--p1) = 0;
	if(atoi(p2) != id)
		continue;
	p1 = temp;
	for(p2 = 0; p2 != 10; p2++)
	{
		if(c == -1)
		{
			*p1++ = ' ';
			continue;
		}
		if(*p1 == ':')
		{
			*p1++ = ' ';
			c = -1;
			continue;
		}
		p1++;
	}
	*p1 = 0;
	return(temp);
}
}
char getc()
{
if(buf.nleft == 0)
{
	if((buf.nleft = read(buf.fildes,&buf.buff[0],512)) < 0)
		return(-1);
	buf.nextp = &buf.buff[0];
}
buf.nleft--;
return(*buf.nextp++);
}
