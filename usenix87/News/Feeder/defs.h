/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

# include <sys/errno.h>
# include <sys/types.h>
# include <sys/file.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <signal.h>
# include <stream.h>
# include <string.h>
# include <osfcn.h>

# define BUFFER_SIZE	4096
# define WARNING(x)	error(x,0,0)
# define SYS_WARNING(x)	error(x,1,0)
# define FATAL(x)	error(x,0,1)
# define SYS_FATAL(x)	error(x,1,1)

struct sitelist
{   const char	*site_name;
    short	site_seen;
    sitelist	*site_next;
};

// to free memory automagically
class auto_delete
{   void	*stored_ptr;
public:
    auto_delete (void *ptr)	{ stored_ptr = ptr; }
    ~auto_delete ()		{ delete stored_ptr; }
};

// to close files automagically
class auto_close
{   int	stored_fd;
public:
    auto_close (int fd)	{ stored_fd = fd; }
    ~auto_close ()	{ int e = errno; close (stored_fd); errno = e; }
};

// feed.o
extern void		feed_news (const char *, const char **, off_t);
// log.o
extern void		open_log (const char *);
extern void		add_to_log (const char *, const sitelist *);
extern void		close_log ();
extern void		sync_log ();
extern void		reset_log ();
// batch.o
extern int		open_batch (const char **, off_t);
extern int		send_article (const char *);
extern int		close_batch ();
extern void		kill_batch ();
// io.o
extern void		fatal_input_stream_error (const char *, int);
extern void		fatal_output_stream_error (const char *);
extern int		getline (const istream&, char *, int);
extern int		append_file (const char *, const char *);
// misc.o
extern const char	*rename_error;
extern char		*new_string (const char *);
extern char		*join_string (const char *, const char *);
extern void		error (const char *, int, int);
// libC.a
extern void		(*_new_handler) ();
