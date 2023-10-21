/*
 * Read (copy) a MSDOS file to Unix
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

long size;
long current;
int textmode = 0;
int nowarn = 0;

main(argc, argv)
int argc;
char *argv[];
{
	extern int optind;
	extern char *optarg;
	int fat, i, ismatch, entry, subdir(), single, c, oops;
	char *filename, *newfile, text[4], tname[9], *getname(), *unixname();
	char *strncpy(), *pathname, *getpath(), *target, *tmp, *malloc();
	char *strcat(), *strcpy();
	void perror(), exit();
	struct directory *dir, *search();
	struct stat stbuf;

	if (init(0)) {
		fprintf(stderr, "mread: Cannot initialize diskette\n");
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
		fprintf(stderr, "Usage: mread [-t|-n] <MSDOS file> <Unix file>\n");
		fprintf(stderr, "    or mread [-t|-n] <MSDOS file> [<MSDOS files...>] <Unix directory>\n");
		exit(1);
	}
					/* only 1 file to copy... */
	single = 1;
	target = argv[argc-1];
					/* ...unless last arg is a directory */
	if (!stat(target, &stbuf)) {
		if (stbuf.st_mode & 040000)
			single = 0;	
	}

	for (i=optind; i<argc-1; i++) {
		filename = getname(argv[i]);
		pathname = getpath(argv[i]);
		if (subdir(pathname))
			continue;
		ismatch = 0;
		for (entry=0; entry<dir_entries; entry++) {
			dir = search(entry);
					/* if empty */
			if (dir->name[0] == NULL)
				break;
					/* if erased */
			if (dir->name[0] == 0xe5)
				continue;
					/* if dir or volume lable */
			if ((dir->attr & 0x10) || (dir->attr & 0x08))
				continue;
			strncpy(tname, dir->name, 8);
			strncpy(text, dir->ext, 3);
			newfile = unixname(tname, text);
			fat = dir->start[1]*0x100 + dir->start[0];
			size = dir->size[2]*0x10000 + dir->size[1]*0x100 + dir->size[0];
					/* if single file */
			if (single) {
				if (!strcmp(newfile, filename)) {
					readit(fat, target);
					ismatch = 1;
					break;
				}
			}
					/* if multiple files */
			else {
				if (match(newfile, filename)) {
					printf("Copying %s\n", newfile);
					tmp = malloc(strlen(target)+1+strlen(newfile)+1);
					strcpy(tmp, target);
					strcat(tmp, "/");
					strcat(tmp, newfile);
					readit(fat, tmp);
					ismatch = 1;
				}
			}
		}
		if (!ismatch) {
			fprintf(stderr, "mread: File '%s' not found\n", filename);
			continue;
		}
	}
	close(fd);
	exit(0);
}

/*
 * Decode the FAT chain given the begining FAT entry, open the named Unix
 * file for write.
 */

readit(fat, target)
int fat;
char *target;
{
	char ans[10];
	void exit();
	FILE *fp;

	current = 0L;
	if (!nowarn) {
		if (!access(target, 0)) {
			while (1) {
				printf("File '%s' exists, overwrite (y/n) ? ", target);
				gets(ans);
				if (ans[0] == 'n' || ans[0] == 'N')
					return;
				if (ans[0] == 'y' || ans[0] == 'Y')
					break;
			}
		}
	}

	if (!(fp = fopen(target, "w"))) {
		fprintf(stderr, "mread: Can't open '%s' for write\n", target);
		return;
	}

	while (1) {
		getcluster(fat, fp);
					/* get next cluster number */
		fat = getfat(fat);
		if (fat == -1) {
			fprintf(stderr, "mread: FAT problem\n");
			exit(1);
		}
					/* end of cluster chain */
		if (fat >= 0xff8)
			break;
	}
	fclose(fp);
	return;
}

/*
 * Read the named cluster, write to the Unix file descriptor.
 */

getcluster(num, fp)
int num;
FILE *fp;
{
	int i, buflen, start;
	void exit(), perror();
	char buf[CLSTRBUF];

	start = (num - 2)*clus_size + dir_start + dir_len;
	move(start);

	buflen = clus_size * MSECSIZ;
	if (read(fd, buf, buflen) != buflen) {
		perror("getcluster: read");
		exit(1);
	}
					/* stop at size not EOF marker */
	for (i=0; i<buflen; i++) {
		current++;
		if (current > size) 
			break;
		if (textmode && buf[i] == '\r')
			continue;
		fputc(buf[i], fp);
	}
	return;
}
