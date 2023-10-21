#

/*
 *	Special password file construction
 *
 *
 * This program reads the standard password file and creates a new
 * file, minus passwords, which contains a 64-byte record for each
 * uid, so that the file can be read with a "seek(fd, uid*64, 0)"
 * followed by "read(fd, buffer, 64)"
 *
 * This program requires read permission on the normal password
 * file and creation permission on the seek-format password file,
 * which usually implies that the user must be either "bin" or
 * "root"
 *
 */

#define	PASS	"/etc/6passwd"
#define	SEEK	"/etc/u-seek"

int fd;	/* File descriptor for seek-format file (to be created) */

main(){

	char c, line[80];
	int uid, count, k;
	extern fin;

	if ((fd=creat(SEEK,0644)) < 0){
		printf("Can't create %s\n", SEEK);
		exit();
	}
	close(fd);
	if ((fd=open(SEEK,2)) < 0){
		printf("Can't open %s for read/write\n", SEEK);
		exit();
	}


	if ((fin=open(PASS,0)) < 0){
		printf("Can't open %s for reading\n", PASS);
		close(fd);
		exit();
	}


	/* Main Loop:
	 * Read until the first colon and copy into "line".
	 * Skip to second colon, then copy to end of line.
	 * Decode user-id and check if already defined.
	 * Write new entry into seek-format file.
	 */

	while(1){
		count = c = 0;
		while((line[count++]=getchar()) != ':')
			if (line[count-1] == 0){
				close(fin);
				close(fd);
				exit();
			}
		while(getchar() != ':');
		line[count++] = ':';
		uid=0;
		while((line[count++]=getchar()) != ':')
			uid = uid * 10 + (line[count-1] - '0');
		while((line[count]=getchar()) != '\n') count++;
		while(count < 63) line[count++]=0;
		line[63]=0;
		seek(fd, uid/8, 3);
		seek(fd, (uid%8)*64, 1);
		read(fd, &c, 1);
		seek(fd, uid/8, 3);
		seek(fd, (uid%8)*64, 1);
		if (c != 0) dupl(uid, line);
			else write(fd, line, 64);
	}
}
dupl(uid, line)
int uid;
char line[];
{

	/* Print an error message if another user is already
	 * assigned to this uid.  
	 */
	char line2[64];
	int k;
	read(fd, line2, 64);
	for (k = 0; line[k] != ':'; k++);
	line[k]=0;

	for (k = 0; line2[k] != ':'; k++);
	line2[k]=0;

	printf("Duplicate names for uid %d: ", uid);
	printf("%s = %s\n", line, line2);
}
