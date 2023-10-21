#						/* Last modified: 10 NOV 78 */
/* this program will either create or kill the teletype process on the dc11
	the 'y' option causes it to create the process if it does not exist
	the 'n' option causes it to kill the process if it exists and there is no one logged in on the dc11
	no option causes it to assume the 'y' option if there is no process, and the 'n' option otherwise */

#define	DCLOC	0	/* location of DC11 control byte in /etc/ttys */
#define	DCCHAR	'0'	/* last character of the DC11 tty name */

int yflag 0, nflag 0, ttyfile, utmp;
char ttybyte, *p;

struct {
	char name[8];
	char tty;
	char filler1;
	int login_time[2];
	int filler2;
	} utmp_buf;

main(argc, argv)
int argc;
char *argv[];
{

if (argc > 2) {
	printf("useage: dctty y    or    dctty n\n");
	exit();
	}

if (argc == 2)
	switch(*argv[1]) {
		case 'y': yflag++;
			break;
		case 'n': nflag++;
			break;
		default:
			printf("useage: dctty y    or    dctty n\n");
			exit();
		}

if ((ttyfile = open("/etc/ttys", 2)) < 0) {
		perror("can't open /etc/ttys");
		exit();
		}

seek(ttyfile, DCLOC, 0);
if (read(ttyfile, &ttybyte, 1) < 1) {
	perror("can't read /etc/ttys");
	exit();
	}

switch (ttybyte) {
	case '0': if (nflag) {
			printf("there is no tty process on the dc11\n");
			}
		else {
			seek(ttyfile, DCLOC, 0);
			if (write(ttyfile, "1", 1) < 1) {
				perror("can't write to /etc/ttys");
				exit();
				}
			if (kill(1, 1) < 0) {
				perror("can't signal init process\n");
				exit();
				}
			printf("tty process created\n");
			}
		break;

	case '1': if (yflag) {
			printf("there is already a teletype process on the dc11\n");
			}
		else {
			if ((utmp = open("/etc/utmp", 0)) < 0) {
				perror("can't open /etc/utmp");
				exit();
				}
			while (read(utmp, &utmp_buf, 16) > 0) {
				if (utmp_buf.tty == DCCHAR) {
					for (p = utmp_buf.name; *p && *p != ' '; p++);
					*p = 0;
					printf("%s is logged in on the DC11\n", utmp_buf.name);
					exit();
					}
				}
			seek(ttyfile, DCLOC, 0);
			if (write(ttyfile, "0", 1) < 1) {
				perror("can't write to /etc/ttys");
				exit();
				}
			if (kill(1, 1) < 0) {
				perror("can't signal init process");
				exit();
				}
			printf("teletype process killed\n");
			}
		break;

	default:
		printf("error in /etc/ttys\n");
	}
}
