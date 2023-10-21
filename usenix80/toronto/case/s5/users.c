/*
 *	users.c - return number of users on system
 *
 *	modified by: W. A. Shannon   11/25/77
 *
 *	Users looks through file /etc/utmp to count
 *	the number of users.  User "Nobody" is not
 *	counted, since he is a dummy user running
 *	the not-logged-in shell.
 */

users(){
	char buf[16];
	register cnt;
	register int file;

	file=open("/etc/utmp",0);
	for (cnt = 0; read(file, buf, sizeof buf) == sizeof buf;) {
		if(buf[0]!=0) {
			if (buf[0]!='N' || buf[1]!='o' || buf[2]!='b' ||
				buf[3]!='o' || buf[4]!='d' || buf[5]!='y')
				cnt++;
		}
	}
	close(file);
	return(cnt);
}
