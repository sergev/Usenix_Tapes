#include "cfs.h"

char *
mkmode(mode)
u_short mode;
{
	static char     ms[11];

	strcpy(ms, "----------");
	switch (mode & S_IFMT) {
	case S_IFREG:
	default:
		break;
	case S_IFDIR:
		ms[0] = 'd'; break;
	case S_IFCHR:
		ms[0] = 'c'; break;
	case S_IFBLK:
		ms[0] = 'b'; break;
	case S_IFLNK:
		ms[0] = 'l'; break;
	case S_IFSOCK:
		ms[0] = 's'; break;
	}

	if (mode & 00400) ms[1] = 'r';
	if (mode & 00200) ms[2] = 'w';
	if (mode & 00200) ms[3] = 'x';
	if (mode & 00040) ms[4] = 'r';
	if (mode & 00020) ms[5] = 'w';
	if (mode & 00010) ms[6] = 'x';
	if (mode & 00004) ms[7] = 'r';
	if (mode & 00002) ms[8] = 'w';
	if (mode & 00001) ms[9] = 'x';

	if (mode & S_ISUID) ms[3] = 's';
	if (mode & S_ISGID) ms[6] = 's';
	if (mode & S_ISVTX) ms[9] = 't';
	return(ms);
}
