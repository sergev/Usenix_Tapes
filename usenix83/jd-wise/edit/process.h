#ifndef FILE
#include <stdio.h>
#endif

#ifndef PROCESS
struct process{
	int	pr_siifd;	/* pipe from edit for stdin */
	int	pr_siofd;	/* pipe to bkgnd for stdin */
	int	pr_soifd;	/* pipe from bkgnd for stdout */
	int	pr_soofd;	/* pipe to edit for stdout */
	FILE*	pr_fmbkg;	/* file from bkgnd */
	FILE*	pr_tobkg;
	int	pr_pid;		/* pid of background process */
	};

int	editpid;		/* pid of edit process */
#define PROCESS
#endif
