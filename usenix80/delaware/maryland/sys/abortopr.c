#define LPDIR   "/usr/lpd"
#define READ    0

struct {
	int     e_inode;
	char    e_name[14];
	} entry;
int filler 0;

struct {
	char    i_minor;
	char    i_major;
	int	i_number;	/* i number, 1-to-1 with device address */
	int	i_mode;
	char	i_nlink;	/* directory entries */
	char	i_uid;		/* owner */
	char	i_gid;		/* group of owner */
	char	i_size0;	/* most significant of size */
	char	*i_size1;	/* least sig */
	int	i_addr[8];	/* device addresses constituting file */
	int     i_actime[2];
	int     i_modtime[2];
	} inode;

main() {
register int uid, lp_fd;

if ((chdir(LPDIR) < 0) || ((lp_fd = open(LPDIR, READ)) < 0)) {
	write(2, "Error!\n", 7);
	exit(1);
	}

uid = getuid() & 0377;

while (read(lp_fd, &entry, 16) == 16)
	if (entry.e_inode && (stat(entry.e_name, &inode) >= 0))
		if (inode.i_uid == uid) {
			if ((inode.i_nlink & 0377) < 2)
				close(creat(entry.e_name));
			unlink(entry.e_name);
			}
}
