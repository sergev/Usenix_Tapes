#
#define LOGCOST 220	/* $2.20 per connect hour - 1980 */
#define CPUCOST 5600	/* $56.00 per cpu hour - 1980 */
/* Used for logout cost summary in shell ("cost" program) */

 
 
/*
 *	login buffer layout	(16 per block)
 */

struct logactbuf {
	char	logname[9];
	char	logtty;
	int	logtime[2];
	char	loguid;
	char	loggid;
	char	logproj[6];
	char	logopr[5];
	char	logfill[5];
};



/*
 *	Shell Accounting buffer layout	(32 per block)
 *
 *	Note:  reads and writes from this structure should use
 *		a size = (sizeof this struct -1).  The filler
		character should be set to '\0' for string
		handling purposes.
 */

struct shactbuf {
	char	shuid;
	char	shgid;
	int	shtime[2];
	char	shproj[6];
	char	shopr[4];
	char	shfill[1];
};


/*
 *	Internal Summary Accounting layout
 */
struct sumactbuf {
	char	sumuid;
	char	sumgid;
	char	sumlogin;
	char	sumname[9];
	char	sumproj[6];
	float	sumcputime;
	float	sumlogtime;
	char	sumopr[5];
	char	sumcc[5];
};


/*
 *	Transmission Record to System C
 *
 *	Costs are computed on System C
struct {
	char	yy[2];
	char	mm[2];
	char	dd[2];
	char	loginame[8];
	char	projname[5];
	char	oprname[4];
	char	numlogin[4];
	char	logtime[8];
	char	cputime[8];
	char	cc[4];
};
 */
