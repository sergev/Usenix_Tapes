struct	tim
{
int	sec;
int	min;
int	hour;
int	day;
int	month;
int	year;
int	dweek;
int	dyear;
int	dst;
};

struct	record
{
int	flags;		/*  flags defined below	*/
int	procids;		/*	current process id	*/
int	pcount;		/* total processes	*/
int	ucount;		/*	total users	*/
int	scount;		/*	total size of swappable  blocks being managed	*/
int	ccount;		/*	I/O character count	*/
int	bcore;		/*	swappable blocks in core	*/
int	pcore;		/*	processes in core	*/
int	score;		/*	runable blocks that are swapped out	*/
int	runproc;		/*	processes ready to run	*/
int	cpuproc;		/*	processes geting time since being in core	*/
int	swproc;		/*	runable processes that are swapped out	*/
int	corefr;		/*	blocks of free core from coremap	*/
int	maxcore;		/*	blocks of free core from unix start_up	*/
int	coreu;		/*	coremap array entries used	*/
int	swapfr;		/*	blocks of free swap space	*/
int	maxswap;		/*	maximum swap space as defined in c.c	*/
int	swapu;		/*	swapmap array entries used	*/
int	fileu;		/*	file table array entries used	*/
int	inou;		/*	inode table array entries used		*/
int	textu;		/*	text table array entries used	*/
int	texts;		/*	text table entries for sticky bit commands	*/
int	callu;		/*	callout table array entries used	*/
int	cytime;		/*	cycle time interval	*/
int	time[2];	/*	system time information	*/
int	filler[6];
}	rbuf;

/*	FLAGS	*/

#define	REBOOT	01
#define	SYSUP	02
#define	SYSDN	04
#define	DATA	0100000		/*	summary data available		*/
#define	PIDATA	0100		/*	process "procids" data available	*/

