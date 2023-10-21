#
/*
 * Handy inode editor - use for patcing the file system
 *
 * Last modified: May 6, 1980
 */

#include        "/usr/sys/h/ino.h"
#define FOREVER for(;;)

main(argc, argv)
int argc;
char *argv[];
{
int file, block, inode, i, *iptr, fld;
char buf[512];

if (argc < 3) {
	printf("Useage: edinode <inumber> <raw device>\n");
	exit(1);
	}

inode = atoi(argv[1]);

if (inode < 1) {
	printf("Bad inode number.\n");
	exit();
	}

block = (inode + 31) >> 4;

if ((file = open(argv[2], 0)) < 0) {
	printf("Can't open '%s' for reading.\n", argv[2]);
	exit(1);
	}

if (seek(file, block, 3) < 0) {
	printf("Seek error!\n");
	exit(1);
	}

if (read(file, buf, 512) < 512) {
	printf("Read error!\n");
	exit(1);
	}

close(file);

iptr = buf + (((inode + 31) << 5) & 0777);

printnode(iptr, 0);

FOREVER {
	printf("? ");
	fld = readnum(10);
	if (fld < 0)
		break;
	if (fld)
		modnode(iptr, fld);
	printnode(iptr, fld);
	}

if ((file = open(argv[2], 1)) < 0) {
	printf("Can't open '%s' for writing.\n", argv[2]);
	exit(1);
	}

if (seek(file, block, 3) < 0) {
	printf("Seek error!\n");
	exit(1);
	}

if (write(file, buf, 512) < 512) {
	printf("Can't write to '%s'.\n", argv[2]);
	exit(1);
	}

printf("\nok\n");
}

printnode(iptr, fld)
struct inode *iptr;
int fld;
{

switch (fld) {

	case 0:
	case 1:
		printf("1)          mode: 0%o\n", iptr->i_mode);
		if (fld)
			break;
	case 2:
		printf("2)         links: %d\n",  iptr->i_nlink);
		if (fld)
			break;
	case 3:
		printf("3)          user: %d\n",  iptr->i_uid);
		if (fld)
			break;
	case 4:
		printf("4)         group: %d\n",  iptr->i_gid);
		if (fld)
			break;
	case 5:
		printf("5)          size: %d\n",  iptr->i_size1);
		if (fld)
			break;
	case 7:
		printf("7)        access: %s", ctime(iptr->i_atime));
		if (fld)
			break;
	case 8:
		printf("8)  modification: %s", ctime(iptr->i_mtime));
	}
}

modnode(iptr, fld)
struct inode *iptr;
int fld;
{
register char c;

switch (fld) {

	case 1:
		iptr->i_mode = readnum(8);
		break;
	case 2:
		iptr->i_nlink = readnum(10);
		break;
	case 3:
		iptr->i_uid = readnum(10);
		break;
	case 4:
		iptr->i_gid = readnum(10);
		break;
	default:
		printf("Can't modify that field!\n");
		while ((c = getchar()) && c != '\n');   /* flush line */
	}
}

readnum(base)
int base;
{
register int  n;
register char c;

n = 0;

for (c = ' '; (c == ' ') || (c == '\t') || (c == ':'); c = getchar());

while (c >= '0' && c <= '9') {
	n = n*base+c-'0';
	c = getchar();
	}

return(n? n: (c <= 0)? -1: 0);
}
