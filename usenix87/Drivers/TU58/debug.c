#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#define DBDIR	"/usr/local/tu"

FILE *dbfd;

char *ctime();

/* open a debug info file.  Use the major and minor dev num of the
 output serial port so that subsequent invocations of the tu58 emulator
 (for a given line) share a file name, and so that multiple
 simultaneous invocations don't share a file name */

db_open(fd)
{
char name[50];
struct stat buf;
long t;

fstat(fd,&buf);
sprintf(name,"%s/tu58debug.%d.%d",DBDIR,major(buf.st_rdev),minor(buf.st_rdev));
dbfd = fopen(name,"w");
if (dbfd == NULL)
	fprintf(stderr,"Cannot open %s for debug output\n",name);
time(&t);
fprintf(dbfd,"Tu58 starting at ");
fprintf(dbfd,ctime(&t));
fflush(dbfd);
}

db_close()
{
long t;

time(&t);
fprintf(dbfd,"Tu58 finished at ");
fprintf(dbfd,ctime(&t));
fflush(dbfd);
fclose(dbfd);
}
