#if BSDREL >= 42
#include <sys/dir.h>
#else

#if defined(pdp11) || USGREL == 30
#define DIRBLKSIZ 512
#else
#define DIRBLKSIZ 1024
#endif

struct	direct {
	long	d_ino;			/* inode number of entry */
	short	d_namlen;		/* length of string in d_name */
	char	d_name[16];		/* name must be no longer than this */
};

/*
 * Definitions for library routines operating on directories.
 */
typedef struct {
	int	dd_fd;
	int	dd_nleft;
	char	*dd_nextc;
	char	dd_buf[DIRBLKSIZ];
} DIR;

#ifndef NULL
#define NULL 0
#endif
extern	DIR *opendir();
extern	struct direct *readdir();
extern	void closedir();
#endif
