#

/*
 *      lock,unlock  -  lock and unlock files/directories
 */

#include "/usr/sys/sbuf.h"

struct  inode   inode;
int     mode,ownmode;
char    v0;

main(argc,argv)
char **argv;
{
	register i;
	register char *cp;
	char buf[128];
	if((v0 = **argv++) == 'u')
		mode = 0755;
	else
		mode = 0700;
	argc--;
	if(argc == 0) {
		if(getpw(getuid(),buf)) {
			printf("getpw error\n");
			exit();
		}
		for(i=0 ; buf[i] ; i++)
			if(buf[i] == ':') buf[i] = 0;
		cp = buf;
		for(i=0 ; i < 5 ; i++)
			while(*cp++ != 0);
		if(chmod(cp,mode))
			nochmod(cp);
		exit();
	}
	for(; argc-- ; argv++) {
		if(stat(*argv,&inode)) {         /* non existant */
			if(v0 == 'u') {
				printf("can't find %s\n",*argv);
				continue;
			}
			while((i = fork()) == -1)
				sleep(1);
			if(i == 0) {
				execl("/bin/mkdir","mkdir",*argv,0);
				exit();
			} else
				wait();
			if(chmod(*argv,mode))
				nochmod(*argv);
			continue;
		}
		if(inode.i_mode & IFMT == IFDIR) {           /* directory */
			if(chmod(*argv,mode))
				nochmod(*argv);
			continue;
		}
		ownmode = inode.i_mode & 0700;          /* else file */
		i = ((ownmode >> 6) & mode ) |
			((ownmode >> 3) & mode) | ownmode;
		if(chmod(*argv,i))
			nochmod(*argv);
	}
}

nochmod(s)
{
	printf("can't chmod %s\n",s);
}
