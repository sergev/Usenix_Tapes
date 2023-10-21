	/*	qp.c
	**
	**	This routine is the main routine of a query processor
	**	derived from a pattern matching filter called M7 
	**	which performed the matching and replacement of text from 
	**	standard input. This program processes a syntax and semantics
	**	macro file and stores a preprocessed version of each in 
	**	"QP_SYN" and "QP_SEM" respectively. "Procmac" preprocesses
	**	the macros from both while "smatch" handles the matching and
	**	replacement process.
	**
	**
	**	Parameters from program call:
	**		M7 options: -t trace; -p give prompt for input;
	**			    -f reuse previously preprocessed macro file.
	**
	**		M7 input files contain the user defined macros.
	**		The file descriptors are:
	**		fd1 - input syntax file; fd2 - preprocessed syntax file
	**		fd3 - input semantic file; fd4 - preprocessed semantic
	**		fd5 - output syntax file;  fd6 - output semantic file
	**
	**	Calls:  procmac, smatch, creat, inistck, inicntr,
	**              putline, reinitlist, fatal_error, getline
	**
	**	UE's: f option used after non-preprocessed macro files given.
	**	      f option files don't exist.
	**	      Non-preprocessed files given after occurrence of f option
	**	      Syntax file doesn't exist.
	**	      Semantics file doesn't exist.
	**	      At least one semantics macro has the '<' option.
	**
	**		Programmers: Tony Marriott & G. Skillman
	*/

# include	"globvar.h"
/* These definitions define the maximum pattern size and input line size. */
/* Also defined is the number of macros that can have the "<" feature. */
# include	"globdefin.h"


int	tswitch;   /* 2 of the 3 exection options are t (trace) and p. */

main (argc,argv)
int	argc;
char	*argv[];

{

	extern int	tswitch;
	extern int	FALSE,TRUE,EOF,RW,RD,FROMBUF,STDIN;
	extern int	SYNMACNUM, SEMMACNUM;

	extern char	COMMENT,LT,MACTERM,EOS;
	int	fd1,fd2,fd3,fd4,fd5,fd6,i,j,index;
	int	synlist [2][MAXSCANOFFS]; /* list of macros w/ the "<" feature. */
	int	semlist [2][MAXSCANOFFS]; /* list of illegal macros  "<" */
        int	synavail; /* Head of the list of available nodes in synlist. */
	int	semavail; 
	int	filesopen, pswitch;
	char	mac[MAXPAT],pmac[MAXPAT],line[MAXLINE],*SYN,*SEM,*SEMOUT,*SYNOUT;


	filesopen = FALSE;
	tswitch = FALSE;
	SYNMACNUM = 0;
	SEMMACNUM = 0;
	pswitch = FALSE;
	SYN = "QP_SYN";
	SEM = "QP_SEM";
	SYNOUT = "QP_SYNOUT";
	SEMOUT = "QP_SEMOUT";
	inistck();  /* Create and initialize storage space for stacks. */
	inicntr();  /* Initialize counters and their increments. */

	synavail = 0; /* Initialize "synavail" to the 1st unused node. */ 
	semavail = 0; /* Init. here also to detect if '<' occured. */

	fd5 = creat(SYNOUT,RD);

	/* get switches */
        for (i=1;i < argc;i++)
	{
	  if (*argv[i] == '-')
	  {
	    switch(*(argv[i]+1))
	    {
              case 'f': /* get already preprocessed files. */
		if (filesopen == TRUE)
			fatal_error("QP: illegal placement of f option");

		/* We must: get the names of the files, then the # of macros
		   that are in each and store them in SYNMACNUM & SEMMACNUM  */
		++i;
		index = 0;
		while ((SYN[index] = argv[i][index]) != EOS)
			++index;

		++i;
		index = 0;
		while ((SEM[index] = argv[i][index]) != EOS)
			++index;

		if ((fd2 = open(SYN,RW)) == -1 || (fd4 = open(SEM,RW)) == -1)
			fatal_error("QP: cannot open 'f' option files");


		filesopen = TRUE;
		break;

	      case 't': /* print a trace of matching and replacement */
	        tswitch = TRUE;
		fd6 = creat(SEMOUT,RD);
	        break;

	      case 'p': /* provide prompt for initial input */
		pswitch = TRUE;
		break;
	    }
	  }
	  else
	  /* get macros file */
	  {
	    if (filesopen == TRUE)
		fatal_error("QP: more macro files can't be used with f option");
	    else
	    {
		fd2  = creat(SYN,RD); /* open temporary file for processed macros. */
		fd4  = creat(SEM,RD); /* open temporary file for semantics macros. */
	    }
	    if ((fd1 = open(argv[i],RW)) == -1)
	    	fatal_error("QP: cannot open macro file");
	    while (getline(mac,fd1,MACTERM) != EOF) /*get macro defintions */
	    {
		  SYNMACNUM = SYNMACNUM + 1;

		/* Preprocess macros and put on preprocessed file. */
		  procmac(mac,pmac,MACTERM,SYNMACNUM,synlist,&synavail);
	  	  putline(pmac,fd2,EOS);          /* put coded macro in file */
	    }
	    synlist[0][synavail] = 0; /* Mark end of list. */
	    close(fd1);
	    ++i;
	    if ((fd3 = open(argv[i], RW)) == -1)
		fatal_error("QP: cannot open semantics file");
	    while (getline(mac,fd3,MACTERM) != EOF)
	    {
		SEMMACNUM = SEMMACNUM + 1;
		procmac(mac,pmac,MACTERM,SEMMACNUM,semlist,&semavail);
		putline(pmac,fd4,EOS);
	    }
	    /* The '<' option is illegal in semantic macros! */
	    if (semavail > 0)
		fatal_error("QP: Error - all semantic macros must use '=' not '<'");

	    close(fd3);
	  }
	}

	close(fd2);
	close(fd4);
	fd2 = open(SYN,RW);
	fd4 = open(SEM,RW);
	if (pswitch == TRUE)
		printf("ready \n"); /* prompt for initial input */

	SYNMACNUM = 0;   /* Reset SYNMACNUM for later use in smatch */
	SEMMACNUM = 0;
	while (getline(line,STDIN,LT) != EOF)
	{
		/* perform matching and replacement */
	  	/* match line with macros */
	  	smatch(line,fd2,fd4,fd5,fd6,synlist);
	  	reinitlist(synlist, synavail); /* Mark all macros to show that they 
				have not been used for the new line of input. */
	}
	close(fd2);
	close(fd4);
	close(fd5);
}
