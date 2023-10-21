/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

# include "defs.h"

static short		batch_error;
static int		pid, output_channel = -1, (*old_pipe_signal) ();
static off_t		current_size, max_size;
static struct stat	statbuf;

// This routine sets up a pipe to the enqueueing command.
// Parameters:
//	command -> command to queue the batch for transfer
//	new_max_size -> maximum batch size
// Return value:
//	-2: couldn't create pipe
//	-1: invalid invocation (pipe already open)
//	 0: ok (as far as the parent process can tell)
open_batch (const char **command, off_t new_max_size)
{   if (output_channel != -1)
	return -1;
    max_size = new_max_size;
    int		pipes [2];
    if (pipe (pipes) == -1)
	return -2;
    // process table could be full, in which case we may be in trouble anyway,
    // but at least it's not our fault for not reporting it
    while ((pid = fork ()) == -1)
	sleep (1);
    if (pid == 0)
    {	// child process ...
	close (pipes [1]);
	if (dup2 (pipes [0], 0) == -1)
	    SYS_FATAL ("dup2()");
	close (pipes [0]);
	execv (*command, command);
	// I hope this works!
	SYS_FATAL (*command);
    }
    close (pipes [0]);
    old_pipe_signal = signal (SIGPIPE, SIG_IGN);
    output_channel = pipes [1];
    batch_error = 0;
    current_size = 0;
    return 0;
}

// This routine closes the pipe and (hopefully) reports whether or not it
// worked.
// Return value:
//	-2: something went wrong!
//	-1: invalid invocation (pipe not open)
//	 0: ok (as far as we can tell)
close_batch ()
{   if (output_channel == -1)
	return -1;
    close (output_channel);
    output_channel = -1;
    signal (SIGPIPE, old_pipe_signal);
    int		status, wait_pid;
    while ((wait_pid = wait (&status)) != pid && wait_pid != -1)
	;
    return (wait_pid == -1 || status) ? -2 : 0;
}

// This routine adds a file to the batch.  If this would cause batch overflow to
// occur, the article is not added, assuming the caller will try again.
// Parameters:
//	pathname -> article to be included
// Return value:
//	-3: error (batch write error)
//	-2: error (article read error)
//	-1: invalid invocation (pipe not open or current batch in error)
//	 0: ok (article added to batch)
//	 1: ok (article didn't exist -- maybe it was cancelled/expired)
//	 2: ok (article would overflow batch -- article not included)
send_article (const char *pathname)
{   if (output_channel == -1 || batch_error)
	return -1;
    register int	article_fd = open (pathname, O_RDONLY, 0);
    if (article_fd == -1)
	return 1;
    auto_close	auto_article (article_fd);
    if (fstat (article_fd, &statbuf) == -1)
	return 1;
    char	*rnews_header = form ("#! rnews %d\n", statbuf.st_size);
    int		rnews_header_size = strlen (rnews_header);
    if (current_size > 0 &&
	max_size > 0 &&
	(current_size + rnews_header_size + statbuf.st_size) > max_size
       )
	return 2;
    char		buffer [BUFFER_SIZE];
    register int	byte_count;
    current_size += rnews_header_size + statbuf.st_size;
    if (write (output_channel, rnews_header, rnews_header_size) !=
	rnews_header_size
       )
    {	batch_error = 1;
	return -3;
    }
    while ((byte_count = read (article_fd, buffer, BUFFER_SIZE)) > 0)
	if (write (output_channel, buffer, byte_count) != byte_count)
	{   batch_error = 1;
	    return -3;
	}
    if (byte_count)
    {	batch_error = 1;
	return -2;
    }
    return 0;
}

// This routine tries to kill the current batch.
void	kill_batch ()
{   if (output_channel != -1)
	kill (pid, SIGTERM);
}
