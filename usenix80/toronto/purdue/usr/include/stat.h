struct	statb {
	int	i_dev;
	int	i_ino;
	int	i_mode;
	char	i_nlink;
	char	i_uid1;
	char	i_uid0;
	char	i_size0;
	char	*i_size1;
	int	i_addr[8];
	long	i_atime;
	long	i_mtime;
};
