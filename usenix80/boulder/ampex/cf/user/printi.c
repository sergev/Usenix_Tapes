/* Print the inode associated with a file name.  Args are:
		1. file name 
	      	2. (optional) "c" for contiguous file format print
 */
#include "cf.h"
main(argc, argv)
char **argv;
{
	struct inodestat ip;
	int *p, i, j, contflag;
	char *c;
	struct{long l;};
	if(argc < 2)
		error("arg count");
	contflag = 0;
    for(j = 1; j < argc; j++) {
	printf("inode of file %s\n", argv[j]);
	i = stat(argv[j], &ip);
	if(i < 0)
		error("stat");
	if(argc > j+1 && !(strcmp(argv[j+1], "c"))) {
		j++;
		contflag++;
	}
	p = &ip;
	printf("dev = 0%o\n", *p++);
	printf("inumber = %d\n", *p++);
	printf("flags = 0%o\n", *p++);
	c = p;
	p =+ 2;
	printf("nlinks = 0%o,", *c++);
	printf(" uid = 0%o,", *c++);
	printf(" gid = 0%o\n", *c++);
	printf("size = 0%o %o\n", *c, *p++);
	if(contflag) {
		printf("addr[0] = %d\n", *p++);
		printf("contdev = 0%o\n", *p++);
		printf("blockdev = 0%o\n", *p++);
		printf("base = %ld\n",p->l);
		p =+ 2;
		printf("last = %ld\n", p->l);
		p =+ 2;
		printf("other = %d\n", *p++);
	}
	else {
		printf("addr = ");
		for (i=0; i<8; i++)
			printf("0%o ", *p++);
		printf("\n");
	}
	printf("atime = %ld\n", p->l);
	p =+ 2;
	printf("mtime = %ld\n", p->l);
    }
}

error(str){printf("error:  %s.\n", str); fflush(stdout); exit();}
