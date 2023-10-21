#include "hd.h"
#include "command.h"

/* This loads the command table from the .vshrc file */

cmdldrc () 
{
    FILE *rcstream,		/* Stream of .vshrc */
	*fopen();	/* Stdio open proc */

    char cbuf [STRMAX],	/* Buffer for input parm */
	*argv [ARGVMAX];	/* Pointers to input parm */
    int argc;	/* Number of parm */

    int line = 1;	/* Current line num of rcstream */

    /* initialize */

    strcpy (cbuf, HOME);
    strcat (cbuf, "/.vshrc");
    rcstream = fopen (cbuf, "r");
    if (rcstream == NULL) return;
    printf ("Loading .vshrc\r\n");

    while (!feof (rcstream)) 
    {
	if (readarg (rcstream, &line, &argc, argv, cbuf) != BAD)
	    cmdldarg (line-1, argc, argv);
    }
}

/* This loads command tables as specified in its arguments */
cmdldarg (line, argc, argv)
int line, argc; char *argv[]; 
{

    char *malloc();	/* Standard allocation proc */
    register i;	/* An index */

    register struct cmdstruct *cmdp;	/* Pointers */
    register struct classstruct *classp;
    register struct parmstruct *parmp;

    /* An addressable representation of CNULL (0) */
    extern char *cnull;

    if (argc <= 0) ;
    else if (argc == 1) lderror ("Too few args", line);

    else if (strlen (argv [0]) == 1) 
    {
	cmdp = cmdloc (*argv [0]);
	if (cmdp->cmd_char == 0) 
	{
	    lderror ("Not a proper command",
		line);
	    return;
	}
	classp = classloc (argv [1]);
	if (*classp->cl_name == 0) 
	{
	    lderror ("Illegal keyword", line);
	    return;
	}
	if (!(
	    argc-2 == classp->cl_count ||
	    (classp->cl_count == -1 && argc <= 3) ||
	    (classp->cl_count == -2 && argc >= 2)))
	{

	    lderror
		("Improper number of parameters", line);
	    return;
	}
	/* All testing over with--store new command */

	cmdp->cmd_proc = classp->cl_proc;
	cmdp->cmd_xdir = classp->cl_xdir;

	if (argc <= 2) cmdp->cmd_argv = &cnull;
	else cmdp->cmd_argv  =
	    (char **) malloc ((argc - 1) * (sizeof *argv));

	for (i=2; i<argc; i++) 
	{
	    cmdp->cmd_argv [i-2] =
		malloc (strlen (argv [i]) + 1);
	    strcpy (cmdp->cmd_argv[i-2], argv [i]);
	    cmdp->cmd_argv [i-1] = CNULL;
	}
    }
    else
    {
	for (parmp = parmtab;
	parmp->p_name &&
	    strcmp (parmp->p_name, argv[0]);
	parmp++);
	if (parmp->p_name) 
	{
	    parmp->p_val = malloc
		(strlen (argv[1]) + 1);
	    strcpy (parmp->p_val, argv[1]);
	}
	else lderror ("Bad parameter name", line);
	if (argc != 2) lderror
	    ("Too many args", line);
    }
}
