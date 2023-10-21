struct	stat	{
	int	s_dev;		/* device where inode resides */
	int	s_number;	/* i number, 1-to-1 with device address */
	int	s_mode;		/* mode for file (see below) */
	char	s_nlink;	/* number of directory entries */
	char	s_uid;		/* owner */
	char	s_gid;		/* group of owner */
	char	s_size0;	/* most significant of size */
	char	*s_size1;	/* least significant of size */
	int	s_addr[8];	/* device addresses constituting file */
	long	s_actime;	/* time of last access */
	long	s_modtime;	/* time of last modification */
};

struct {
	char	minor;		/* minor device */
	char	major;		/* major device */
};

/* modes */
#define	IALLOC	0100000		/* file is used */
#define	IFMT	060000		/* type of file */
#define		IFDIR	040000	/* directory */
#define		IFCHR	020000	/* character special */
#define		IFBLK	060000	/* block special, 0 is regular */
#define	ILARG	010000		/* large addressing algorithm */
#define	ISUID	04000		/* set user id on execution */
#define	ISGID	02000		/* set group id on execution */
#define ISVTX	01000		/* save swapped text even after use */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100
#define	INDIR	(ISUID|ISGID|ISVTX)	/* indirect file */
