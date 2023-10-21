/*  semsmatch.c
*
*	This routine processes semantics macros. Notice that a match of the
*	the macros is only checked for on the first byte of semline. This
*	is because it is assumed that semline, which is initialized to "S"
*	here, will be replaced by the contents of a stack. The stack will
*	have some identifying symbols which tell exactly what syntax macro
*	has just been matched. The remainder of the semantics macros will
*	check for the identifying symbols of their partner syntax macros.
*	If a semantics macros sees those symbols on semline, then it knows
*	that it must carry out the latter part of the syntax-semantics macro
*	matching process.
*
*	Calls: amatch, rewrite, seek, getprocpat, docomand, docall
*
*	Parameters:
*		semline - where the semantic output will be placed.
*		semfd - file descriptor for the preprocessed semantics file.
*
*	UE's: An error occurrs if 2 semantic macros are not matched for every
*	      syntax macro.
*
*					Programmer: G. Skillman
*/

# include	"globdefin.h"

semsmatch(semline,semfd)

char	*semline;
int	semfd;
{
	extern int	YES,TRUE,FALSE,SEMMACNUM;
	extern char	EOS,EOF,MACDELIM,MACTERM;
	int	semend, sempatpos, index, count, semtagary[100][2];
	char	sempat[MAXPAT];

	SEMMACNUM = 0;
	index = 0;
	semline[0] = 'S';
	semline[1] = EOS;
	seek(semfd,0,0);
	count = 0;

	/* While not at the end of the file, process the macro that gets the
	   identifying symbols of the syntax macro just matched, and the 
	   corresponding semantics macro: */
	while (getline(sempat,semfd,EOS) != EOF && count < 2)
	{
		SEMMACNUM = SEMMACNUM + 1;
		sempatpos = 0;

		/* Process stack and counter calls found before the pattern. */
		docalls(sempat,&sempatpos,semline,semtagary);
		if ((semend = amatch(semline,0,sempat,&sempatpos,
					MACDELIM,semtagary)) != FALSE)
		{
			count = count + 1;
			/* process calls found before the replacement def. */
			docalls(sempat,&sempatpos,semline,semtagary);
			rewrite(semline,0,&semend,sempat,sempatpos,semtagary);
		}
	}
	if (count != 2)
		printf("SEMSMATCH: warning %d semantic macros matched\n",count);
}
