#

/*	KT DAEMON
 *
 *
 * This program keeps track of system times by reading the
 * special device "/dev/kt" at specified intervals and averaging
 * This information every specified number of samples.  The
 * average values are written to the file /usr/adm/wtmp.
 *
 * The number of seconds between samples and the number of
 * samples per record can be specified as arguments to this
 * program.  The default values are 60 and 30 (sample every
 * minute, write out record every 30 minutes).
 *
 * This program should not be run for extended periods of time
 * since it tends to generate rather sizable files.
 *
 * This program will fork off a child to do the actual work
 * (always runs in the background).  It will attempt to do
 * a "clrtty" and will issue a nice(-5) if run by super-user.
 * This helps to keep it in memory and allows better response.
 *
 * The "as" routine "showps" is used to show ps the message
 * "[accounting]".
 *
 *
 * Author:  John Bruner
 *
 */

#include        "/usr/sys/stats.h"
#include	<ktd.h>
#define KTDEV   "/dev/kt"
#define UTMP    "/etc/utmp"

int wt;
char buf[16];
int     intvl   60;             /* Sample every 60 seconds */
int     num     30;             /* Take 30 samples before writing */
char    noargs  0;


main(argc, argv)
char **argv;
{

	register j, k;
	int fd, kt, val;


	/* Open files for I/O */

	if ((kt=open(KTDEV,0)) < 0){
		printf("Can't open %s\n", KTDEV);
		exit();
	}

	if ((fd=open(KTMP,2)) < 0 && (fd=creat(KTMP,0644)) < 0){
		printf("Can't create %s\n", KTMP);
		exit();
	}
	seek(fd, 0, 2);

	if ((wt=open(UTMP,0)) < 0){
		printf("Can't open %s\n", UTMP);
		exit();
	}


	/* If arguments were specified, determine sampling interval
	 * and number of intervals per record.
	 */


	if (argc == 1) noargs++;
	k = j = 0;
	while(--argc > 0 && j < 2){
		if (*argv[++k] < '0' || *argv[k] > '9') continue;
		if ((val = atoi(argv[k])) <= 0) continue;
		if (j++) num = val;
			else intvl = val;
	}


	/* If run by super-user, clear terminal and set priority
	 * down by 5 (to prevent "research" mode).
	 */

	while((k=fork()) < 0) sleep(5);
	if (k) exit();
	if (getuid() == 0){
		nice(-5);
		clrtty();
	}

	signal(2,1);            /* Ignore interrupts */
	signal(3,1);            /* Ignore quits */


	/* Close all terminal file descriptors */

	close(0);
	close(1);
	close(2);



	/* Change line 1 display (for ps) */

	showps("[accounting]",0);


	/* Keep track of sums over each minute, sampling
	 * each 5 seconds.
	 */

	while(1){
		for(j=0; j<NMON; j++) sum.cnt[j] = 0;
		for(j=0; j<NUSR; j++) sum.u_count[j] = 0;
		for(k=0; k<num; k++){
			sleep(intvl);
			seek(kt, 0, 0);
			read(kt, &stats,sizeof stats);
			for(j=0; j<NMON; j++)
				sum.cnt[j] =+ stats.mon_cnt[j];
			users();
		}
		for(j=0; j<NMON; j++) sum.cnt[j] =/ num;
		for(j=0; j<NUSR; j++) sum.u_count[j] =/ num;
		time(sum.timev);
		write(fd, &sum, sizeof sum);
	}
}
users(){

	register j;
	int count, count2, count3;

	count = count2 = count3 = 0;
	seek(wt, 0, 0);

	while(read(wt, buf, 16) == 16){
		if (buf[0] != 0) count++;
		for(j=0; j < (sizeof staff) - 1; j++)
			if (buf[8] == staff[j]) count2++;
		for(j=0; j < (sizeof techtyp) -1; j++)
			if (buf[8] == techtyp[j]) count3++;
	}

	sum.u_count[0] =+ count-count2-count3;
	sum.u_count[1] =+ count2;
	sum.u_count[2] =+ count3;
}
