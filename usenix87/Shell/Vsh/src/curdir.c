#
/*  Curdir finds the name of the working directory and puts in
    wdname.  This is done by calling on the pwd program.
*/
#include "hd.h"
#include "mydir.h"
static char pwd[] = 
{
    "/bin/pwd"
} ;

curdir () 
{

    int p, rdlen, pipefile [2];
#define pipein	pipefile [0]
#define pipeout	pipefile [1]

    pipe (pipefile);
    printf ("%s\r\n", pwd);
    if ((p = myfork ()) == 0) 
    {
	close (outfile); dup (pipeout);
	close (pipein); close (pipeout);
	execl (pwd, pwd, 0);
	exit (1);
    }
    else 
    {
	close (pipeout);
	join (p);
	rdlen = read (pipein, wdname, sizeof wdname);
	if (rdlen < 2 || rdlen == sizeof wdname || wdname [0] != '/') 
	{
	    printf ("Cannot find name of working directory\r\n");
	    return 1;
	}
	wdname [rdlen - 1] = 0;
	close (pipein);
    }
    return 0;
}
