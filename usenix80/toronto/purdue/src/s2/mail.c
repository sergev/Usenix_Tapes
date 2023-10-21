#

/* mail command usage
 *	mail [-yn]
 *		prints your mail
 *	mail [+-] people ...
 *		sends standard input to people
 *		optional "-" invokes editor to accept text
 *		optional "+" invokes XED to accept text
 *
 *	77/07/22 - added SUID to root for secure mailboxes
 *		- TGI
 *	77/09/16 - changed search for .mail and mbox
 *		from current directory to "home" directory
 *		- TGI
 *	77/09/17 - added editor call for accepting text
 *		- TGI
 *	77/12/28 - added editor interface protocol
 *		including SIGTRC (#5 trace/BPT)
 *		code into /bin/ed.  Fixed bug with
 *		dead.letter stuff.
 *		- TGI
 *	78/04/30 - changed check() to allow sending
 *		to a mailbox with links != 1
 *		if owned by sendee.
 *		- TGI
 *	78/11/14 - added "mail + people" to call XED
 *		- TGI
 *	79/04/13 - added interlock files for each uid's
 *		mailbox, to avoid trash when many bulky
 *		parcels are stuffed into mailbox at once.
 *		Also, if "-f" specified as flag, will fork()
 *		upon EOF on typing in mail.
 *		- TGI
 *
 *	The SU permission allows sending to any user whose mailbox
 *	file (".mail") has -rw------- permission bits on (any others
 *	are ignored).  This means that other users cannot look at
 *	this mailbox, therefore "sensitive" info (passwd's, etc)
 *	may be sent via mail.  This also means that others cannot
 *	edit a mailbox in order to conceal the origin of the message.
 *
 *	If the destination mailbox seems suspicious in any way
 *	(links != 1, type != plain file), no message is sent.
 *	If the destination mailbox does not exist, it will
 *	not be created.  If mail can't be sent to any user,
 *	the message will be written on "dead.letter" in the
 *	current directory (if it is owned by sender, or
 *	world-write permission [ --------w- ] is enabled),
 *	or, if the sender is root, the message will go to
 *	"/tmp/dead.letter".
 */

#define SIGHUP	1
#define SIGINT	2
#define SIGQIT	3
#define SIGTRC	5
#define ROOT	0	/* super-user */
#define USER	13	/* user account */
#define ED	"/bin/ed"
#define XED	"/usr/bin/xed"

struct	inode {
	char	dmajor;
	char	dminor;
	int	inonum;
	int	i_mode;
	char	i_nlink;
	char	i_uid1;
	char	i_uid0;
	char	i_size0;
	char	*i_size1;
	int	i_addr[8];
	int	i_atime[2];
	int	i_mtime[2];
} statbuf;

#define IFTYPE	060000
#define IFDIR	040000
#define IREAD	000400
#define IWRITE	000200
#define WWRITE	000002

struct	utmp {
	char	name[8];
	char	tty;
	char	pad1;
	int	ltime[2];
	int	pad2;
};

struct	passwd {
	char	*pw_name;
	char	*pw_passwd;
	int	pw_uid;
	int	pw_gid;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

char	lettmp[]	"/tmp/ma00000";
char	preptmp[]	"/tmp/mb00000";
char	edittmp[]	"/tmp/mc00000";
char	m_lock[]	"/tmp/md00000";
int	pwfil;

int	uid	0;
int	to_uid	0;
int	dir_w	0;
char	dead[]		"/tmp/dead.letter";
char	*dead_letter	&dead[5];
char	mail_box[]	"/.mail";
char	m_box[]		"/mbox";
char	finish	0;
char	abort	0;
char	abort2	0;
char	intf	0;
int	intrupt();
int	quit();

main(argc, argv)
char **argv;
{
	register me;
	extern fout, delexit();
	register struct passwd *p;
	register char *cp;
/*	static struct utmp ubuf;	*/
	int uf;

	me = uid = getuid();
	maketemp();
	if ((intf = (signal(SIGINT, 1) & 01)) == 0) {
		signal(SIGHUP, 1);
		signal(SIGINT, delexit);
		signal(SIGQIT, quit);
	}
	intf = !intf;
	if (argc == 1 || argc == 2 && argv[1][0] == '-') {
		setuid(uid);
		printmail(argc, argv);
		delexit();
	}
	if ((me = uid) == USER) {
		printf("Illegal account for mailing.\n");
		exit(0);
	}
	if (stat(".", &statbuf) != -1)
		if (uid == ROOT || (((statbuf.i_uid1 << 8) & 0177400 |
		    (statbuf.i_uid0) & 0377) == uid &&
		    (statbuf.i_mode & IWRITE)) ||
		    (statbuf.i_mode & WWRITE))
			dir_w++;
	if (uid == ROOT)
		dir_w++;

	fout = creat(lettmp, 0600);
/*
	if (((me = ttyn(1)) != 'x' || (me = ttyn(2)) != 'x')
	    && (uf = open("/etc/utmp", 0)) > 0) {
		while (read(uf, &ubuf, sizeof ubuf) == sizeof ubuf)
			if (ubuf.tty == me) {
				ubuf.name[8] = ' ';
				close(uf);
				for (cp = ubuf.name; *cp++ != ' ';);
				*--cp = 0;
				bulkmail(argc, argv, ubuf.name);
			}
	}
 */
	me = uid;
	setpw();
	for (;;)
		if ((p = getpwent()) && p->pw_uid == me)
			bulkmail(argc, argv, p->pw_name);
	fout = 1;
	printf("Who are you?\n");
	flush();
	delexit();
}

printmail(argc, argv)
char **argv;
{
	extern fin, fout;
	struct passwd *p;
	register n, c, f;
	char cbuf[32], *s;
	int info;

	setpw();
	while (!((p = getpwent()) && p->pw_uid == uid));
	if(fopen(cat(p->pw_dir,mail_box),&fin)>=0 && (c=getchar())) {
		fout = dup(1);
		do {
			putchar(c);
		} while (c = getchar());
		flush();
		close(fin);
		if (argc < 2) {
			if (ttyn(0) != 'x') {
				printf("Save? ");
				flush();
				fin = 0;
				c = getchar();
			}
		} else
			c = argv[1][1];
		if (c == 'y' || c == 'n') {
			if (c == 'y') {
				copy(cat(p->pw_dir, m_box), cbuf);
				s = &m_lock[12];
				n = uid;
				while (s > &m_lock[7]) {
					*--s = n % 10 + '0';
					n =/ 10;
				}
				info = 1;
				if (intf)
					signal(SIGINT, intrupt);
				while ((n = creat(m_lock, 0)) < 0) {
					if (finish) {
						printf("interrupt\n");
						flush();
						return;
					}
					if (info && intf) {
						printf("Wait mailbox busy: %s\n",
						    p->pw_name);
						info = 0;
						flush();
					}
					sleep(5);
				}
				if (intf)
					signal(SIGINT, 1);
				prepend(cat(p->pw_dir, mail_box),
				    cbuf);
			}
			close(creat(cat(p->pw_dir, mail_box), 0600));
			unlink(m_lock);
		}
	} else
		printf("No mail.\n");
}

bulkmail(argc, argv, from)
char **argv, *from;
{
	extern fin, fout;
	int tbuf[2];
	int *savint, retcode;
	int pid, wpid;
	char *editor;
	register c;
	int forkf;

	fin = 0;
	(&fin)[1] = 0;
	time(tbuf);
	if (*argv[1]=='-' && argv[1][1]=='f' && argv[1][2]=='\0') {
		forkf++;
		argv++;
		--argc;
	}
	if ((*argv[1]=='-' || *argv[1]=='+') && argv[1][1]=='\0') {
		if (*argv[1] == '-')
			editor = ED;
		else
			editor = XED;
		if ((pid = fork()) == 0) {
			setuid(uid);
			close(fout);
			if ((pid = creat(edittmp, 0600)) < 0) {
				printf("Can't create temp file\n");
				exit(1);
			}
			execl(editor, "[mail]", "-nfm0", edittmp, 0);
			fout = 1;
			(&fout)[1] = 0;
			printf("Can't execute %s\n", editor);
			exit(1);
		}
		if (intf)
			savint = signal(SIGINT, 1);
		while (((wpid = wait(&retcode)) != -1 || abort2) &&
		    wpid != pid) {
			if (abort2)
				abort2 = 0;
			if (abort)
				kill(pid, SIGTRC);
		}
		if (abort || retcode) {
			fout = 1;
			(&fout)[1] = 0;
			printf("No mail sent.\n");
			flush();
			delexit();
		}
		if (intf)
			signal(SIGINT, savint);
		fopen(edittmp, &fin);
	}
	signal(SIGQIT, delexit);
	printf("From %s %s", from, ctime(tbuf));
	while (c = getchar())
		putchar(c);
	putchar('\n');
	flush();
	close(fout);
	fout = 1;
	if (*argv[1] == '-' || *argv[1] == '+') {
		argv++;
		argc--;
		close(fin);
		fin = 0;
		(&fin)[1] = 0;
	}
	if (*argv[1]=='-' && argv[1][1]=='f' && argv[1][2]=='\0') {
		forkf++;
		argv++;
		--argc;
	}
	if (forkf) {
		if ((pid = fork()) > 0)
			exit(0);
		if (pid == 0) {
			signal(SIGINT, 1);
			signal(SIGQIT, 1);
			intf = 0;
		}
	}
	while (--argc > 0)
		sendto(*++argv);
	delexit();
}

sendto(person)
char *person;
{
	static saved;
	extern fout;
	extern fin;
	register struct passwd *p;
	register *savint;
	char *c;
	int n, info;

	if (finish) {
		printf("Mail not sent to %s.\n", person);
		return;
	}
	if (intf)
		savint = signal(SIGINT, intrupt);
	setpw();
	while (p = getpwent()) {
		if (equal(p->pw_name, person)) {
			if (stat(cat(p->pw_dir,mail_box),&statbuf) < 0)
				break;
			c = &m_lock[12];
			n = p->pw_uid;
			while (c > &m_lock[7]) {
				*--c = n % 10 + '0';
				n =/ 10;
			}
			info = 1;
			while ((n = creat(m_lock, 0)) < 0) {
				if (finish) {
					printf("Mail not sent to %s\n",
					    person);
					return;
				}
				if (info && intf) {
					printf("Wait mailbox busy: %s\n",
					    person);
					info = 0;
					flush();
				}
				sleep(5);
			}
			if (prepend(lettmp,cat(p->pw_dir,mail_box))==0)
				break;
			unlink(m_lock);
			if (intf)
				signal(SIGINT, savint);
			return;
		}
	}
	flush();
	fout = 1;
	printf("Can't send to %s.\n", person);
	if (ttyn(0) != 'x' && saved == 0) {
		if (dir_w) {
			unlink(uid? dead_letter : dead);
			saved++;
			if (prepend(lettmp, uid? dead_letter : dead)) {
				printf("Letter saved in '%s'\n",
				    uid? dead_letter : dead);
				flush();
				chown(uid? dead_letter : dead, uid);
			}
		}
	}
	if (intf)
		signal(SIGINT, savint);
}

prepend(from, to)
char *from, *to;
{
	extern int fin, fout;

	if (check(from, 0) == 0 || check(to, 1) == 0)
		return(0);
	close(creat(preptmp, 0600));
	fcreat(preptmp, &fout);
	fopen(from, &fin);
	while (putchar(getchar()));
	close(fin);
	fopen(to, &fin);
	while (putchar(getchar()));
	close(fin);
	flush();
	close(fout);
	close(creat(to, 0600));
	if (fcreat(to, &fout) < 0) {
		fout = 1;
		return(0);
	}
	fopen(preptmp, &fin);
	while(putchar(getchar()));
	flush();
	close(fout);
	close(fin);
	fout = 1;
	return(1);
}

setpw() {
	extern fin;

	if (pwfil == 0) {
		fopen("/etc/passwd", &fin);
		pwfil = fin;
	} else
		fin = pwfil;
	(&fin)[1] = 0;
	seek(fin, 0, 0);
}

getpwent() {
	register char *p;
	register c;
	static struct passwd passwd;
	static char line[100];
	extern fin;

	p = line;
	while((c=getchar()) != '\n') {
		if(c <= 0)
			return(0);
		if(p < line+98)
			*p++ = c;
	}
	*p = 0;
	p = line;
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	to_uid = passwd.pw_uid = atoi(p);
	p = pwskip(p);
	passwd.pw_gid = atoi(p);
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	return(&passwd);
}

pwskip(ap)
char *ap;
{
	register char *p;

	p = ap;
	while(*p != ':') {
		if(*p == 0)
			return(p);
		p++;
	}
	*p++ = 0;
	return(p);
}

delexit() {
	extern fout;

	if (fout == 1)
		flush();
	unlink(lettmp);
	unlink(preptmp);
	unlink(edittmp);
	exit(0);
}

maketemp() {
	register i, pid, d;

	pid = getpid();
	for (i=11; i>=7; --i) {
		d = pid % 10 + '0';
		lettmp[i] = d;
		preptmp[i] = d;
		edittmp[i] = d;
		pid =/ 10;
	}
}

equal(as1, as2) {
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(1);
	return(0);
}

cat(ap1, ap2)
char *ap1, *ap2;
{
	register char *p1, *p2;
	static char fn[32];

	p1 = ap1;
	p2 = fn;
	while (*p2++ = *p1++);
	p2--;
	p1 = ap2;
	while (*p2++ = *p1++);
	return(fn);
}


check(as, cf) {
	register own;

	if (stat(as, &statbuf) < 0)
		return(1);
	own = (statbuf.i_uid1 << 8) & ~0377 | statbuf.i_uid0 & 0377;
	if ((statbuf.i_mode & IFTYPE) == IFDIR)
		return(0);
	if (statbuf.i_mode & WWRITE)
		return(1);
	if ((statbuf.i_mode & (IREAD | IWRITE)) == (IREAD | IWRITE)) {
		if (own == to_uid)
			return(1);
		if (statbuf.i_nlink == 1)
			return(1);
	}
	return(0);
}

copy(as1, as2)
char *as1, *as2;
{
	register char *p1, *p2;

	p1 = as1;
	p2 = as2;
	while (*p2++ = *p1++);
}

intrupt() {
	signal(SIGINT, 1);
	finish++;
}

quit() {
	signal(SIGQIT, 1);
	abort++;
	abort2++;
}
