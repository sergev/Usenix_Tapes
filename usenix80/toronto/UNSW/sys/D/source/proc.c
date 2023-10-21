/* initialised data for all other routines..... */

#include "proc.h"

char	*stat[] {
	"unassigned\n",
	"sleeping on high priority\n",
	"sleeping on low priority\n",
	"ready to run\n",
	"being created\n",
	"being terminated (zombie)\n",
	"being traced\n",
	};

char	*flag[NOFLAG] {
	"in core; ",
	"the scheduling process; ",
	"locked in core; ",
	"being swapped out; ",
	"being traced; ",
	"has the \"swted\" tracing flag set; ",
	"in a funny state with bit 7 set; ",
	"in a funny state with bit 8 set; ",
	};

char	*signals[NSIG+1] {
	"null ",
	"hangup ",
	"interrupt (rubout) ",
	"quit (FS or cntrl/\\) ",
	"illegal instruction ",
	"trace or breakpoint ",
	"IOT instruction ",
	"EMT instruction ",
	"floating exception ",
	"kill (wont kill if hung on IO) ",
	"bus error ",
	"segmentation violation ",
	"bad arg to system call ",
	"broken pipe (write; no read) ",
#ifndef	UNSW
	"signal 14 ",
	"signal 15 ",
	"signal 16 ",
#endif
#ifdef	UNSW
	"terminate ",
	"real-time limit ",
	"cpu-time limit ",
#endif
	"signal 17 ",
	"signal 18 ",
	"signal 19 ",
	"signal 20 ",
	};

struct symtab symbols[]
	{
	"_buf\0\0\0\0",	sizeof buf[0],
	"_proc\0\0\0",	sizeof proc[0],
	"_inode\0\0",	sizeof inode[0],
	"_text\0\0\0",	sizeof text[0],
	"_file\0\0\0",	sizeof file[0],
	"_kl11\0\0\0",	sizeof tty,
#ifdef	DJ11
	"_dj11\0\0\0",	sizeof tty,
#endif
#ifdef	DZ11
	"_dz11\0\0\0",	sizeof tty,
#endif
	0,		0
	};

char	*tflag[] {
	"locked busy; ",
	"wanted for locking; ",
	};

struct symbol nullnum {
	"??(0)??\0",
	0,
#ifdef	BIG_UNIX
	0,
#endif
	0,
	};

int	*regbuf	&wkbuf[0];	/* equivalenced to wkbuf */

char	partab[1];
/*
 * to do:
 *
 *	tty queues
 *	call nodes
 *	paged system
 *	buf queues
 *	raw buffers
 *	i-nodes
 *	superblock
 */
