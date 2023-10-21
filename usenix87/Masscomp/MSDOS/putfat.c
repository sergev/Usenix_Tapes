/*
 * putfat(), writedir(), zapit(), writefat()
 */

#include <stdio.h>
#include "msdos.h"
#undef DUP_FAT

extern int fd, fat_len, dir_chain[25];
extern unsigned char *fatbuf;

/*
 * Puts a code into the FAT table.  Is the opposite of getfat().  No
 * sanity checking is done on the code.  Returns a 1 on error.
 */

int
putfat(num, code)
int num;
unsigned int code;
{
/*
 *	|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
 *	| | | | | | | | | | | | | | | | | | | | | | | | |
 *	|  n.0  |  n.5  | n+1.0 | n+1.5 | n+2.0 | n+2.5 |
 *	    \_____  \____   \______/________/_____   /
 *	      ____\______\________/   _____/  ____\_/
 *	     /     \      \          /       /     \
 *	| n+1.5 |  n.0  |  n.5  | n+2.0 | n+2.5 | n+1.0 |
 *	|      FAT entry k      |    FAT entry k+1      |
 */
	int start;
					/* which bytes contain the entry */
	start = num * 3 / 2;
	if (start < 0 || start+1 > (fat_len * MSECSIZ))
		return(1);
					/* (odd) not on byte boundary */
	if (num % 2) {
		*(fatbuf+start) = (*(fatbuf+start) & 0x0f) + ((code << 4) & 0xf0);
		*(fatbuf+start+1) = (code >> 4) & 0xff;
	}
					/* (even) on byte boundary */
	else {
		*(fatbuf+start) = code & 0xff;
		*(fatbuf+start+1) = (*(fatbuf+start+1) & 0xf0) + ((code >> 8) & 0x0f);
	}
	return(0);
}

/*
 * Write a directory entry.  The first argument is the directory entry
 * number to write to.  The second is a pointer to the directory itself.
 * All errors are fatal.
 */

writedir(num, dir)
int num;
struct directory *dir;
{
	int skip, entry;
	void exit(), perror();
	static struct directory dirs[16];
					/* which sector */
	skip = dir_chain[num / 16];

	move(skip);
					/* read the sector */
	if (read(fd, (char *) &dirs[0], MSECSIZ) != MSECSIZ) {
		perror("writedir: read");
		exit(1);
	}
					/* which entry in sector */
	entry = num % 16;
					/* copy the structure */
	dirs[entry] = *dir;
	move(skip);
					/* write the sector */
	if (write(fd, (char *) &dirs[0], MSECSIZ) != MSECSIZ) {
		perror("writedir: write");
		exit(1);
	}
	return;
}

/*
 * Remove a string of FAT entries (delete the file).  The argument is
 * the beginning of the string.  Does not consider the file length, so
 * if FAT is corrupted, watch out!  All errors are fatal.
 */

zapit(fat)
int fat;
{
	int next;

	while (1) {
					/* get next cluster number */
		next = getfat(fat);
					/* mark current cluster as empty */
		if (putfat(fat, 0) || next == -1) {
			fprintf(stderr, "zapit: FAT problem\n");
			exit(1);
		}
		if (next >= 0xff8)
			break;
		fat = next;
	}
	return;
}

/*
 * Write the FAT table to the disk.  Up to now the FAT manipulation has
 * been done in memory.  All errors are fatal.  (Might not be too smart
 * to wait till the end of the program to write the table.  Oh well...)
 */

writefat()
{
	int buflen;

	move(1);
	buflen = fat_len * MSECSIZ;
	if (write(fd, fatbuf, buflen) != buflen) {
		perror("writefat: write");
		exit(1);
	}
#ifdef DUP_FAT
					/* the duplicate FAT table */
	if (write(fd, fatbuf, buflen) != buflen) {
		perror("writefat: write");
		exit(1);
	}
#endif DUP_FAT
	return;
}
