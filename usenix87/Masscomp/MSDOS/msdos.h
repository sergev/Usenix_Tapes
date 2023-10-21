/*
 * msdos common header file
 */

#define MSECSIZ	512			/* sector size */
#define MDIRSIZ	32			/* directory size */
#define CLSTRBUF 1024			/* largest cluster size */
#define	FLOPPY	"/dev/rflp"

struct directory {
	unsigned char	name[8];	/* file name */
	unsigned char	ext[3];		/* file extent */
	unsigned char	attr;		/* attribute byte */
	unsigned char	reserved[10];	/* ?? */
	unsigned char	time[2];		
	unsigned char	date[2];
	unsigned char	start[2];	/* starting cluster number */
	unsigned char	size[4];	/* size of the file */
};
