/*
 * Write (copy) a Unix file to MSDOS
 *
 * Emmet P. Gray			US Army, HQ III Corps & Fort Hood
 * ...!ihnp4!uiucuxc!fthood!egray	Attn: AFZF-DE-ENV
 * 					Directorate of Engineering & Housing
 * 					Environmental Management Office
 * 					Fort Hood, TX 76544-5057
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
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

int full = 0;
int textmode = 0;
int nowarn = 0;
int filesize;

main(argc, argv)
int argc;
char *argv[];
{
	extern int optind;
	extern char *optarg;
	int i, entry, ismatch, nogo, slot, start, dot, subdir(), single;
	int isdir(), root, c, oops;
	char *filename, *newfile, tname[9], text[4], *fixname(), *getname();
	char *unixname(), ans[10], *strncpy(), *pathname, *getpath(), *fixed;
	char *tmp, *malloc(), *strcat(), *strcpy();
	void exit();
	struct directory *dir, *search(), *writeit();

	if (init(2)) {
		fprintf(stderr, "mwrite: Cannot initialize diskette\n");
		exit(1);
	}
					/* get command line options */
	oops = 0;
	while ((c = getopt(argc, argv, "tn")) != EOF) {
		switch(c) {
			case 't':
				textmode = 1;
				break;
			case 'n':
				nowarn = 1;
				break;
			default:
				oops = 1;
				break;
		}
	}

	if (oops || (argc - optind) < 2) {
		fprintf(stderr, "Usage: mwrite [-t|-n] <Unix file> <MSDOS file>\n");
		fprintf(stderr, "    or mwrite [-t|-n] <Unix file> [<Unix files...>] <MSDOS directory>\n");
		exit(1);
	}
	root = 0;
	if (!strcmp(argv[argc-1], "/") || !strcmp(argv[argc-1], "\\"))
		root = 1;
	filename = getname(argv[argc-1]);
	pathname = getpath(argv[argc-1]);
					/* test if path is ok first */
	if (subdir(pathname))
		exit(1);
					/* test if last argv is a dir */
	if (isdir(filename) || root) {
		if (!strlen(pathname)) {
					/* don't alter the presence or */
					/* absence of a leading separator */
			tmp = malloc(strlen(filename)+1);
			strcpy(tmp, filename);
		}
		else {
			tmp = malloc(strlen(pathname)+1+strlen(filename)+1);
			strcpy(tmp, pathname);
			strcat(tmp, "/");
			strcat(tmp, filename);
		}
					/* subdir is not recursive */
		subdir(tmp);
		single = 0;
	}
	else
		single = 1;

	for (i=optind; i<argc-1; i++) {
		if (single) {
			fixed = fixname(argv[argc-1]);
			filename = unixname(fixed, fixed+8);
		}
		else {
			fixed = fixname(argv[i]);
			filename = unixname(fixed, fixed+8);
			printf("Copying %s\n", filename);
		}
					/* see if exists and get slot */
		ismatch = 0;
		slot = -1;
		dot = 0;
		nogo = 0;
		for (entry=0; entry<dir_entries; entry++) {
			dir = search(entry);
					/* is empty */
			if (dir->name[0] == NULL) {
				if (slot < 0)
					slot = entry;
				break;
			}
					/* is erased */
			if (dir->name[0] == 0xe5) {
				if (slot < 0)
					slot = entry;
				continue;
			}
			strncpy(tname, dir->name, 8);
			strncpy(text, dir->ext, 3);
			newfile = unixname(tname, text);
					/* save the '.' entry info */
			if ((dir->attr & 0x10) && !strcmp(".", newfile)) {
				dot = dir->start[1]*0x100 + dir->start[0];
				continue;
			}
					/* is dir or volume lable */
			if ((dir->attr & 0x10) || (dir->attr & 0x08))
				continue;
					/* if file exists, delete it first */
			if (!strcmp(filename, newfile)) {
				ismatch = 1;
				start = dir->start[1]*0x100 + dir->start[0];
				if (nowarn) {
					zapit(start);
					dir->name[0] = 0xe5;
					writedir(entry, dir);
					if (slot < 0)
						slot = entry;
				} else {
					while (1) {
						printf("File '%s' exists, overwrite (y/n) ? ", filename);
						gets(ans);
						if (ans[0] == 'n' || ans[0] == 'N') {
							nogo = 1;
							break;
						}
						if (ans[0] == 'y' || ans[0] == 'Y') {
							zapit(start);
							dir->name[0] = 0xe5;
							writedir(entry, dir);
							if (slot < 0)
								slot = entry;
							break;
						}
					}
				}
			}
			if (ismatch)
				break;
		}
		if (nogo)		/* chickened out... */
			continue;
					/* no '.' entry means root directory */
		if (dot == 0 && slot < 0) {
			printf(stderr, "mwrite: No directory slots\n");
			exit(1);
		}
					/* make the directory grow */
		if (dot && slot < 0) {
			if (grow(dot)) {
				fprintf(stderr, "mwrite: Disk full\n");
				exit(1);
			}
					/* first entry in 'new' directory */
			slot = entry;
		}
					/* write the file */
		dir = writeit(fixed, argv[i]);
		if (dir != NULL)
			writedir(slot, dir);

		if (full) {
			fprintf(stderr, "mwrite: Disk Full\n");
			break;
		}
		if (single)
			break;
	}
					/* write FAT sectors */
	writefat();
	close(fd);
	exit(0);
}

/*
 * Open the named file for write, create the cluster chain, return the
 * directory structure.
 */

struct directory *
writeit(fixed, path)
char *fixed;
char *path;
{
	FILE *fp;
	int size, fat, firstfat, oldfat, nextfat(), putcluster(), putfat();
	struct directory *mk_entry();
	static struct directory *dir;
	struct stat stbuf;

	if (stat(path, &stbuf) < 0) {
		fprintf(stderr, "mwrite: Can't stat '%s'\n", path);
		return(NULL);
	}
	filesize = stbuf.st_size;
	if (!(fp = fopen(path, "r"))) {
		fprintf(stderr, "mwrite: Can't open '%s' for read\n", path);
		return(NULL);
	}
	size = 0;
	firstfat = nextfat(0);
	if (firstfat == -1) {
		full = 1;
		return(NULL);
	}
	fat = firstfat;
	while (1) {
		size += putcluster(fat, fp);
		if (size >= filesize) {
			putfat(fat, 0xfff);
			break;
		}
		oldfat = fat;
					/* get next free cluster */
		fat = nextfat(oldfat);
		if (fat == -1) {
			putfat(oldfat, 0xfff);
			full = 1;
			break;
		}
		putfat(oldfat, fat);
	}
	fclose(fp);
	dir = mk_entry(fixed, 0, firstfat, size);
	return(dir);
}

/*
 * Write to the cluster from the named Unix file descriptor.
 */

int
putcluster(num, fp)
int num;
FILE *fp;
{
	long start;
	void exit(), perror();
	int buflen, c;
	static int current;
	char tbuf[CLSTRBUF];

	start = (num - 2)*clus_size + dir_start + dir_len;
	move(start);

	buflen = clus_size * MSECSIZ;
					/* '\n' to '\r\n' translation */
	if (textmode) {
		current = 0;
		while (current < buflen) {
			if ((c = fgetc(fp)) == EOF) {
					/* put a file EOF marker */
				tbuf[current] = 0x1a;
				break;
			}
			if (c == '\n') {
				tbuf[current++] = '\r';
				if (current == buflen)
					break;
				tbuf[current++] = '\n';
					/* make the file appear larger */
				filesize++;
			}
			else 
				tbuf[current++] = c;
		}
	}
	else {
		if ((current = fread(tbuf, sizeof(char), buflen, fp)) < 0) {
			perror("putcluster: fread");
			exit(1);
		}
					/* all files get an EOF marker */
		if (current != buflen) 
			tbuf[current+1] = 0x1a;
	}
	
	if (write(fd, tbuf, buflen) != buflen) {
		perror("putcluster: write");
		exit(1);
	}
	return(current);
}

/*
 * Returns next free cluster or -1 if none are available.
 */

int
nextfat(last)
int last;
{
	static int i;

	for (i=last+1; i<num_clus+2; i++) {
		if (!getfat(i))
			return(i);
	}
	return(-1);
}
