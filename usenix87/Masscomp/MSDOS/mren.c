/*
 * Rename an existing MSDOS file
 *
 * Emmet P. Gray			US Army, HQ III Corps & Fort Hood
 * ...!ihnp4!uiucuxc!fthood!egray	Attn: AFZF-DE-ENV
 * 					Directorate of Engineering & Housing
 * 					Environmental Management Office
 * 					Fort Hood, TX 76544-5057
 */

#include <stdio.h>
#include "msdos.h"

int fd;				/* the file descriptor for the floppy */
int dir_start;			/* starting sector for directory */
int dir_len;			/* length of directory (in sectors) */
int dir_entries;		/* number of directory entries */
int dir_chain[25];		/* chain of sectors in directory */
int clus_size;			/* cluster size (in sectors) */
int fat_len;			/* length of FAT table (in sectors) */
int num_clus;			/* number of available clusters */
unsigned char *fatbuf;		/* the File Allocation Table */
char *mcwd;			/* the Current Working Directory */

main(argc, argv)
int argc;
char *argv[];
{
	int entry, ismatch, subdir(), nogo, isdir();
	char *filename, *newfile, *fixname(), *strncpy(), *unixname();
	char *getpath(), *pathname, tname[9], text[4], *getname(), *target;
	char *new, ans[10], *temp, *strcpy();
	void exit();
	struct directory *dir, *search();

	if (init(2)) {
		fprintf(stderr, "mren: Cannot initialize diskette\n");
		exit(1);
	}
	if (argc != 3) {
		fprintf(stderr, "Usage: mren <MSDOS source file> <MSDOS target file>\n");
		exit(1);
	}
	filename = getname(argv[1]);
	pathname = getpath(argv[1]);
	if (subdir(pathname))
		exit(1);

	temp = getname(argv[2]);
	target = fixname(argv[2]);
	if (isdir(filename) && strcmp(target+8, "   ")) {
		strcpy(target+8, "   ");
		fprintf(stderr, "mren: Directory names may not have extentions\n");
	}
	new = unixname(target, target+8);
	nogo = 0;
					/* the name supplied may be altered */
	if (strcmp(temp, new)) {
		while (!nogo) {
			printf("Do you accept '%s' as the new file name (y/n) ? ", new);
			gets(ans);
			if (ans[0] == 'y' || ans[0] == 'Y')
				break;
			if (ans[0] == 'n' || ans[0] == 'N')
				nogo = 1;
		}
	}
	if (nogo)
		exit(0);
					/* see if exists and do it */
	ismatch = 0;
	for (entry=0; entry<dir_entries; entry++) {
		dir = search(entry);
					/* if empty */
		if (dir->name[0] == NULL)
			break;
					/* if erased */
		if (dir->name[0] == 0xe5)
			continue;
					/* you may rename a directory */
		strncpy(tname, dir->name, 8);
		strncpy(text, dir->ext, 3);
		newfile = unixname(tname, text);
		if (!strcmp(filename, newfile)) {
			ismatch = 1;
			strncpy(dir->name, target, 8);
			strncpy(dir->ext, target+8, 3);
			writedir(entry, dir);
		}
	}
	if (!ismatch) {
		fprintf(stderr, "mren: File '%s' not found\n", filename);
		exit(1);
	}
	close(fd);
	exit(0);
}
