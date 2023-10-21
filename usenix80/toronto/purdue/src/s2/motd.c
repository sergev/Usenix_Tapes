#
/*
 *	motd -- show new message of the day
 *
 * modified 27-mar-77, G Goble, convert to 16 bit uid's
 * modified 30-nov-77, J. Bruner, add specific messages for terminals
 * modified 19-dec-77, J. Bruner, add specific messages for users
 * modified 15-jun-78, G. Goble, add time of last login, bad pass attempts
 * modified 25-sep-78, C. Schleifer, add update flag '+', bad pass flag '-'
 */

/*
 * If motd's first arg is a '+', then update last login time.
 * If motd's first arg is a '-', then only bump bad password attempts.
 */

#define NLOG 32
#include "/usr/sys/sbuf.h"

struct{
	int uid;
	int pos1;
	char pos0;
	char cnt;
	int badpass;	/* number of bad attempts on password since last login */
	int tvec[2];	/* time of last login */
	int junk[2];
}  log[NLOG];

int     fout;
int	intflg;
int	spmsgf;		/* nz if spec mesg already given */
int	count;
int	update; 	/* nz if last login time should be changed */

char buf[512];
char *banner	"/usr2/news/banner";
char *logfile	"/etc/banner-log";
char *motd	"/etc/motd";

int fd1, fd2;

struct inode sbuf;

main(argc, argp)
char **argp;
{
	int indx;
	extern sigint();

	if (argc > 1)
		if( *argp[1] == '+' || *argp[1] == '-' )
			update++;
	if (argc != 2 || argp[1][0] != '-') {
		signal(2,sigint);
		fd1 = opn(motd, 0);
		display(fd1);
		close(fd1);
	}
	fd1 = opn(banner, 0);
	fd2 = opn(logfile, 2);
	fstat(fd1, &sbuf);
	indx = position(getuid());
	if (argc > 1 && argp[1][0] == '-')
		log[indx].badpass++;
	else {
		signal(2,sigint);
		display(fd1);
		prtime(indx);
	}
	if( update )
		write(fd2, &log[0], count);
	spmsg();
}

sigint()
{
	signal(2,1);
	intflg++;
}

position(user)
{
	register int i, j, k;

	while((count = read(fd2, &log[0], 512)) > 0){
		for(i=0; i<count/16; i++)
			if(log[i].uid == user)
				goto found;
	}

	/*
	 *	not in log file -- add him
	 */

	count = 16;
	log[0].uid = user;
	log[0].pos1 = sbuf.i_size1;
	log[0].pos0 = sbuf.i_size0;
	log[0].cnt = 0;
	log[0].badpass = 0;
	log[0].tvec[0] = 0;
	log[0].tvec[1] = 0;
	return(0);

	/*
	 *	already in log file:
	 *
	 *	1)  update his entry
	 *	2)  position banner file for latest message
	 *	3)  setup log buf so that it can be simply
	 *	    written out to record his new position
	 */

found:

	if(log[i].pos1 == sbuf.i_size1 && log[i].pos0 == sbuf.i_size0)
		spmsg();
	k = (log[i].pos1 >> 9) &0177;
	k =| (log[i].pos0 <<7);
	if(k != 0)
		seek(fd1, k, 3);
	seek(fd1, log[i].pos1&0777, 1);
	seek(fd2, -count, 1);
	log[i].pos1 = sbuf.i_size1;
	log[i].pos0 = sbuf.i_size0;
	log[i].cnt++;
	return(i);
}

opn(file, rw)
{
	register fd;

	if((fd = open(file,rw)) < 0) {
	        printf("can't open file %s\n", file);
		exit(1);
	}
	return(fd);
}

display(fd)
{
	register cnt;
	char buff[512];

	while((cnt = read(fd, buff, 512)) > 0) {
		if(intflg)
			return;
		write(1, buff, cnt);
	}
}

prtime(indx)
{
	register i;

	fout = dup(1);
	i = indx;
	putchar('\n');
	if (log[i].tvec[0] || log[i].tvec[1]) {
		if (log[i].badpass) {
			printf("%l bad attempt", log[i].badpass);
			if (log[i].badpass != 1)
				putchar('s');
			printf(" on your password since ");
		}
		printf("last login on %s", ctime(&log[i].tvec[0]));
	}
	time(&log[i].tvec[0]);
	log[i].badpass = 0;
	flush();
}

/*
 *	Print special message for this terminal if there is one
 */

#define TERM    3

spmsg(){

	register fd, len, k;
	char buf[512];
	char *file "ttyx";
	char name[80];

	signal(2,1);
	if(spmsgf++)
		return;
	if ((file[TERM] = ttyn(1)) == 'x')
		return;
	if (chdir("/usr2/news/trmsg") < 0)
		return;
	if ((fd = open(file,0)) >= 0) {
		while((len = read(fd, buf, 512)) > 0)
			write(1, buf, len);
		close(fd);
	}
	if (getpw(getuid(),name) < 0)
		return;
	for(k=0; k<64; k++) if (name[k] == ':')
		name[k] = 0;
	if (name[0] != 0 && (fd = open(name,0)) >= 0) {
		while((len = read(fd, buf, 512)) > 0)
			write(1, buf, len);
		close(fd);
	}
	return;
}
