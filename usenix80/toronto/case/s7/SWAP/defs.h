#

/*
 *	Globally defined symbols for swapping display
 */

# include "/usr/sys/param.h"
# include "/usr/sys/text.h"
# include "/usr/sys/proc.h"

int	swapmap[SMAPSIZ],
	swplo,
	nswap;

struct namelist{
	char	n_name[8];
	int	n_type;
	int	n_value;
};

struct namelist nl[5];

struct map{
	unsigned	m_size;
	unsigned	m_addr;
};

struct terminal{
	char	*t_name;
	int	t_cnt;
};

int	type,	/* terminal type */
	mfd;	/* kernel memory file descriptor */

struct screen{
	char	s_size[180];	/* swap area sizes */
	int	s_pid[180];	/* pid associated with screen slot */
}screen;

char users[100][6];		/* user names indexed by uid */

# define READ	0
# define WRITE	1
# define RW	2

# define TRUE	1
# define FALSE	0

# define DATA	1
# define TEXT	2
# define SVTXT	3

# define UNIX		"/unix"
# define KMEM		"/dev/kmem"
# define TTYS		"/etc/ttys"
# define SYSMESG	"/etc/sysmesg"

# define DTITLE		1,1
# define TTITLE		65,1
# define MTITLE		31,1
# define STITLE		1,3
# define TDATA		71,1
# define DDATA		7,1
# define SUDATA		28,3
# define SADATA		15,3
# define W1TITLE	69,5
# define A1TITLE	1,9
# define S1TITLE	69,8
# define W2TITLE	69,12
# define A2TITLE	1,16
# define S2TITLE	69,15
# define W3TITLE	69,19
# define A3TITLE	1,23
# define S3TITLE	69,22

# define TTYSIZE	150
# define TEK4023	1
# define BEEHIVE	2
# define ADM3		3
# define ADDS		4

# define MNAME		5
# define GRANULARITY	5

# define convert(x)	(x)/GRANULARITY
