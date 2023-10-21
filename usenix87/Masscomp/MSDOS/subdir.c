/*
 * subdir(), getdir(), get_chain(), reset_dir()
 */

#include <stdio.h>
#include "msdos.h"

extern int dir_chain[25], dir_start, dir_len, dir_entries, clus_size;
extern char *mcwd;
static char *lastpath;

/*
 * Parse the path names of a sub directory.  Both '/' and '\' are
 * valid separators.  However, the use of '\' will force the operator
 * to use quotes in the command line to protect '\' from the shell.
 * Returns 1 on error.  Attempts to optimize by remembering the last
 * path it parsed
 */

subdir(name)
char *name;
{
	char *s, *tmp, *path, *malloc(), *strcpy(), *strcat();
	static int code;
	int getdir();
					/* if full pathname */
	if (*name == '/' || *name == '\\') {
		tmp = malloc(strlen(name)+1);
		strcpy(tmp, name);
	}
					/* if relative to MCWD */
	else {
		if (!strlen(name)) {
			tmp = malloc(strlen(mcwd)+1);
			strcpy(tmp, mcwd);
		}
		else {
			tmp = malloc(strlen(mcwd)+1+strlen(name)+1);
			strcpy(tmp, mcwd);
			strcat(tmp, "/");
			strcat(tmp, name);
		}
	}
					/* if paths are same, do nothing */
	if (!strcmp(tmp, lastpath))
		return(0);
					/* not recursive, start at root */
	reset_dir();
	lastpath = malloc(strlen(tmp)+1);
	strcpy(lastpath, tmp);
					/* zap the leading separator */
	if (*tmp == '\\' || *tmp == '/')
		tmp++;
	for (s = tmp; *s; ++s) {
		if (*s == '\\' || *s == '/') {
			path = tmp;
			*s = NULL;
			if (getdir(path))
				return(1);
			tmp = s+1;
		}
	}
	code = getdir(tmp);
	return(code);
}

/*
 * Find the directory and get the starting cluster.  A null directory
 * is ok.  Returns a 1 on error.
 */

int
getdir(path)
char *path;
{
	int entry, start, get_chain();
	char *newname, *unixname(), *strncpy(), name[9], ext[4];
	struct directory *dir, *search();

					/* nothing required */
	if (*path == NULL)
		return(0);

	for (entry=0; entry<dir_entries; entry++) {
		dir = search(entry);
					/* if empty */
		if (dir->name[0] == NULL)
			break;
					/* if erased */
		if (dir->name[0] == 0xe5)
			continue;
					/* skip if not a directory */
		if (!(dir->attr & 0x10))
			continue;
		strncpy(name, dir->name, 8);
		name[8] = NULL;
		strncpy(ext, dir->ext, 3);
		ext[3] = NULL;
		newname = unixname(name, ext);
		if (!strcmp(newname, path)) {
			start = dir->start[1]*0x100 + dir->start[0];
					/* if '..' pointing to root */
			if (!start && !strcmp(path, "..")) {
				reset_dir();
				return(0);
			}
					/* fill in the directory chain */
			dir_entries = get_chain(start) * 16;
			return(0);
		}
	}
					/* if '.' or '..', must be root */
	if (!strcmp(path, ".") || !strcmp(path, "..")) {
		reset_dir();
		return(0);
	}
	fprintf(stderr, "Path component '%s' is not a directory\n", path);
	return(1);
}

/*
 * Fill in the global variable dir_chain.  Argument is the starting
 * cluster number.  Info, in this variable is used by search() to 
 * scan a directory.  An arbitrary limit of 25 sectors is placed, this
 * equates to 400 entries.  Returns the number of sectors in the chain.
 */

int
get_chain(num)				/* fill the directory chain */
int num;
{
	int next;
	static int i;
	void exit();

	i = 0;
	while (1) {
		dir_chain[i] = (num - 2)*clus_size + dir_start + dir_len;
					/* sectors, not clusters! */
		if (clus_size == 2) {
			dir_chain[i+1] = dir_chain[i] + 1;
			i++;
		}
		i++;
		if (i >= 25) {
			fprintf(stderr, "get_chain: directory too large\n");
			exit(1);
		}
					/* get next cluster number */
		next = getfat(num);
		if (next == -1) {
			fprintf(stderr, "get_chain: FAT problem\n");
			exit(1);
		}
					/* end of cluster chain */
		if (next >= 0xff8) {
			break;
		}
		num = next;
	}
	return(i);
}

/* 
 * Reset the global variable dir_chain to the root directory.
 */

reset_dir()
{
	int i;

	for (i=0; i<dir_len; i++)
		dir_chain[i] = dir_start + i;
	dir_entries = dir_len * 16;
					/* disable subdir() optimization */
	lastpath = "\0";
	return;
}
