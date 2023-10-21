#
#include "/u/sys/sbuf.h"
#define	MYPERM	0200
#define	YOUPERM	02

char *pp;
main(argc,argv)
int argc;
char *argv[]; {

	struct {
		int inum;
		char name[15];
	} stbuf;
	struct inode dirs;
	int fd,owner,oldinum,oldev,newdev,uid,permbits;
	register i,any;

	if(argc == 1){
		printf("Usage: rmdir name [name] .....\n");
		exit(1);
	}
	for(i=1;i < argc;i++){
		pp = argv[i];
		if(eq(pp,".") || eq(pp,"/") || stat(pp,&dirs) < 0){
			error();
			continue;
		}
		if((dirs.i_mode & IFDIR) != IFDIR || dirs.inonum == 1){
			error();
			continue;
		}
		oldev = (dirs.dmajor << 8) | (0377 & dirs.dminor);
		oldinum = dirs.inonum;
		stat(".",&dirs);
		newdev = (dirs.dmajor << 8) | (0377 & dirs.dminor);
		if(oldinum == dirs.inonum && newdev == oldev){
			error();
			continue;
		}
		stat(cat(pp,"/.."),&dirs);
		owner = (dirs.i_uid1 << 8) | (0377 & dirs.i_uid0);
		uid = getuid();
		if(uid){
			if(uid == owner)
				permbits = MYPERM;
			else
				permbits = YOUPERM;
			if((dirs.i_mode & permbits) == 0){
				error();
				continue;
			}
		}
		fd=open(pp,0);
		any = 0;
		while(read(fd,&stbuf,16) > 0){
			if(stbuf.inum == 0)
				continue;
			if(eq(stbuf.name,".") || eq(stbuf.name,".."))
				continue;
			any++;
			break;
		}
		if(any){
			printf("%s not empty\n",pp);
			continue;
		}
		unlink(cat(pp,"/."));
		unlink(cat(pp,"/.."));
		unlink(pp);
	}
}
error()
{
	printf("Can't unlink %s\n",pp);
}
eq(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return(1);
	return(0);
}
cat(a,b)
char *a,*b;
{
	static char save[128];
	register char *x,*y,*sv;

	x = a;
	y = b;
	sv = save;
	while(*x)
		*sv++ = *x++;
	while(*y)
		*sv++ = *y++;
	*sv = 0;
	return(save);
}
