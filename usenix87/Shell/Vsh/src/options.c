#include "hd.h"
#include "command.h"

/* Print and modify command tables interactively */

options (parm) char **parm; 
{
    FILE *vshout;
    int line = 0;	/* Current line number */
    register struct cmdstruct *cmdp;
    register struct classstruct *classp;
    register char **argv;
    struct parmstruct *parmp;

    /* Print command tab in .vshrc format */
    /* If parm present, dump to file and quit */
    erase ();
    if (*parm) 
    {
	vshout = fopen(*parm, "w");
	if (vshout == NULL) {
		printf(" ");
		hilite("%s: Cannot create%s\r\n", *parm);
		return REPLOT;
	}
	line= -9999;
    }
    else {
	vshout = stdout;
	bufout();
    }
    for (;;) 
    {
	for (cmdp = cmdtab; cmdp->cmd_char >= 0; cmdp++) 
	{
	    if (cmdp->cmd_proc) 
	    {
		for (classp = classtab;
		*classp->cl_name &&
		    classp->cl_proc !=
		    cmdp->cmd_proc;
		classp++);

		fprintf (vshout, "%c\t%s", cmdp->cmd_char, 
		    classp->cl_name);

		for (argv = cmdp->cmd_argv; *argv;)
		    fprintf (vshout, "  %s", *argv++);

		if (optline (&line, vshout) == BAD)
		    return REPLOT;
	    }
	}
	for (parmp = parmtab; parmp->p_name; parmp++) 
	{
	    fprintf (vshout, "%s\t%s", parmp->p_name, parmp->p_val);
	    if (optline (&line, vshout) == BAD) return REPLOT;
	}

	if (*parm) {
		fclose(vshout);
		return REPLOT;
	}
	if (line != 0 && optcmd () == BAD) break;
	erase (); line = 0;
    }
    unbufout();
    return REPLOT;
}
/* Processing for end of each line includes:
	1.  Print the newline.
	2.  If the end of the page, prompt for a command.
*/
optline (line, fd)
int *line; 
FILE *fd;
{
    int ret;	/* return from optcmd */

    if (fd == stdout)
	putc (CR, fd);
    putc (LF, fd);
    if (++*line < (window-6)) return GOOD;

    ret = optcmd (); erase (); *line = 0;
    return ret;
}

optcmd () 
{
    char cbuf [STRMAX],	/* Buffer for input parm */
	*argv [ARGVMAX];	/* Pointers to input parm */
    int argc;	/* Number of parm */
    int ret;	/* Return from readarg */

    int line = 0;	/* Current line num of rcstream */

    printf ("\r\n");
    hilite ("Type in a new parameter, or\r\n\
Press ^D to leave.  Press -Return- to display more parameters.\r\n\n");
    unbufout();
    tty_push (COOKEDMODE);
    for (;;) 
    {
	ret = readarg (stdin, &line, &argc, argv, cbuf);
	if (argc == 0) break;
	if (compe ("quit", argv [0])) 
	{
	    ret = BAD; break;
	}
	if (ret != BAD) cmdldarg (line, argc, argv);
	/* Reset screen window */
	setwindow();
	/* Reset columns */
	setcolumn();
    }
    tty_pop ();
    erase();
    if (ret != BAD)
	bufout();
    return ret;
}
