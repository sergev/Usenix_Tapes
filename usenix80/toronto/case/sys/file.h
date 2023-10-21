/*
 * One file structure is allocated
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct	file
{
	char	f_flag;		/* see below for flag bit assignments */
	char	f_count;	/* reference count */
	struct inode *f_inode;	/* pointer to inode structure */
	char	*f_offset[2];	/* read/write character pointer */
} file[NFILE];

/* flags */
#define	FREAD	01
#define	FWRITE	02
#define	FPIPE	04
#define FNET	010
#define FTIMEO	020
#define FOPEN	040
#define FERR	0100		/* there was an error in an net open */
