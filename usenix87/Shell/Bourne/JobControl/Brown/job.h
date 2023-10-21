#
/*
 *	SSH job-control structure (one for each job).
 *	Copyright (c) S. Brown, 1987
 */

#ifdef JOB

/* Because of the algorithm used for parsing, only the first and last
   processes of a pipeline are posted to the job-table, so set PPERJ=2.	*/


#define JOB_MAX	30	/* maximum number of jobs allowed */
#define PPERJ	2	/* processes known per job */
#define EXTRAPROCS JOB_MAX	/* size of "extra proc" table */

struct jobnod {
	union {
		struct {
			unsigned run : 1;	/* running */
			unsigned stop : 1;	/* stopped */
			unsigned back : 1;	/* background */
			unsigned notify : 1;	/* to be notified of termination */
			unsigned deaths : PPERJ;/* processes which have died */
			unsigned lifes : PPERJ;	/* processes associated with job */
			unsigned knowtty : 1;	/* have fetched tty status for this job */
			unsigned stopstate : 2;	/* stop states */
		} singlebits;	/* for referencing single bits */
		short totalbits;/* for making global assignments */
 	} jb_flags;		/* is this not perhaps a bit unpleasant altogether? */
	char *jb_dir;		/* current directory of job */
	int jb_pgrp;		/* process group (useful only for BSD) */
	char *jb_cmd;		/* name of command */
	int jb_pri;		/* priority (stack for %+ and %-) */
	struct {
		int proc_pid;		/* process id */
		int proc_status;	/* exit status */
		short proc_flags;	/* flags? not used, anyway */
	} jb_proc[PPERJ];	/* processes associated with job */
#ifdef JOBSXT
	int jb_sxtfg;		/* file descriptor for job's sxt */
#endif JOBSXT
};


#define SS_STOP	0
#define SS_TSTP	1
#define SS_TTIN	2
#define SS_TTOU	3


#ifdef PROCESS_DOT_C

struct extrawait {
	int pid;
	int status;
};

#endif PROCESS_DOT_C


#ifdef JOBSXT
#define SXTMAX (MAXPCHAN-1)
#define SXT(n) ((n)?(n)+1:sxtkludge)
extern int sxtchan;
extern int sxtkludge;
extern int sxt_fd;
extern TTYNOD realtty;
#endif JOBSXT

#define WAIT_FG		1
#define WAIT_BG		2
#define WAIT_NOPAUSE	4
#define WAIT_JOB	8

extern struct jobnod *jobs[];
extern int currentjob;

#endif JOB
