#
/*
 * Bootdf - read/set default filename in system bootstrap.
 * Bootdf reads in the system bootstrap from block 0 of
 * the filesystem and enters a new default name and rewrites
 * the boot block.
 * Default boot filename must reside in last 16 bytes, left
 * justified, zero filled in the bootstrap block.
 *
 * Syntax:
 * 
 * bootdf filename [ filesystem ]
 *
 * If "filesystem" is null, default will be used. If "filename"
 * is null, the name of the current default bootup file will
 * be reported.  A "-" may be used as a place holder to examine
 * the default boot filename on a specified filesystem.
 *
 * --ghg 03/18/78.
 */

#define FILESYS	"/dev/hp5"	/* default bootup filesys */
#define DFPOS	512-16		/* position of default filename */
#define	NCHARS	16		/* number of chars in filename */

char	bootblk[512];		/* system bootstrap buffer */
char	*fsys	FILESYS;

main(argc, argv)
char *argv[];
{

	register fd;
	register char *cp, *fp;

	if (argc == 3)
		fsys = argv[2];
	if (argc >= 2 && argv[1][0] != '-') {
		/*
		 * Rewrite bootstrap with new filename
		 */
		if ((fd=open(fsys,2)) < 0) {
			printf("Can't open %s for write access\n", fsys);
			exit(1);
		}
		if (read(fd, &bootblk, 512) != 512) {
			printf("Read I/O error on boot block on %s\n", fsys);
			exit(1);
		}
		fp = argv[1];
		for (cp = &bootblk[DFPOS]; *fp; cp++)
			*cp = *fp++;
		while (cp < &bootblk[DFPOS+NCHARS])
			*cp++ = 0;
		seek(fd, 0, 0);
		if(write(fd, &bootblk, 512)  != 512) {
			printf("Write I/O error on boot block on %s\n",fsys);
			exit(1);
		}
	}
	else  {
		/*
		 * just report default boot filename
		 */
		if ((fd=open(fsys, 0)) < 0) {
			printf("Can't open %s for read\n", fsys);
			exit(2);
		}
		if (read(fd, &bootblk, 512) != 512) {
			printf("Read I/O error on boot block on %s\n", fsys);
			exit(1);
		}
		printf("%s\n", &bootblk[DFPOS]);
	}
}

