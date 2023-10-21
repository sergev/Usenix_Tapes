/*	Table of expermentally derived processor speeds.
 *	Speed is expressed in BPSXX, where XX is the processor type,
 *	and BPS stands for blocks per second.
 *	Speeds are dependent mostly on the processor type
 *	used, the features of the UNIX kernel being run
 *	on the system, and perhaps existence of processor
 *	options such as cache.
 *	Speed is mostly independent of the disk drivers being
 *	used (unless very complex), system buffer availability
 *	(param.h), mounted filesystems, and core memory.
 *	Also, for purposes of its computation, system speed
 *	is independent of system load from other users, and I/O.
 *
 *	BPSXX - obtained by executing "time cp file /dev/null" 
 *	against a large file (8000 blocks or more) in an otherwize empty
 *	file system.  The system time (sys) resulting is divided into the number
 *	of blocks in the file to get the number of blocks/second (BPSXX)
 *	for the processor used.
 *	Resulting values should fall within 20% of each other.
 *	Use the lowest value for BPSXX.
 *	BPSXX represents the speed at which UNIX can
 *	handle the read/write, read/write ahead, interupt, and return
 *	code associated with block I/O.
 */

#define	BPS34	100
#define	BPS35	100
#define BPS40	100
#define	BPS44	170
#define	BPS45	170
#define	BPS60	170
#define	BPS70	200
