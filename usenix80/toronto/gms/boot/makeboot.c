/*
 * Makeboot creates a self-booting tape.
 * compile: cc -n -v -s -O makeboot.c -lS -o makeboot
 */

#include <exit.h>		/* standard exit status codes */
#include <stdio.h>		/* parameters and externals for standard i/o */

#define	BLCKSIZ	512		/* number of bytes in a block */

char	buf[BLCKSIZ];		/* buffer to hold data records */

char	*bootfil = {		/* name of boot program to copy */
	"/usr/adm/boot/selfboot"
};

char	*rootfil = {		/* name of root file system to copy */
	"/dev/rdp0"
};

char	*unixfil = {		/* name of operating system to copy */
	"/unix"
};

char	*tapefil = {		/* name of tape drive to write on */
	"/dev/rmt0-800"		/* NOTE: must be 800 BPI */
};

main() {
	register i;
	int	bootfd,rootfd,tapefd,unixfd;

/* open the boot program for reading */
	bootfd = open(bootfil,0);
	if(bootfd == -1) {
		fprintf(stderr,"cannot open %s for reading\n",bootfil);
		exit(EX_SYS);
	}
/* open the root file system for reading */
	rootfd = open(rootfil,0);
	if(rootfd == -1) {
		fprintf(stderr,"cannot open %s for reading\n",rootfil);
		exit(EX_SYS);
	}
/* open the operating system for reading */
	unixfd = open(unixfil,0);
	if(unixfd == -1) {
		fprintf(stderr,"cannot open %s for reading\n",unixfil);
		exit(EX_SYS);
	}
/* open the tape for writing */
	tapefd = open(tapefil,1);
	if(tapefd == -1) {
		fprintf(stderr,"cannot open %s for writing\n",tapefil);
		exit(EX_SYS);
	}
/* skip over the a.out file header in the boot program */
	if(seek(bootfd,16,0) == -1) {
		fprintf(stderr,"error seeking on %s\n",bootfil);
		exit(EX_SYS);
	}
/* read in a block from the boot program */
	if(read(bootfd,buf,BLCKSIZ) == -1) {
		fprintf(stderr,"error reading %s\n",bootfil);
		exit(EX_SYS);
	}
/* write out records 0 and 1 */
	if((write(tapefd,buf,BLCKSIZ) != BLCKSIZ) ||
	   (write(tapefd,buf,BLCKSIZ) != BLCKSIZ)) {
		fprintf(stderr,"error writing %s\n",tapefil);
		exit(EX_SYS);
	}
/* copy the file system to the tape */
	for(i = 5500;i;i--) {
		if(read(rootfd,buf,BLCKSIZ) != BLCKSIZ) {
			fprintf(stderr,"error reading %s\n",rootfil);
			exit(EX_SYS);
		}
		if(write(tapefd,buf,BLCKSIZ) != BLCKSIZ) {
			fprintf(stderr,"error writing %s\n",tapefil);
			exit(EX_SYS);
		}
	}
/* skip over the a.out file header in the operating system */
	if(seek(unixfd,16,0) == -1) {
		fprintf(stderr,"error seeking on %s\n",unixfil);
		exit(EX_SYS);
	}
/* copy the operating system to the tape */
	for(i = 110;i;i--) {
		if(read(unixfd,buf,BLCKSIZ) == -1) {
			fprintf(stderr,"error reading %s\n",unixfil);
			exit(EX_SYS);
		}
		if(write(tapefd,buf,BLCKSIZ) != BLCKSIZ) {
			fprintf(stderr,"error writing %s\n",tapefil);
			exit(EX_SYS);
		}
	}
	exit(EX_OK);
}
