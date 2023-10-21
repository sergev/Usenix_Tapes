#

/*
 *	Seek-file generation program
 *
 * This program takes the contents of the file "/etc/passwd" and
 * generates as output the file "/etc/usrnam".  This file contains
 * 8-byte records containing the login-names for each uid in use.
 * 
 * This program will output on the standard output any accounts 
 * which share the same uid, and the first name will be used in
 * "/etc/usrnam".
 *
 * This program can only be run by a user with write permission on
 * the file "/etc/usrnam" and read permission on "/etc/passwd",
 * which usually requires either "bin" or "root".
 *
 */

#define PASSFILE "/etc/passwd"
#define SEEKFILE "/etc/u-index"
main(){

	int passwd, usrnam;
	int uid;
	extern fin;
	char c, name[8];

	if ((passwd=open(PASSFILE,0)) < 0){
		printf("Can't open password file\n");
		exit();
	}
	fin=passwd;

	if ((usrnam=creat(SEEKFILE,0644)) < 0){
		printf("Can't create seek-format file\n");
		exit();
	}
	close(usrnam);
	usrnam=open(SEEKFILE,2);

	while(scan(name, &uid) == 0){
		seek(usrnam, uid*8, 0);
		read(usrnam, &c, 1);
		if (c != 0) dupl(name, uid, usrnam); else {
			seek(usrnam, uid*8, 0);
			write(usrnam, name, 8);
		}
	}

	close(passwd);
	close(usrnam);
}
scan(name, uid)
char name[8];
int *uid;
{
	int j;

	for (j=0; j<8; j++) name[j]=0;

	return(scanf("%[^:] %*[:] %d %*[^\n] %*[\n]", name, uid));
}
dupl(name, uid, fd)
char name[8];
int uid, fd;
{
	char name2[8];

	seek(fd, uid*8, 0);
	read(fd, name2, 8);
	printf("Duplicate names for UID %d: ", uid);
	printf("%s = %s\n", name, name2);
}
