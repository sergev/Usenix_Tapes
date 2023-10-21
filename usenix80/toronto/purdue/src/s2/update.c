#
/*
 * update - execute system "sync" call at predetermined intervals.
 * replaces earlier assembly version. Optional first arg is number to
 * seconds to sleep (default 30).
 * --ghg 03/10/78.
 *
 * Optional second arg is flag to check amount of free space in
 * filesystems.
 */

#include "/usr/sys/filsys.h"
#define NEWFS	1

struct filsys sblock;

struct {
	char    *device;        /* Name of device */
	char    *name;          /* Directory name mounted on device */
	char    avail;          /* == 0 if device not available */
	char    fstatus;        /* 0, 1, or 2 -- last status */
	char    istatus;        /* 0, 1, or 2 -- last inode status */
	char    fd;             /* filler */
	int     flow;           /* Low limit on free space */
	int     fcrit;          /* Critical limit on free space */
	int     ilow;           /* Low limit on inodes */
	int     icrit;          /* Critical limit on inodes */
} dev[] {
	"/dev/hp0", "root", 1, 0, 0, 0, 100, 50, 50, 10,
	"/dev/sj4", "root", 1, 0, 0, 0, 100, 50, 50, 10,
	"/dev/hp1", "/usr", 1, 0, 0, 0, 1000, 500, 400, 100,
	"/dev/hp2", "/b", 1, 0, 0, 0, 1000, 500, 400, 100,
	"/dev/hs0", "/tmp", 1, 0, 0, 0, 200, 100, 100, 10,
	"/dev/sj5", "/usr2", 1, 0, 0, 0, 1000, 500, 400, 100,
	"/dev/sj3", "/a", 1, 0, 0, 0, 1000, 500, 400, 100,
	0
};

#define ever ;;

main(argc, argv)
char *argv[];
{
	register i, k;
	int checkfree 0;

	while((i=fork()) < 0)
		sleep(10);
	if(i)
		exit(0);

	switch(argc){
	case 0:
	case 1: i = 30;                 /* Default time to 30 seconds */
		break;
	case 2: if (argv[1][0] == '-' && argv[1][1] == 'c'){
			checkfree++;
			i = 30;
			goto endcase;
		}
		if ((i=atoi(argv[1])) <= 0) i = 30;
		break;
	case 3: if ((i=atoi(argv[1])) <= 0) i= 30;
		if (argv[2][0] == '-' && argv[2][1] == 'c') checkfree++;
		break;
	}
	endcase:


#ifdef NEWFS

	/* Open all devices declared in the "dev" structure above for
	 * read.  If a device can't be opened, invalidate it so it won't
	 * be used later for checking.
	 */

	if (checkfree)
		for(k=0; dev[k].device; k++)
			if((dev[k].fd = open(dev[k].device, 0)) < 0){
				printf("UPDATE: CAN'T OPEN %s (%s)\n",
					dev[k].device, dev[k].name);
				dev[k].avail = 0;
			}
#endif

	for(ever) {
		sync();

#ifdef  NEWFS

		if (checkfree) for(k=0; dev[k].device; k++){
			if (!dev[k].avail) continue;
			seek(dev[k].fd, 1, 3);          /* To block 1 */
			if (read(dev[k].fd, &sblock, 512) < 512){
				dev[k].avail = 0;
				printf("UPDATE:  READ ERROR ON %s (%s)\n",
					dev[k].device, dev[k].name);
				continue;
			}
			if (dev[k].fstatus < 2
			   && sblock.s_tfree < dev[k].fcrit){
			    printf("** CRITICALLY LOW SPACE ON %s (%s) ",
					dev[k].device, dev[k].name);
			    printf("%d BLOCKS LEFT **\007\007\n",
					sblock.s_tfree);
			    dev[k].fstatus = 2;

			} else if (dev[k].fstatus != 1
			   && sblock.s_tfree < dev[k].flow
			   && sblock.s_tfree > dev[k].fcrit){
			    printf("** LOW SPACE ON %s (%s) ",
					dev[k].device, dev[k].name);
			    printf("%d BLOCKS LEFT **\007\007\n",
					sblock.s_tfree);
			    dev[k].fstatus = 1;

			} else if (dev[k].fstatus > 0
			   && sblock.s_tfree > dev[k].flow){
			    printf("** ADEQUATE SPACE ON %s (%s) ",
					dev[k].device, dev[k].name);
			    printf("%d BLOCKS LEFT **\n", sblock.s_tfree);
			    dev[k].fstatus = 0;
			}

			if (dev[k].istatus < 2
			   && sblock.s_tinode < dev[k].icrit){
			    printf("** CRITICALLY LOW INODES ON %s (%s) ",
					dev[k].device, dev[k].name);
			    printf("%d INODES LEFT **\007\007\n",
					sblock.s_tinode);
			    dev[k].istatus = 2;

			} else if (dev[k].istatus != 1
			   && sblock.s_tinode < dev[k].ilow
			   && sblock.s_tinode > dev[k].icrit){
			    printf("** LOW INODES ON %s (%s) ",
					dev[k].device, dev[k].name);
			    printf("%d INODES LEFT **\007\007\n",
					sblock.s_tinode);
			    dev[k].istatus = 1;

			} else if (dev[k].istatus > 0
			   && sblock.s_tinode > dev[k].ilow){
			    printf("** ADEQUATE INODES ON %s (%s) ",
					dev[k].device, dev[k].name);
			    printf("%d INODES LEFT **\n", sblock.s_tinode);
			    dev[k].istatus = 0;
			}
		}

#endif

		sleep(i);
	}
}
