#
# include <file.h>

# define CR	13
# define NL	10
# define CTRL_Z 26

char *
ADDCR(file)
char *file; {
	int id, oid;
	char buf[512], *index();
	static char temp[11];
	char *end = "\r\n";
	int ln, cnt;
	register char *ptr = buf, *od;
	register i, count;

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
			
	while (ln = read(id,ptr,512)) {
		count = 0;
		while ( od = index(ptr,NL) ) {
			if (od > &buf[ln]) break;
			*od = '\0';
			cnt = strlen(ptr);
			count += cnt;
			write(oid, ptr, cnt);
			write(oid, end, 2);
			ptr = ++od;
			count++;
		}
		if (count = ln - count)
			write(oid,ptr,count);
		ptr = buf;
	}
	*ptr = CTRL_Z;
	write(oid,ptr,1);

	close(oid); close(id);
	return(temp);
}
