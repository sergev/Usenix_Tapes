#

/*
 *      remove all
 *
 *      rmall - <directory>
 *
 *      The '-' switch if present prevents the top directory
 *      level from being deleted.
 */

#include	"/usr/sys/sbuf.h"

struct	inode	inode;
int     ksw,lev;

main(argc,argv)
char **argv;
{
	int pid;
	if(argc == 1) {
		printf("Syntax:  rmall [-] <directory>\n");
		exit();
	}
	argv++;
	if(argc > 2) {
		argv++;
		ksw++;
	}
	while((pid = fork()) == -1) sleep(1);
	if(pid)
		wait();
	else {
		rmall(*argv);  /* alters current dir */
		exit();
	}
	if(!ksw) rmdir(*argv); /* relative to original current dir */
}

rmall(nm)
char *nm;
{
	int i,fd;
	struct {
		int d_inode;
		char d_name[14];
		int d_z;
	}d;

	for(i=0;i<lev;i++)
		putchar(' ');
	printf("%s\n",nm);

	d.d_z = 0;
	if((fd = open(nm,0)) < 0) {
		printf("can't read directory %s\n",nm);
		return;
	}

	if(chdir(nm)){
		chmod(nm, 0777);
		if(chdir(nm)) {
			printf("can't chdir %s\n",nm);
			exit();
		}
	}
	lev++;
	while(read(fd,&d,16) == 16) {
		if(d.d_inode == 0) continue;
		if(stat(d.d_name, &inode) != 0) {
			printf("can't stat %s\n", d.d_name);
			exit();
		}
		if((inode.i_mode & IFMT) == IFDIR) {
			if(d.d_name[0] == '.' &&
			(d.d_name[1] == 0 || 
			(d.d_name[1] == '.' && d.d_name[2] == 0)))continue;
			rmall(d.d_name);
			rmdir(d.d_name);
		}else{
			if(unlink(d.d_name) != 0)
				printf("can't unlink %s\n",d.d_name);
		}
	}
	chdir("..");
	close(fd);
	lev--;
}
rmdir(nm)
char *nm;
{
	int pid;
	while((pid = fork()) == -1) sleep(1);
	if(pid)
		wait();
	else {
		execl("/bin/rmdir", "rmalldir", nm, 0);
		exit();
	}
}
