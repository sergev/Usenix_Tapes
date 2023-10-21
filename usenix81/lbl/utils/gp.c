/***********************************************************/
/*                                                         */
/*                          GP                             */
/*                                                         */
/* GP (GetPages) is used for extracting pages from a file  */
/* which has been SROFFed.  It is intended for use by      */
/* people who don't want to use SROFF to pick out the      */
/* specified pages, since that crunches CPU time.          */
/*                                                         */
/* Form: gp [-snn] [-emm]                                  */
/*      input is always from STDIN, output to STDOUT.  If  */
/*      -snn is omitted, the default is page 1.  If -emm   */
/*  is omitted, the default is the last page.  Any         */
/*      other parameters will cause error messages.        */
/*                                                         */
/* Program name:        GP                                 */
/* Source language:     Version 7 C                        */
/* Programmer:          Jeremy Epstein                     */
/* Installation:        University of New Mexico           */
/* Operating System:    PWB/UNIX                           */
/* Date:                June 27, 1980                      */
/*                                                         */
/* Revised by Suzanne O'Dell June 5, 1981                  */
/*   New version of SROFF does not initalize fonts on      */
/*   every page so GP must get the initalizing string      */
/*   from the beginning of the file.                       */
/*                                                         */
/***********************************************************/


/* Standard I/O Library */

#include <stdio.h>

/* Global variables */

#define FF 014    	/* Form feed character */
#define LINELEN 2000   /* Number of characters on line */

int    curpage;	/* Current page being processed */
char    buffer[LINELEN];/* Input/output buffer */


/****************/
/* Main routine */
/****************/

main(argc,argv)
    int argc;
    char **argv;

{

int    argnum,		/* Argument being processed */
       startpage,   /* Page to start on */
       endpage;     /* Page to end on */


/* First look for arguments */

startpage = 1;
endpage = 16000;    /* Go virtually forever */
for (argnum = 1; argnum < argc; argnum++)
    {
    if (argv[argnum] [0] == '-')
        {
        if (argv[argnum] [1] == 's')
            /* Start page number */
            startpage = atoi(&argv[argnum] [2]);
    	else
            if (argv[argnum] [1] == 'e')
                /* End page number */
                endpage = atoi(&argv[argnum] [2]);
    		else
                {
                fprintf(stderr, "Invalid option\\nForm: gp [-snn] [-emm]\n");
                exit(0);
                }
    	}
    else
    	{
    	fprintf(stderr, "Invalid argument\nForm: gp [-snn] [-emm]\n");
    	exit(0);
    	}
    }


/* Now check for validity */

if (startpage > endpage)
    {
    fprintf(stderr, "Start page greater than end page\n");
    exit(0);
    }

if (startpage < 0)
    {
    fprintf(stderr, "Start page less than zero\n");
    exit(0);
    }

if (endpage < 0)
    {
    fprintf(stderr, "End page less than zero\n");
    exit(0);
    }


/* Option processing now complete */

/* First get to correct page */

curpage = 0;

if(startpage != 0) {
	getline(buffer);	/* first line includes initalization  */
	fputs(buffer, stdout);		/* SPO   */
	}

while (curpage < startpage)
    getline(buffer);

/* Now copy pages desired */

while (curpage <= endpage)
    {
    fputs(buffer, stdout);
    getline(buffer);
    }

/* Dump last line */

fputs(buffer, stdout);


/* And that's all folks */

exit(0);

}




/**********************/
/* Get a line routine */
/**********************/

getline(buffer) char buffer[]; {

int index;

/* Quit at end of file */

if (fgets(buffer, LINELEN, stdin) == NULL)
    exit(0);

/* Look for a formfeed */

index = 0;
while (buffer[index])
    {
    if (buffer[index++] == FF)
    	curpage++;
    }

}
