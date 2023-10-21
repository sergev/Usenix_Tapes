#include "hd.h"
#include "strings.h"
#include "command.h"

/* Interface to make.  Fmake forks make off and returns.  Wmake waits
   for completion.  Both cause output to be saved in the file .makerror.
*/

fmake () 
{
    int p;	/*      Process number */
    int mfile;	/*      File number of .makerror */
    FILE *mstream;	/*      Stream of .makerror */
    FILE *showopen ();	/*      Opener of mstream */
    char inline[STRMAX];

    if (getcomment(inline))
	return NOREPLOT;

    mstream = showopen ("w", MAKEMODE);
    if (mstream == NULL) return NOREPLOT;
    mfile = fileno (mstream);

    if (myfork () == 0) 
    {
	if ((p = myfork ()) == 0) 
	{
	    close (outfile); close (errorfile);
	    dup (mfile); dup (mfile);
	    close (infile); open ("/dev/null", 0);
	    hilite("%s", inline);
	    printf("\r\n");
	    if(SHELL && *SHELL)
		myexecl (SHELL, "+", "-c", inline, 0);
	    else
		myexecl("/bin/sh", "+", "-c", inline, 0);
	}
	else 
	{
	    p = join (p);
	    beep (); sleep (2); beep ();
	    if (p) {
		sleep(1);
		beep();
	    }
	    exit (0);
	}
    }
    else 
    {
	fclose (mstream);
	putmsg (MAKE);
    }
    return NOREPLOT;
}

wmake () 
{
    int p;	/*      Process number */
    FILE *mstream;	/*      Stream of .makerror */
    FILE *showopen ();	/*      Opener of mstream */
    register ch;	/*      Work character */
    char inline[STRMAX];

    int pipefile [2];	/*      Pipe file numbers */
#	define pipein	pipefile [0]
#	define pipeout	pipefile [1]

    FILE *pipestrm;	/*      Stream of pipein */

    if (getcomment(inline))
	return NOREPLOT;

    mstream = showopen ("w", MAKEMODE);
    if (mstream == NULL) return NOREPLOT;

    tty_push(COOKEDMODE);
    pipe (pipefile);
    if ((p = myfork ()) == 0) 
    {
	close (outfile); close (errorfile);
	dup (pipeout); dup (pipeout);
	hilite("%s", inline);
	printf("\r\n");
	if(SHELL && *SHELL)
		myexecl (SHELL, "+", "-c", inline, 0);
	else
		myexecl("/bin/sh", "+", "-c", inline, 0);
    }
    else 
    {
	close (pipeout);
	pipestrm = fdopen (pipein, "r");
	while ((ch = getc (pipestrm)) != EOF) 
	{
	    putchar (ch); putc (ch, mstream);
	}
	fclose (pipestrm); fclose (mstream);
	join (p);
    }
    tty_pop ();
    return CMD_SE | REPLOT;
}

getcomment(inline) /* get user make parameters */
char *inline;
{
    register char *s, *t;

    /* get user comments for the make command */
    s = inline;
    t = MAKE;
    while (*s++ = *t++);
    --s;
    *s++ = ' ';
    tty_push(COOKEDMODE);
    clearmsg(2);
    putmsg("Make options (quit):");
    printf(" ");
    xgetline(stdin, s, STRMAX);
    tty_pop();
    if (!strcmp("q", s) || *s == (QUITCHAR-'@')
	|| !strcmp("quit", s)) {
	hilite("(Aborted)");
	return 1;
    }
    return 0;
}
