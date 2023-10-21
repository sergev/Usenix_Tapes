/*
 * Inode structure as it appears on
 * the disk. Not used by the system,
 * but by things like check, df, dump.
 */
struct	inode
{
	int	i_mode;
	char	i_nlink;
	char	i_uid;
	char	i_gid;
	char	i_size0;
	char	*i_size1;
	int	i_addr[8];
	int	i_atime[2];
	int	i_mtime[2];
};

/* modes */
#define	IALLOC	0100000
#define	IFMT	060000
#define		IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
#define	ILARG	010000
#define	ISUID	04000
#define	ISGID	02000
#define ISVTX	01000
#define	IREAD	0400
#define	IWRITE	0200
#define	IEXEC	0100
#define	INDIR		(ISVTX|ISGID|ISUID)

/* contiguous info offsets into i_addr */
#define CN_FLAG		0	/* has 1 means this is contiguous file	*/
#define CN_LEN		1	/* length of contiguous region	 	*/
#define CN_BEG		2	/* block # of first contiguous block 	*/
#define CN_TOP		3	/* first i_addr slot not used by contig	*/
				/* all slots below here are reserved	*/
