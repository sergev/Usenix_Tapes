/*
 * Name:	MicroEMACS
 *		Spawn CLI for System V.
 * Version:	0
 * Last edit:	17-Apr-86
 * By:		gonzo!daveb
 *		{sun, amdahl, mtxinu}!rtech!daveb
 *
 * Spawn for System V.
 */
#include	"def.h"

#include	<signal.h>

char	*shellp	= NULL;			/* Saved "SHELL" program.	*/
char	*shname = NULL;			/* Saved shell name		*/

extern	char	*getenv();

/*
 * On System V, we no gots job control, so always run
 * a subshell using fork/exec. Bound to "C-C", and used
 * as a subcommand by "C-Z". (daveb)
 *
 * Returns 0 if the shell executed OK, something else if
 * we couldn't start shell or it exited badly.
 */
spawncli(f, n, k)
{
	extern char	*strrchr();
	register int	pid;
	register int	wpid;
	register int	(*oqsig)();
	register int	(*oisig)();
	int		status;
	int		errp = FALSE;

	if (shellp == NULL) {
		shellp = getenv("SHELL");
		if (shellp == NULL)
			shellp = getenv("shell");
		if (shellp == NULL)
			shellp = "/bin/sh";	/* Safer.		*/
		shname = strrchr( shellp, '/' ); 
		shname = shname ? shname++ : shellp;
		
	}
	ttcolor(CTEXT);
	ttnowindow();
	ttmove(nrow-1, 0);
	if (epresf != FALSE) {
		tteeol();
		epresf = FALSE;
	}
	ttclose();
	sgarbf = TRUE;				/* Force repaint.	*/
	oqsig = signal(SIGQUIT, SIG_IGN);
	oisig = signal(SIGINT,  SIG_IGN);
	if ((pid=fork()) == 0) {
		execlp(shellp, shname, "-i", NULL);
		_exit(1);			/* Should do better!	*/
	}
	else if (pid > 0) {
		while ((wpid=wait(&status))>=0 && wpid!=pid)
			;
	}
	else errp = TRUE;

	signal(SIGINT,  oisig);
	ttopen();
	if(errp)
		ewprintf("Failed to create process");

	return ( errp | status );
}
