char tty, usrname[100], *p usrname, filename[15], *t;
int fd;

main() {

tty = ttyn(0);
getpw((getuid() & 0377), usrname);
while (*p && (*p != ':'))
	p++;
*p = 0;
if ((fd = open(t = stringf("/tmp/resav%c.%s", tty, usrname), 0)) < 0) {
	printf("Can't find editor file.\n");
	printf("%s\n", t);
	exit(1);
	}
seek(fd, 022, 0);
read(fd, filename, 14);
printf("\nModifications to %s:\n", filename);
execl("/bin/diff", "diff", stringf("%s.bak", filename), filename, 0);
execl("/usr/bin/diff", "diff", stringf("%s.bak", filename), filename, 0);
printf("Can't exec diff.\n");
exit(1);
}
