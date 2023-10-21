/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

# include "defs.h"
# include "config.h"

static void	new_handler ()
{   FATAL ("out of memory");
}

// This program does a batched feed of several sites simultaneously.  After
// changing directory to BATCH_OUT_DIR (in feed_news()), the sites in base_file
// are fed.  If any (optional) hold sites are given, they are not fed but are
// saved for a future run.  This could be done if, say, those sites are down
// for an extended period or perhaps ...
main (int argc, char **argv)
{   register int	fd;
    register off_t	max_size = (off_t)DEFAULT_MAX_SIZE;
    _new_handler = &new_handler;

    // just in case we've been given a duff environment
    while ((fd = open ("/dev/null", O_RDWR, 0)) != -1 && fd < 3)
	;
    if (fd != -1)
	close (fd);
    if (argc < 2 || (**++argv == '-' && argc < 3))
    {	cerr << "usage: newsfeed [-max_size] base_file [list_of_hold_sites]\n";
	exit (1);
    }
# ifndef DEBUG
# ifdef NICENESS
    if (setpriority (PRIO_PROCESS, getpid (), NICENESS) == -1)
	SYS_WARNING ("setpriority()");
# endif NICENESS
# ifdef NEWSGID
    if (setgid (NEWSGID) == -1)
	SYS_WARNING ("setgid()");
# endif NEWSGID
# ifdef NEWSUID
    if (setuid (NEWSUID) == -1)
	SYS_WARNING ("setuid()");
# endif NEWSUID
    umask (022);	// We are (usually) not invoked by a user.
# ifdef BATCH_OUT_DIR
    char	*batch_out_dir = BATCH_OUT_DIR;	// saving space !!!
    // go to where it's at!
    if (chdir (batch_out_dir) == -1)
	SYS_FATAL (batch_out_dir);
# endif BATCH_OUT_DIR
# endif DEBUG

    if (**argv == '-')
	max_size = (off_t)atol (*argv++ + 1);
    feed_news (*argv, argv + 1, max_size);
    exit (0);
}
