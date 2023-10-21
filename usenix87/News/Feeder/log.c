/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

# include "defs.h"
# include "config.h"

static char	*base_log;
static int	log_fd, log_size = -1;
static ostream	*stream_log;

// This routine puts a date stamp on the log file, if this hasn't been done
// yet.  It also creates the output stream for it (if necessary).
static void	start_log ()
{   static short	log_started;
    if (log_started)
	return;
    ++log_started;
    if ( ! stream_log)
	stream_log = new ostream (log_fd);
    struct timeval	tv;
    struct timezone	tz;
    gettimeofday (&tv, &tz);
    struct tm		*tp = localtime (&tv.tv_sec);
    char		*ap = asctime (tp),
			*tzn = timezone (tz.tz_minuteswest, tp->tm_isdst);
    *stream_log << form (":%lu:%.20s%s%s",
			 tv.tv_sec,
			 ap,
			 tzn ? tzn : "",
			 ap + 19
			);
}

// This routine removes those articles expected to be expired from the log file
// and returns with the log file opened (for appending) and locked.
// Parameters:
//	base_file -> file containing articles to be batched
// Return value:
//	exits on error
void	open_log (const char *base_file)
{   if ((log_fd = open (base_log = join_string (base_file, ".log"),
			O_CREAT | O_RDWR | O_APPEND,
			0666
		       )
	) == -1
       )
	SYS_FATAL (base_log);
    fcntl (log_fd, F_SETFD, 1);
    if (flock (log_fd, LOCK_EX | LOCK_NB) == -1)
	FATAL (form ("previous batching for \"%s\" is still running", base_file)
	      );
# ifdef LOG_EXPIRE_SECONDS
    istream		stream_old_log (log_fd);
    char		buffer [BUFFER_SIZE];
    register int	ret;
    if (ret = getline (stream_old_log, buffer, BUFFER_SIZE))
    {	if (ret == 1)	// empty log file
	    return;
	fatal_input_stream_error (base_log, ret);
    }
    if (buffer [0] != ':')
    {	WARNING ("bad log format -- not truncated");
	return;
    }
    long	expire_time = time (0) - LOG_EXPIRE_SECONDS;
    if ((atol (&buffer [1]) - expire_time) > 0)
	return;

    // we don't want the old log anymore
    auto_close auto_log_fd (log_fd);
    char	*base_newlog = join_string (base_file, ".newlog");
    if ((log_fd = open (base_newlog,
			O_CREAT | O_TRUNC | O_WRONLY | O_APPEND,
			0666
		       )
	) == -1
       )
	SYS_FATAL (base_newlog);
    fcntl (log_fd, F_SETFD, 1);
    if (flock (log_fd, LOCK_EX | LOCK_NB) == -1)
	SYS_FATAL (base_newlog);
    stream_log = new ostream (log_fd);
    short	transferring = 0;
    while ((ret = getline (stream_old_log, buffer, BUFFER_SIZE)) == 0)
    {	if ( ! transferring &&
	    buffer [0] == ':' &&
	    (atol (&buffer [1]) - expire_time) > 0
	   )
	    ++transferring;
	if (transferring)
	    *stream_log << buffer << "\n";
    }
    if (ret != 1)
	fatal_input_stream_error (base_log, ret);

    (*stream_log).flush ();
    if ( ! (*stream_log).good ())
	fatal_output_stream_error (base_newlog);
    if (rename (base_newlog, base_log) == -1)
	SYS_FATAL (form (rename_error, base_newlog, base_log));
# endif LOG_EXPIRE_SECONDS
}

// This routine adds an article, together with the list of sites it was sent
// to, to the log file.
void	add_to_log (const char *article_line, const sitelist *fed_sites)
{   register const sitelist	*site_ptr;
    start_log ();
    *stream_log << article_line;
    for (site_ptr = fed_sites; site_ptr; site_ptr = site_ptr->site_next)
	*stream_log << " " << site_ptr->site_name;
    *stream_log << "\n";
}

// This routine closes the log file.
void	close_log ()
{
# ifdef ALWAYS_STAMP_LOG
    start_log ();
# endif ALWAYS_STAMP_LOG
    if (stream_log)
    {	(*stream_log).flush ();
	if ( ! (*stream_log).good ())
	    fatal_output_stream_error (base_log);
	delete stream_log;
    }
}

// This routine attempts to flush all data to the log file and remember its size
// so that the next routine can truncate it if necessary.
void	sync_log ()
{   if ( ! stream_log)
	FATAL ("sync_log(): log not open");
    (*stream_log).flush ();
    if ( ! (*stream_log).good ())
	fatal_output_stream_error (base_log);
    if ((log_size = lseek (log_fd, 0, L_INCR)) == -1)
	SYS_WARNING ("sync_log()");
}

// This routine attempts to truncate the log to the size determined by the
// previous routine.
void	reset_log ()
{   if (stream_log && log_size != -1)
	if (ftruncate (log_fd, log_size) == -1)
	    SYS_WARNING ("reset_log()");
}
