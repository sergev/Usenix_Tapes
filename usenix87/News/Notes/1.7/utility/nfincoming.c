#include <stdio.h>

/*
 * a more secure login shell for "rsh host nfrcv nf myhost"
 *
 *	sender puts a line:
 *		host:x:::rsh host -l signon nfrcv %s %s
 *	in his net.how file.
 *	"host" is the target host
 *	"signon" is a signon on "host" that has this program as
 *	a login shell.
 *
 * Ray Essick, Septembet 1986
 */

extern char **environ;

int     Debug = 0;

main (argc, argv)
int     argc;
char  **argv;
{
    int     i,
            j,
            k;
    int     status;					/* from system */
    char   *p,
           *q,
           *r;
    char  **pp,
          **qq,
          **rr;

    /* 
     * check that the arg list is satisfactory
     */

    if (argc != 3)
    {
	fprintf (stderr, "%s: incorrect arg count\n", argv[0]);
	exit (1);
    }
    if (strcmp (argv[1], "-c") != 0)
    {
	fprintf (stderr, "Usage: %s -c \"nfrcv nf host\"\n", argv[0]);
	exit (2);
    }

    /* 
     * verify that the "nfrcv nf host" line is correct
     * make sure there are no meta-characters in there.
     * make sure he can't crack out
     *
     */

    p = argv[2];
    while (*p && *p != ' ')
	p++;
    i = *p;
    *p = '\0';
    if (strcmp (argv[2], "nfrcv") != 0)
    {
	fprintf (stderr, "%s: 'nfrcv' is the only allowed command\n",
		argv[0]);
	exit (3);
    }
    *p = i;						/* restore the byte */

    /* 
     * compare against a list of meta characters
     */
    p = argv[2];
    while (*p)
    {
	if (index ("`!\"#$%&'[]\\;<>*{}?|~^", *p) != NULL)
	{
	    fprintf (stderr, "%s: meta-character '%c' in '%s' not permitted\n",
		    argv[0], *p, argv[2]);
	    exit (4);
	}
	p++;						/* to next character */
    }
    /* 
     * fix the SHELL environment variable
     */
    if (Debug)
    {
	printf ("environ before:\n");
	system ("printenv");
    }
    pp = environ;
    while (p = *pp)
    {							/* for each environment value */
	if (strncmp ("SHELL=", p, 6) == 0)
	{
	    *pp = "SHELL=/bin/sh";
	    break;
	}
	pp++;
    }
    if (Debug)
    {
	printf ("doing printenv\n");
	system ("printenv");
    }
    status = execl ("/bin/sh", "sh", "-c", argv[2], 0);	/* do it */
    /* 
     * should never return, but just in case it does
     * fail:
     */
    exit (status);
}
