/*
 * definitions for archive files
 */
# define ARMAG	0177545

struct archive{
	char	a_name[14];
	long	a_date;
	char	a_uid;
	char	a_gid;
	int	a_mode;
	long	a_size;
};
