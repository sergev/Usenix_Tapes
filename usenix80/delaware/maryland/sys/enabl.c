#define TTYSFILE "/etc/ttys"
#define UTMP     "/etc/utmp"
#define CONSOLE  '8'
#define READ     0
#define RDWRI    2
#define YES      '1'
#define NO       '0'
#define RELATIVE 1

struct {                /* format of ttys file */
	char getty_flag;
	char tty_num;
	char tty_type;
	char nl;
	} ttybuf;

struct {
	char name[8];
	char tty;
	char filler1;
	int login_time[2];
	int filler2;
	} utmp_buf;

int ttys_fd, utmp_fd;
char ttyno, line_flag 0;

main(argc, argv)
int argc;
char *argv[];
{

if (argc < 2) {
	printf("Useage: %s <ttyno> [ <line type> ] \n", argv[0]);
	exit(1);
	}

if ((ttyno = *argv[1]) == CONSOLE) {
	printf("Can't change console\n");
	exit(1);
	}

line_flag = *argv[2];

if ((ttys_fd = open(TTYSFILE, RDWRI)) < 0) {
	perror("Can't open ttys file");
	exit(1);
	}

if (findtty()) {

	ttybuf.getty_flag = argv[0][0] == 'e'? YES: NO;
	if (line_flag)
		ttybuf.tty_type = line_flag;

	seek(ttys_fd, -sizeof(ttybuf), RELATIVE);

	if (ttybuf.getty_flag == NO)
		check_busy();

	if (write(ttys_fd, &ttybuf, sizeof(ttybuf)) < sizeof(ttybuf)) {
		printf("Can't write to %s\n", TTYSFILE);
		exit(1);
		}

	if (kill(1, 1) < 0) {
		printf("Can't signal init process\n");
		exit(1);
		}
	printf("ok\n");
	}
else
	printf("tty%c: nonexistent\n", ttyno);
}

/****************************************************************************/


findtty() {

while (read(ttys_fd, &ttybuf, sizeof(ttybuf)) == sizeof(ttybuf))
	if (ttybuf.tty_num == ttyno)
		return(1);

printf("Entry not found in ttys file.\n");
}

/****************************************************************************/

check_busy() {
register char *p;

if ((utmp_fd = open(UTMP, READ)) < 0) {
	perror("can't open utmp file");
	exit(1);
	}

while (read(utmp_fd, &utmp_buf, sizeof(utmp_buf)) == sizeof(utmp_buf))
	if (utmp_buf.tty == ttyno) {
		for (p = utmp_buf.name; *p && *p != ' '; p++);
		*p = 0;
		printf("%s is logged in on tty%c\n", utmp_buf.name, ttyno);
		exit(1);
		}
}

/****************************************************************************/
