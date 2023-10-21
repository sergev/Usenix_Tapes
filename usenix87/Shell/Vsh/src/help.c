#include "hd.h"

/* Help displays a file in paged mode.  The parm is the file name. */

help (parm) char *parm; 
{
    FILE *helpfile;
    int flag;

    flag = 0;
    helpfile = fopen (parm, "r");
    if (helpfile == NULL) 
    {
	myperror (parm);
	return NOREPLOT;
    }
    else
	flag = page (helpfile, parm);
    fclose (helpfile);
    return flag ? REPLOT : NOREPLOT;
}

/* Display is the command processor's version of help */
display (argv) char **argv; 
{
    register char *msg;
    char name [STRMAX];

    msg = 0;
    if (*argv == CNULL) {
	if (VSHMODE == SELECTMODE && selecttype == DIRCMD && *selectname)
		return help(selectname);
	msg = "Display: ";
    }
    if (getfname (*argv, name, msg) == BAD) return NOREPLOT;
    else return help (name);
}
