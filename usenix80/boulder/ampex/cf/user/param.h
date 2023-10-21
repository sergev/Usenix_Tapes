#define	NOFILE	15		/* max open files per process */
#define	NSIG	20
#define NULL 0
#define	DIRSIZ	14		/* max characters per directory */
struct {		/* access a long as words */
	int hiword;
	int loword;
} ;
#define R0	02

struct inode {
	char	i_flag;
	char	i_count;	/* reference count */
	int	i_dev;		/* device where inode resides */
	int	i_number;	/* i number, 1-to-1 with device address */
	int	i_mode;
	char	i_nlink;	/* directory entries */
	char	i_uid;		/* owner */
	char	i_gid;		/* group of owner */
	char	i_size0;	/* most significant of size */
	char	*i_size1;	/* least sig */
	int	i_addr[8];	/* device addresses constituting file */
	int	i_lastr;	/* last logical block read (for read-ahead) */
} ;

/* modes */
#define	IFMT	060000		/* type of file */
#define		IFDIR	040000	/* directory */
#define		IFCHR	020000	/* character special */
#define		IFBLK	060000	/* block special, 0 is regular */

struct	file {
	char	f_flag;
	char	f_count;	/* reference count */
	int	f_inode;	/* pointer to inode structure */
	char	*f_offset[2];	/* read/write character pointer */
} ;
