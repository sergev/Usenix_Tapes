struct systemid {
	char	si_type[2];		/* system type (wp) */
	int	si_num;			/* system number (1) */
	char	si_title[78];		/* system title (AF/DA Text Processing System) */
	char	si_class[2];		/* system classification (ts) */
	int	si_osver;		/* operating system version (6) */
	int	si_osed;		/* operating system edition (3) */
	int	si_oschg;		/* operating system change (4) */
	int	si_cpu;			/* processor type (70) */
	int	si_fp;			/* floating point processor (1=present) */
	int	si_ndisk;		/* number of disks (4) */
	int	si_ntape;		/* number of tapes (1) */
	int	si_ntty;		/* number of tty ports (48) */
};
