/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

# include "defs.h"
# include "config.h"

# if !defined(DEBUG) && defined(ERROR_LOG)
static int	error_fd;
# endif
static ostream	*error_stream;

const char	*rename_error = "rename(%s,%s)";

// This routine prints an error message (and may exit).
// Parameters:
//	string -> error message
//	perror_style -> adds a system error message by looking up "errno"
//	fatal -> exits at end
// Return value:
//	exits if "fatal" is set or on error
void	error (const char *string, int perror_style, int fatal)
{
# if !defined(DEBUG) && defined(ERROR_LOG)
    char	*error_log = ERROR_LOG;	// saving space !!!
    if ( ! error_stream)
    {	if ((error_fd = open (error_log, O_WRONLY | O_APPEND, 0)) != -1)
	{   fcntl (error_fd, F_SETFD, 1);
	    error_stream = new ostream (error_fd);
	} else
	    perror (error_log);
    }
# else
    error_stream = &cerr;
# endif
    if (error_stream)
    {	*error_stream << "newsfeed: "
		      << (fatal ? "fatal: " : "warning: ")
		      << string
		      << (perror_style ? ": " : "")
		      << (perror_style ? (errno < sys_nerr ? sys_errlist [errno]
							   : "Unknown error"
					 )
				       : ""
			 )
		      << "\n";
	(*error_stream).flush ();
    }
    if (fatal)
    {	kill_batch ();
	reset_log ();
	exit (1);
    }
}

// This routine returns a copy of its parameter.
char	*new_string (const char *old_string)
{   char	*string_ptr = new char [strlen (old_string) + 1];
    strcpy (string_ptr, old_string);
    return string_ptr;
}

// This routine returns the concatenation of its first and second parameter.
char	*join_string (const char *first_string, const char *second_string)
{   return new_string (form ("%s%s", first_string, second_string));	}
