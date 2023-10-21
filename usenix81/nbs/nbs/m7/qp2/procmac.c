/*  procmac.c
*
*	This program preprocesses the input macros in mac and writes them
*	to pmac. "Makpat" will preprocess the pattern, "maksub" the replacement 
*	definition and "proccalls" will preprocess the stack and counter calls
*	and "procmac" the replacement symbol found between the pattern and
*	replacement definition.
*
*	Parameters: mac - the original macro buffer;
*	   pmac - the preprocessed macro buffer;
*	   macterm - the semicolon; macnum - new macro's #; 
*	   scanlist - list of macros with "<" feature; availhead - 1st unused
*	   array element in scanlist
*
*	Calls: makpat, fatal_error, maksub, addset, addnode,
*	       size, itoc, procstck, proccntr, proccalls
*	UE'S: None.
*
*			Programmmers: Tony Marriott & G. Skillman
*/
/* The following defines the maximum size of the pattern and the maximum 
   # of macros with the "<" feature. */
# include	"globdefin.h"
procmac(mac,pmac,macterm,macnum,scanlist,availhead)
char	*mac,*pmac,macterm;
int	scanlist [2][MAXSCANOFFS]; /* linked list of macros to be used once: */
			/* 1st dimension has the macro #, the 2nd has the */
			/* flag of whether the macro has been used yet. */
int	macnum,*availhead;
{
	int	i, j, index, prevnode, code;
  	char	SPACE;
	extern char	MACDELIM,REPLACE,SCANOFF,EOS,STACK,CNTR;
	extern int	RESCANMAC;

	i = 0;
	SPACE = ' ';
	j = 0;

	/* Preprocess stack and counter calls found before the pattern. */
	code = 1;
	proccalls(mac,&i,pmac,&j,code);

	/* preprocess the pattern part of mac and write it to pmac. */
	makpat(mac,&i,pmac,&j);
	
	/* While not at '=' or '<' in the input macro: */
	++i;


	/* Preprocess stack and cntr calls found before '=' or '<'. */
	code = 2;
	proccalls(mac,&i,pmac,&j,code);
	pmac[j] = mac[i];

	/* If '<' present, add the macro's # to the list 'scanlist': */
	if (mac[i] == SCANOFF)
		addnode(scanlist,macnum,RESCANMAC,availhead);
	++j;
	++i;

	/* While the 3rd quote beginning the replacement definition hasn't
	   been found:                                      */
	code = 1;
	proccalls(mac,&i,pmac,&j,code);

	/* Preprocess the replacement part of the macro. */
	maksub(mac,&i,pmac,&j);

	/* add 'EOS' to pmac. */ 

	addset(EOS,pmac,&j,MAXMAC); /* Store end of string marker. */
}
