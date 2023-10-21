#include "/usr/sys/filsys.h"
#include <stdio.h>

/* args are 1:  fs name, 2: inumber */
main(argc, argv)
char *argv[];
{
	int buf[256], j, ino;
	FILE *f;

	if(argc < 3) error("argc");
	ino = atoi(argv[2]);
	if((f = fopen(argv[1], "r")) == NULL) error("open");
	if(fseek(f, 512L, 0) < 0) error("seek");
	if(fread(buf, 1, 512, f) < 512) error("read");
	buf->s_infl = ino;
	fclose(f);
	if((f = fopen(argv[1], "w")) == NULL) error("2nd open");
	if(fseek(f, 512L, 0) < 0) error("2nd seek");
	if(fwrite(buf, 1, 512, f) < 512) error("write");
		else printf("write successful\n");
}
error(str) {
	extern errno;
	printf("error %s %d\n", str, errno);
	exit();
}
