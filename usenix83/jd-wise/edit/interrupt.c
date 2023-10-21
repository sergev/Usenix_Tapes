#include "ed.h"
#include "process.h"
#include "shell.h"
#include "spell.h"

/** the resetting of signals needs to be cleaned up */

onalarm()	/* update text and sync files */
	{

	if(shellpid)
		flushpipe();
	if(spellpid)
		getbadword();
	synctemp();
	signal(SIGALRM,onalarm);
	alarm(30);
#ifdef debugging
debug(15,"alarm returned");
#endif
	}

onfpe()	/* handle signal from shell */
	{
	char c;
	int j;

/* debug
fprintf(stderr,"signal recieved\n");
 */
	shellactive=0;
	signal(SIGSHELL,onfpe);
	j=alarm(0);
	flushpipe();
	if(sbflag)
		write(1,"\007",1);
	alarm(j?j:30);	/** this may need adjustment */
	}

onemt()	/* handle signal from spell */
	{
	int j;


#ifdef debugging
debug(73,"\007spell signal recieved");
#endif
	signal(SIGEMT,onemt);
	j=alarm(0);
	getbadword();
	alarm(j?j:30);	/** this may need adjustment */
	}

onintr()
	{
	signal(SIGINT, onintr);
	if(shellpid)
		kill(shellpid,SIGINT);
	lastc = '\n';
	errfunc("Interrupt");
	}

#ifdef noalarm
/* eunice alarm doesn't work */
alarm(){}
#endif
