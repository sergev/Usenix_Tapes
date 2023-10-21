/* unpack portable archives - written by Bill Welch */
#include <stdio.h>
#include <sys/types.h>

char buf[512];
main() {
	char name[80];
	struct utimbuf {
		time_t actime;
		time_t modtime;
		} times;
	long mtime;
	int uid, gid, mode;
	long len, i;
	FILE *fp;

	while(gets(buf) != NULL) {
		if (buf[strlen(buf) - 1] == '`') {
			printf("%s\n", buf);
			sscanf(buf, "%s %ld %d %d %o %ld", name, &mtime, &uid, &gid, &mode, &len);
			printf("%s %ld\n", name, len);
			fp = fopen(name, "w");
			for (i = 0; i < len; i++) {
				putc(getchar(), fp);}
			fclose(fp);
			times.actime = times.modtime = mtime;
			/* If you don't have utime, just remove next 4 lines */
			if( utime(name, &times) < 0 ) {
				perror("Can't modify date");}
			if( chmod(name, mode) < 0 ) {
				perror("Can't modify mode");}}}
	exit(0);}
