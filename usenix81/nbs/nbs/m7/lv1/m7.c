	/*	M7.c
	**
	**	This routine is the main routine of a pattern matching filter
	**	which performs the matching and replacement of text from 
	**	standard input.  A temporary file, "M7_WKS.tmp", is created to 
	**	hold preprocessed versions of user macros which control the
	**	pattern matching and replacement process.
	**
	**	Parameters from program call:
	**		M7 options: -t trace; -p give prompt for input;
	**			    -n print only those lines that were matched.
	**			    -f reuse previously preprocessed macro file.
	**
	**		M7 input files contain the user defined macros.
	**
	**	Calls:  procmac, smatch, creat, inistck, inicntr, inivartbl,
	**              putline, reinitlist, fatal_error, getline, atoi
	**
	**	UE's: M7 can't open macro files.
	**
	**		Programmers: Tony Marriott & G. Skillman
	*/

# include 	"globvar.h"
/* These definitions define the maximum pattern size and input line size. */
/* Also defined is the number of macros that can have the "<" feature. */
# include 	"globdefin.h"

int	tswitch;   /* 3 of the 5 execution options are t (trace), n and p. */
int	nswitch;
int	pswitch;

main (argc,argv)
int	argc;
char	*argv[];

{

	extern int	nswitch,tswitch,pswitch;
	extern int	FALSE,TRUE,EOF,RW,RD,FROMBUF,STDIN,MACNUM;

	extern char	LT,MACTERM,EOS;
	int	fd1,fd2,i,j,totnummacs,index,fflag;
	int	scanlist [2][MAXSCANOFFS]; /* list of macros w/ the "<" feature. */
        int	availhead; /* Next available node in scanlist. */
	char	mac[MAXPAT],pmac[MAXPAT],line[MAXLINE],*WKS,maccount[2];


	fflag = FALSE;
	tswitch = FALSE;
	nswitch = FALSE;
	pswitch = FALSE;
	WKS="M7_WKS.tmp";
	inistck();  /* Create and initialize storage space for stacks. */
	inicntr();  /* Initialize counters and their increments. */
	inivartbl(); /* Initialize table of legal '%' commands. */

	availhead = 1; /* Initialize "availhead" to the 1st unused node. */ 
	scanlist[0][0] = -1;


	/* get switches */
        for (i=1;i < argc;i++)
	{
	  if (*argv[i] == '-')
	  {
	    switch(*(argv[i]+1))
	    {

	      case 't': /* print a trace of matching and replacement */
	        tswitch = TRUE;
	        break;

              case 'n': /* print only matched lines */
                nswitch = TRUE;
                break;

	      case 'p': /* suppress prompt for initial input */
		pswitch = TRUE;
		break;

	     case 'f': /* use an already existing preprocessed file */
		if (fflag == TRUE)
			fatal_error("M7: illegal placement of 'f' option");
		++i;

		/* Put the name of the 'f' file in "WKS": */
		index = 0;
		while ((WKS[index] = argv[i][index]) != EOS)
			++index;

		/* Open the 'f' file. */
		if ((fd2 = open(WKS,RW)) == -1)
			fatal_error("M7: cannot open 'f' option file");
		++i;
		/* Get # of macros in the f option file: */
		MACNUM = atoi(argv[i]) + MACNUM;
		seek(fd2,0,0);
		fflag = TRUE;
		break;

	      case 'a': /* get pattern matching and replacement  
		            info from argument list		*/
		++i;
		if (fflag == FALSE)
		{
			/* Create "M7_WKS.tmp". */
			fd2 = creat(WKS,RD);
			MACNUM = 0;
			fflag = TRUE;
		}
		MACNUM = MACNUM + 1;

                /* Preprocess the macro and put it on the preprocessed file */
		procmac(argv[i],pmac,MACTERM,MACNUM,scanlist,&availhead);
		putline(pmac,fd2,EOS);
		break;
	    }
	  }
	  else /* if not "-", (not an execution option). */
	  /* get macros file */
	  {
	    if (fflag == FALSE)
	    {
	    	/* Create "M7_WKS.tmp". */
	    	fd2 = creat(WKS,RD);
	    	MACNUM = 0;
	    	fflag = TRUE;
	    }
	    if ((fd1 = open(argv[i],RW)) == -1)
	    	fatal_error("M7: cannot open macro file");
	    while (getline(mac,fd1,MACTERM) != EOF) /*get macro defintions */
	    {
		  MACNUM = MACNUM + 1;

                /* Preprocess the macro and put it on the preprocessed file */
		procmac(mac,pmac,MACTERM,MACNUM,scanlist,&availhead);
		putline(pmac,fd2,EOS);
	    }
	    scanlist[0][availhead] = 0; /* Mark end of list. */
	    close(fd1);
	  }
	}

	if (fflag == FALSE)
		fd2 = creat(WKS,RD); /* Create "M7_WKS.tmp" if we haven't yet. */
	close(fd2);
	fd2 = open(WKS,RW);
	if (pswitch == TRUE)
		printf("ready \n"); /* prompt for intial input */

	totnummacs = MACNUM;
	while (getline(line,STDIN,LT) != EOF)
	{
	  /* perform matching and replacement */
	  smatch(line,fd2,scanlist,&totnummacs,&availhead);  
	  /* Mark all non-rescanable macros to show they haven't been used. */
	  reinitlist(scanlist, availhead); 
	  MACNUM = totnummacs;
	}
	close(fd2);
}
