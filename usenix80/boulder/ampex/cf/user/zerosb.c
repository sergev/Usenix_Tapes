/* On the superblock of a normal file system, zero out the "unused"
   part in preparation for putting a contiguous file system there.
   Args are:	1. file system name
		2. (optional) "n..." to not print out junk found
 */
#include "/usr/sys/cf.h"
#include "/usr/sys/filsys.h"
main(argc, argv)
char **argv;
{
	int sb, *i, j, pflag;
	int fs[256];
	
	if(argc<2)
		error("arg count");
	sb = open(argv[1], 2);
	if(sb<0) {
		printf("can't open ");
		error(argv[1]);
	}
	pflag = 1;
	if(argc > 2 && *argv[2] == 'n')
		--pflag;
	if(seek(sb, 1, 3) < 0)
		error("seek");
	j = read(sb, fs, 512);
	if(j<0)
		error("read failed");
	for(i = &(fs->s_devc); i <= &(fs[256]); i++) {
		if(*i != 0 && pflag)
			printf("junk found = %6o.\n", *i);
		*i = 0;
	}
	seek(sb, 1, 3);
	j = write(sb, fs, 512);
	if(j<512) {
		printf("write error, %d bytes written.\n", j);
		exit(-1);
	}
	else	exit(0);
}

error(str)
{
	printf("%s\n", str);
	exit(-1);
}
