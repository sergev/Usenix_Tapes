#include "/usr/sys/stats.h"

/*
 * kt - print out Kernel timing statistics
 */

main(argc, argv)
char *argv[];
{
	int fd;
	extern int fout;
	register i;

	if((fd=open("/dev/kt", 0)) < 0) {
		printf("Can't open /dev/kt\n");
		exit(1);
	}
	fout = dup(1);
printf("USER	SYS	SYSI	SWAP	IDLE	RPBSY	RPXFRS 	PC\n");

loop:
	seek(fd, 0, 0);
	read(fd, &stats, sizeof stats);
	for(i=0; i<NMON-1; i++)
		stats.mon_cnt[i] = stats.mon_cnt[i] * 100 / 60;
	printf("%d%%%\t%d%%\t%d%%\t%d%%\t%d%%\t%d%%\t%d\t0%o\n",
	stats.mon_cnt[USER_CNT],
	stats.mon_cnt[SYS_CNT],
	stats.mon_cnt[SYSI_CNT],
	stats.mon_cnt[SWAP_CNT],
	stats.mon_cnt[IDLE_CNT],
	stats.mon_cnt[RP04_CNT],
	stats.mon_cnt[RPXF_CNT],
	stats.ken_pc
	);
	flush();
	if(argc > 1) {
		sleep(atoi(argv[1]));
		goto loop;
	}
}
