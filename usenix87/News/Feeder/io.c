/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

# include "defs.h"

// This routine prints an error message and exits.
// Parameters:
//	filename -> file on which the error occurred
//	code -> cause of error
void	fatal_input_stream_error (const char *filename, int code)
{   register char	*cp;
    switch (code)
    {	case -3:
	    cp = "stream error";
	    break;
	case -2:
	    cp = "incomplete last line";
	    break;
	case -1:
	    cp = "line too long";
	    break;
	case 1:
	    cp = "unexpected end of file";
	    break;
	default:
	    cp = form ("something(code %d) is badly wrong", code);
	    break;
    }
    FATAL (form ("%s: %s", filename, cp));
}

// This routine prints an error message and exits.
// Parameters:
//	filename -> file on which the error occurred
void	fatal_output_stream_error (const char *filename)
{   FATAL (form ("%s: output stream error", filename));	}

// This routine attempts to read the next line from the given input stream.
// Parameters:
//	infile -> input stream
//	buffer -> where to put the line
//	buffer_size -> size of the above
// Return value:
//	-3: stream failure (irrecoverable)
//	-2: last line not terminated by a newline
//	-1: line too long (irrecoverable)
//	 0: ok
//	 1: end of file
getline (const istream& infile, char *buffer, int buffer_size)
{   register char	c = '\0';
    if (infile.get (buffer, buffer_size))
    {	if (infile.get (c))
	    return (c == '\n') ? 0 : -1;
	return infile.eof () ? -2 : -3;
    }
    return infile.eof () ? 1 : -3;
}

// This routine attempts to copy the first file (if it exists) to the end of
// the second (which is created if it doesn't exist).  Then, the first file is
// removed.
// Parameters:
//	from -> name of input file
//	to -> name of output file
// Return Value:
//	-3: error in unlinking input file
//	-2: error writing output file
//	-1: error reading input file
//	 0: ok
//	 1: no input file
//	 2: output file was created
append_file (const char *from, const char *to)
{   register int	ret = 0, count, from_fd, to_fd;
    if ((from_fd = open (from, O_RDONLY, 0)) == -1)
	return (errno == ENOENT) ? 1 : -1;
    auto_close auto_from_fd (from_fd);
    if (access (to, F_OK) == -1)
	ret = 2;
    if ((to_fd = open (to, O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1)
	return -2;
    auto_close auto_to_fd (to_fd);
    char	buffer [BUFFER_SIZE];
    while ((count = read (from_fd, buffer, BUFFER_SIZE)) > 0)
	if (write (to_fd, buffer, count) != count)
	    return -2;
    if (count < 0 || close (from_fd) == -1)
	return -1;
    if (close (to_fd) == -1)
	return -2;
    if (unlink (from) == -1)
	return -3;
    return ret;
}
