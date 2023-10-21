/*
 * Display a MSDOS directory
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
char *mcwd;			/* the current working directory */

main(argc, argv)
int argc;
char *argv[];
{
	int i, entry, files, blocks, subdir(), fargn, wide;
	long size;
	char name[9], ext[4], *date, *time, *convdate(), *convtime();
	char *strncpy(), *dirname, *getname(), *getpath(), *pathname, sep;
	char *newfile, *filename, *malloc(), *unixname(), volume[12];
	char *strcpy(), *strcat();
	void exit();
	struct directory *dir, *search();

	if (init(0)) {
		fprintf(stderr, "mdir: Cannot initialize diskette\n");
		exit(1);
	}
					/* find the volume label */
	reset_dir();
	for (entry=0; entry<dir_entries; entry++) {
		dir = search(entry);
		strncpy(name, dir->name, 8);
		strncpy(ext, dir->ext, 3);
					/* if empty */
		if (dir->name[0] == NULL)
			break;
					/* if not volume label */
		if (!(dir->attr & 0x08))
			continue;
		strcpy(volume, name);
		strcat(volume, ext);
		break;
	}
	if (volume[0] == NULL)
		printf(" Volume in drive has no label\n");
	else
		printf(" Volume in drive is %s\n", volume);
	fargn = 1;
	wide = 0;
					/* first argument number */
	if (argc > 1) {
		if (!strcmp(argv[1], "-w")) {
			wide = 1;
			fargn = 2;
		}
	}
					/* fake an argument */
	if (argc == fargn) {
		argv[argc] = ".";
		argc++;
	}
	files = 0;
	for (i=fargn; i<argc; i++) {
		filename = getname(argv[i]);
		pathname = getpath(argv[i]);
					/* move to first guess subdirectory */
					/* required by isdir() */
		if (subdir(pathname))
			continue;
		if (isdir(filename)) {
			dirname = malloc(strlen(argv[i])+1);
			strcpy(dirname, pathname);
			if (strcmp(pathname,"/") && strcmp(pathname, "\\")) {
				if (*pathname != NULL)
					strcat(dirname, "/");
			}
			strcat(dirname, filename);
					/* move to real subdirectory */
			if (subdir(dirname))
				continue;
			filename = "*";
		}
		if (*filename == NULL)
			filename = "*";
		if (*dirname == '/' || *dirname == '\\')
			printf(" Directory for %s\n\n", dirname);
		else if (!strcmp(dirname, "."))
			printf(" Directory for %s\n\n", mcwd);
		else {
			if (strlen(mcwd) == 1 || !strlen(dirname))
				sep = NULL;
			else
				sep = '/';
			printf(" Directory for %s%c%s\n\n", mcwd, sep, dirname);
		}
		for (entry=0; entry<dir_entries; entry++) {
			dir = search(entry);
			strncpy(name, dir->name, 8);
			strncpy(ext, dir->ext, 3);
			newfile = unixname(name, ext);
					/* if empty */
			if (dir->name[0] == NULL)
				break;
					/* if erased */
			if (dir->name[0] == 0xe5)
				continue;
					/* if a volume label */
			if (dir->attr & 0x08)
				continue;
			if (!match(newfile, filename))
				continue;
			files++;
			if (wide && files != 1) {
				if (!((files-1) % 5))
					putchar('\n');
			}
			date = convdate(dir->date[1], dir->date[0]);
			time = convtime(dir->time[1], dir->time[0]);
			size = dir->size[2]*0x10000 + dir->size[1]*0x100 + dir->size[0];
					/* is a subdirectory */
			if (dir->attr & 0x10) {
				if (wide)
					printf("%-15.15s", name);
				else
					printf("%8s     <DIR>      %s  %s\n", name, date, time);
				continue;
			}
			if (wide)
				printf("%-9.9s%-6.6s", name, ext);
			else
				printf("%8s %3s %8d   %s  %s\n", name, ext, size, date, time);
		}
		if (argc > 2)
			putchar('\n');
	}
	blocks = getfree() * MSECSIZ;
	if (!files)
		printf("File '%s' not found\n", filename);
	else
		printf("     %3d File(s)     %6ld bytes free\n", files, blocks);
	close(fd);
	exit(0);
}

/*
 * Get the amount of free space on the diskette
 */

int getfree()
{
	int i;
	static int total;

	total = 0;
	for (i=2; i<num_clus+2; i++) {
					/* if getfat returns zero */
		if (!getfat(i))
			total += clus_size;
	}
	return(total);
}
