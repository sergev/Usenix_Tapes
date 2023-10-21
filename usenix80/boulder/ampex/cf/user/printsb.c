/* Print out the superblock of a file system.  Args are:
		1. file system name
		2. (optional) "c..." for contiguous file format
		3. (optional) "s..." for short format
 */
#include "/usr/sys/cf.h"
#include "/usr/sys/filsys.h"
main(argc, argv)
char **argv;
{
	int *p, sb, i, fs[256], cflag, sflag, lim;
	if(argc < 2 || (sb = open(argv[1], 0)) < 0)
		error("arg");
	cflag = sflag = 0;
	for(i=2; i<argc; i++)
		switch(*argv[i]) {
			case 's':
				sflag++;
				break;
			case 'c':
				cflag++;
				break;
			default:
				printf("%s: unknown argument.\n", argv[i]);
				break;
		}
	seek(sb, 1, 3);
	i = read(sb, fs, 512);
	if(i<0)
		error("read");
	printf("Superblock of %s:\n", argv[1]);
	if(sflag) {
		p = &(fs->s_devc);
		goto shortp;
	}
	p = fs;
	printf("isize = %d\n", *p++);
	printf("fsize = %d\n", *p++);
	printf("nfree = %d\n", *p++);
	printf("free:");
	for(i=0; i<100; i++)
		printf("%s%6o", i%10?" ":"\n	", *p++);
	printf("\nninode = %d\ninode:", *p++);
	for(i=0; i<100; i++)
		printf("%s%6o", i%10?" ":"\n	", *p++);
	printf("\nflock, ilock = 0%o\n", *p++);
	printf("fmod, ronly = 0%o\n", *p++);
	printf("time = 0%o 0%o\n", *p++, *p++);
shortp:
    if(cflag) {
	printf("devc = 0%o\n", *p++);
	printf("devb = 0%o\n", *p++);
	printf("cfb0 = %d\n", *p++);
	printf("cfsize = %d\n", *p++);
	printf("chunksiz = %d\n", *p++);
	printf("bound = %d\n", *p++);
	printf("flsiz = %d\n", *p++);
	printf("infl = %d\n", *p++);
	printf("rootlbn = %d\n", *p++);
    }
	if(sflag)
		return;
	printf("pad:");
	lim = cflag? 39:48;
	for(i=0; i<lim; i++)
		printf("%s%6o", i%10?" ":"\n	", *p++);
	printf("\n");
}

error(str){printf("%s\n", str); exit();}
