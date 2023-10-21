/*
 *	tarscan.c
 *
 *	News archive tape scanner.
 *
 *	Reads a tar news archive, extracts tape file names and
 *	associated subject lines, and writes same to stdout.
 *
 *	Written for Ultrix, but will probably work at most BSD sites.
 *
 *	Paul L. Allen
 *	July, 1986
 */

#include <stdio.h>
#include <sys/file.h>
#include <strings.h>

main () {

#define TBLOCK	512
#define NAMSIZ	100

union hblock {
	char dummy [TBLOCK];
	struct header {
		char name [NAMSIZ];
		char mode [8];
		char uid[8];
		char gid [8];
		char size [12];
		char mtime [12];
		char chksum [8];
		char linkflag;
		char linkname [NAMSIZ];
	} dbuf;
} *myblock;

int size;
int nblocks;
int mt;
char *cptr, *tptr, filename[100];
int firstblock;

if ((mt=open("/dev/rmt0", O_RDONLY, 0)) == -1) {
	perror ("tarscan: /dev/rmt0");
	exit(0);
	}

firstblock = 0;
nblocks = 0;
while (myread(&myblock, TBLOCK, mt)) {
	if (nblocks) {
		nblocks--;
		if (firstblock) {
			firstblock = 0;
			myblock->dbuf.name[TBLOCK-1] = 0;
			cptr = myblock->dbuf.name;
			while (tptr=index(cptr, '\n')) {
				*tptr = 0;
				cptr[7] = 0;
				if (strcmp(cptr, "Subject") == 0) {
				    printf ("%s   %s\n", filename, cptr+8);
				    break;
				    }
				cptr = tptr+1;
				}
			}
		}
	else {
		if (myblock->dbuf.name[0] == 0) {
			exit (0);
			}
		if (myblock->dbuf.name[NAMSIZ-1]) {
			printf ("name > 100 bytes! aborting...\n");
			exit (1);
			}
		sscanf (myblock->dbuf.size, "%o", &size);
		if (myblock->dbuf.linkflag == '1') size = 0;
		nblocks = (size/TBLOCK) + ((size % TBLOCK) > 0);
		strcpy (filename, myblock->dbuf.name);
		firstblock = (nblocks > 0);
		}
	}
}

#define TAPEBLK 10240
char tapebuff [TAPEBLK];
int buffpos = TAPEBLK;

myread (buff, size, mt)

char **buff;
int size;
int mt;
{
	if (buffpos == TAPEBLK) {
		if (read (mt, tapebuff, TAPEBLK) == 0) {
			perror("tarscan");
			exit (1);
			}
		buffpos = 0;
		}
	*buff = &tapebuff[buffpos];
	buffpos+=TBLOCK;
	return (1);
}

