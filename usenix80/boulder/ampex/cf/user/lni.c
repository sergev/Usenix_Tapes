/* link the inode of the free list of a c.f.s. onto the superblock 
   args are:
	name of file system
	inumber of free list
 */
#include <stdio.h>
#include "/usr/sys/filsys.h"
main(argc, argv)
char *argv[];
{
	struct filsys fs;
	int inumber, f;
	long nbytes;

	if(argc < 3)
		error("arg count");
	inumber = atoi(argv[2]);
	if(inumber < 2) {
		printf("%d ", inumber);
		error("i number");
	}
	f = fopen(argv[1], "w");
	if(f == NULL)
		error("open");
	nbytes = 512 + (((&fs.s_infl) - &fs) * 2);
	if(fseek(f, nbytes, 0) < 0)
		error("seek");
	if(fwrite(&inumber, 2, 1, f) < 2)
		error("write");
}

error(str) {
	extern errno;
	printf("error %d %s\n", errno, str);
}
