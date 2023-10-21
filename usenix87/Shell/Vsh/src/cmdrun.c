#
#include "hd.h"
#include "command.h"

/* Each command called by command.c must return special information.
   First, the next command to execute is returned in the last eight
   bits (and can be masked out with CMDMASK).  If no more commands
   are to be run, these bits are set to 0.  The next bit is the
   REPLOT bit.  If on, the screen has been altered enough to require
   a replot.  The NOOP bit indicates a command was not found.
   Finally, the ENTERDIR bit indicates a new directory has been
   entered.
*/

command (cmd, ctype) register cmd; int ctype; 
{

    register ret; /* return value */
    int next; /* Temp variable for next command */
    register struct cmdstruct *cmdp;

    ret = 0;
    selecttype = ctype;
    while (cmd) 
    {
	if ((ctype == DIRCMD) &&
	    ((next = dircmd (cmd)) != NOOP)
	    )
	    cmd = next;
	else
	{
	    cmdp = cmdloc (cmd);
	    if (cmdp->cmd_proc && (
		(cmdp->cmd_xdir)||(ctype == DIRCMD))) 
	    {

		cmd = (*cmdp->cmd_proc) (cmdp->cmd_argv);
	    }
	    else cmd = NOOP;
	}
	ret |= cmd & (REPLOT | ENTERDIR | NOOP);
	cmd &= CMDMASK;
    }
    if (ret & NOOP) 
    {
	putmsg ("Unknown command.  Press ? for help.  ");
	if (ret & REPLOT) getrtn ();
    }
    return ret;
}

/* Classloc returns the classtab element corresponding the keyword referenced
   by cp */

struct classstruct *
    classloc (cp) register char *cp; 
{

    register struct classstruct *classp;

    for (classp = classtab;
    *classp->cl_name && strcmp (cp, classp->cl_name);
    classp++);
    return classp;
}

/* Cmdloc returns the cmdtab element corresponding to ch */

struct cmdstruct *
    cmdloc (ch) register char ch; 
{

    register struct cmdstruct *cmdp;

    for (cmdp = cmdtab;
    cmdp->cmd_char && cmdp->cmd_char != ch;
    cmdp++);

    return cmdp;
}

/* Cmdproc returns a pointer to the procedure which runs the command
   corresponding to ch.  */

int (*
    cmdproc (ch))() char ch; 
{
    extern struct cmdstruct *cmdloc();

    return ((*cmdloc)(ch)->cmd_proc);
}
