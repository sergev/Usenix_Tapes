/*  docomand.c
*
*	This routine processes the '%' feature when it is found at the 
*	beginning of an input line. The legal commands are '%TRACE', 
*	'%NFLAG' and '%MACRO' to alter the t and n options or add a macro
*	respectively.
*
*	The routine 'streq' is used to determine if the five letters of the
*	command match the list of commands in the table 'VARTABLE', then,
*	depending on which entry of the table it matched, the corresponding
*	command is executed.
*
*	Arguments:  line - the current input line;  
*		    fd2  - file pointer to the preprocessed macro file;
*		    totmacs - the total # macros preprocessed so far;
*		    scanlist - list of non rescanable macros;
*		    availhead - the next available spot in scanlist;
*		    index - the current node in scanlist being observed.
*
*	Calls: streq, procmac, putline, seek
*
*	UE's: Errors occur if the macro being added in a '%MACRO' command
*	      is in error, or if the eighth column (7th element) of either
*	      the '%TRACE' or '%NFLAG' command is not a zero or one.
*	      (Execution does not terminate on the later type of error.)
*
*			Programmer: G. Skillman
*/
# include 	"globdefin.h"


docomand(line, fd2, totmacs, scanlist, availhead, index)
char	*line;
int	scanlist [3][MAXSCANOFFS];
int	fd2, *totmacs, *availhead, *index;

{
	extern char	*VARTABLE[], MACTERM, EOS;
	extern int	 EOT, NO, YES, tswitch, nswitch, MACNUM;
	int	tblentry, answer, temp;
	char	pmac[MAXPAT];

	answer = NO;
	tblentry = 0;

	while (tblentry <= EOT && answer == NO)
	{
		answer = streq(VARTABLE[tblentry], &(line[1]));
		++tblentry;
	}

	if (answer == NO)
		return(NO);

	switch(tblentry)
	{
		case 1:
			seek(fd2, 0, 2);
			*totmacs = *totmacs + 1;
			temp = *totmacs;
			MACNUM = MACNUM + 1;
			procmac(&(line[7]),pmac,MACTERM,temp,scanlist,availhead);
			putline(pmac, fd2, EOS);
			*index = *availhead - 1;
			seek(fd2, -3, 2);
			return(YES);

		case 2:
			if (line[7] == '0')
				tswitch = 0;
			else if (line[7] == '1')
				tswitch = 1;
			else
				printf("DOCOMAND: Illegal code ignored.\n");
			break;

		case 3:
			if (line[7] == '0')
				nswitch = 0;
			else if (line[7] == '1')
				nswitch = 1;
			else 
				printf("DOCOMAND: Illegal code ignored.\n");
			break;
	}
	return(YES);
 }
