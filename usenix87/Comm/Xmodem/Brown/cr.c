#
#include <file.h>

#define CR	13
#define NL	10

char *
LF2CR(file)
char *file; {
	int id, oid;
	char buf[512], *index();
	static char temp[11];
	register char *ptr = buf;
	register ln, count;

	strcpy(temp,"tempXXXXXX");
	mktemp(temp);
	if ((oid = open(temp,O_CREAT | O_WRONLY,0644)) < 0) {
		perror(temp);
		unlink(temp);
		exit(-1);
	}
	if ((id = open(file,O_RDONLY,0644)) < 0) {
		perror(file);
		unlink(temp);
		exit(-1);
	}
			
	count = 0;
	while (ln = read(id,ptr,512)) {
		while ( ptr = index(ptr,NL) ) {
			if (ptr > &buf[ln]) break;
			*ptr = CR;
		}
		write(oid,ptr = buf,ln);
		count += ln;
	}
	if (ln = count % 128) {
		count = 128 - ln;
		for (ln = 0, ptr = buf; ln < count ; ln++, *ptr++ = ' ');
		write(oid,buf,ln);
	}

	close(oid); close(id);
	return(temp);
}
