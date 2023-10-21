/*
 * structure of a directory
 */
# define DIRSIZ	14

struct dir{
	int	d_ino;
	char	d_name[DIRSIZ];
};
