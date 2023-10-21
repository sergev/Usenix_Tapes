/* Written by Stephen J. Muir, Lancaster University, England, UK.
 *
 * EMAIL: stephen@comp.lancs.ac.uk
 * UUCP:  ...!mcvax!ukc!dcl-cs!stephen
 *
 * Version 1.0
 */

# include "defs.h"

static char	*base_again, *base_hold, *base_cmd, *default_site;
static short	first_article;
static int	fd_again = -1, fd_hold = -1;
static off_t	max_batch_size;
static sitelist	*hold_list;
static ostream	*stream_again, *stream_hold;

// This routine writes the given line to the "again" file for reprocessing.
// Parameters:
//	article_line -> line to write
// Return value:
//	exits on error
static void	write_again (const char *article_line)
{   if (fd_again == -1)
    {	if ((fd_again = open (base_again,
			      O_CREAT | O_TRUNC | O_WRONLY | O_APPEND,
			      0666
			     )
	     ) == -1
	   )
	    SYS_FATAL (base_again);
	fcntl (fd_again, F_SETFD, 1);
	stream_again = new ostream (fd_again);
    }
    *stream_again << article_line << "\n";
}

// This routine writes the given article to the "hold" file.
// Parameters:
//	article -> article to write
// Return value:
//	exits on error
static void	write_hold (const char *article)
{   if (fd_hold == -1)
    {	if ((fd_hold = open (base_hold, O_CREAT | O_WRONLY | O_APPEND, 0666))
	    == -1
	   )
	    SYS_FATAL (base_hold);
	fcntl (fd_hold, F_SETFD, 1);
	stream_hold = new ostream (fd_hold);
    }
    *stream_hold << article;
    for (sitelist *site_ptr = hold_list;
	 site_ptr;
	 site_ptr = site_ptr->site_next
	)
	if (site_ptr->site_seen)
	    *stream_hold << " " << site_ptr->site_name;
    *stream_hold << "\n";
}

// This routine tries to write the given article to the current batch, starting
// one if none is current.  If the article doesn't belong to the current batch,
// it will save it for further processing.  Finally, it may add the article to
// the "hold" file.
// Parameters:
//	article_line -> a line from the batch file
// Return value:
//	exits on error
static void	feed_article (const char *article_line)
{   static short	batch_finished;
    static sitelist	*feed_list;
    register char	*c_ptr;
    register int	ret, holding = 0;
    register sitelist	*site_ptr, **site_ptr_ptr;

    // wait until the whole input is reprocessed
    if (first_article)
	batch_finished = 0;
    if (batch_finished)
    {	write_again (article_line);
	return;
    }

    // reset the "hold" list
    for (site_ptr = hold_list; site_ptr; site_ptr = site_ptr->site_next)
	site_ptr->site_seen = 0;

    register char	*new_article_line = new_string (article_line);
    auto_delete	auto_new_article_line (new_article_line);

    // split line into (article, sitelist)
    for (c_ptr = new_article_line; *c_ptr != ' ' && *c_ptr != '\0'; ++c_ptr)
	;
    while (*c_ptr == ' ')
	*c_ptr++ = '\0';
    if (*c_ptr == '\0')
	c_ptr = default_site;

    // if first article, make sure we can read it and set up the "feed" list
    if (first_article)
    {	int	site_count = 0;
	// delete any previous "feed" list
	for (site_ptr = feed_list; site_ptr; )
	{   sitelist	*delete_ptr = site_ptr;
	    site_ptr = site_ptr->site_next;
	    delete delete_ptr->site_name;
	    delete delete_ptr;
	}
	feed_list = 0;

	// the first article *must* exist
	if ((ret = access (new_article_line, R_OK)) == -1)
	{   if (errno == ENOENT)
		return;
	    SYS_WARNING (new_article_line);
	}

	// now set up the "feed" list
	for (site_ptr_ptr = &feed_list; *c_ptr; )
	{   register char	*new_c_ptr = c_ptr;

	    while (*new_c_ptr != ' ' && *new_c_ptr != '\0')
		++new_c_ptr;
	    while (*new_c_ptr == ' ')
		*new_c_ptr++ = '\0';

	    // make sure it's not in the hold list
	    for (site_ptr = hold_list; site_ptr; site_ptr = site_ptr->site_next)
		if ( ! strcmp (c_ptr, site_ptr->site_name))
		{   site_ptr->site_seen = holding = 1;
		    break;
		}

	    // if it's not in the hold list, add it to the feed list
	    if (site_ptr == 0)
	    {	sitelist	*new_entry = new sitelist;
		new_entry->site_name = new_string (c_ptr);
		//new_entry->site_seen = 1;
		*site_ptr_ptr = new_entry;
		site_ptr_ptr = &new_entry->site_next;
		++site_count;
	    }

	    c_ptr = new_c_ptr;
	}
	*site_ptr_ptr = 0;

	// write the article to the hold list (if necessary)
	if (holding)
	    write_hold (new_article_line);

	// we *must* have some sites to feed
	if ( ! site_count)
	    return;

	// start a batch
	const char	**command_args = new char *[site_count + 2],
			**args_ptr_ptr = command_args;
	*args_ptr_ptr++ = base_cmd;
	for (site_ptr = feed_list; site_ptr; site_ptr = site_ptr->site_next)
	    *args_ptr_ptr++ = site_ptr->site_name;
	*args_ptr_ptr = 0;
	if ((ret = open_batch (command_args, max_batch_size)) < 0)
	    FATAL (form ("open_batch() returns %d", ret));
	delete command_args;
	first_article = 0;
    }	else	// ! first_article
    {	// reset feed list
	for (site_ptr = feed_list; site_ptr; site_ptr = site_ptr->site_next)
	    site_ptr->site_seen = 0;

	// check each site
	while (*c_ptr)
	{   register char	*new_c_ptr = c_ptr;
	    while (*new_c_ptr != ' ' && *new_c_ptr != '\0')
		++new_c_ptr;
	    while (*new_c_ptr == ' ')
		*new_c_ptr++ = '\0';

	    // check hold list
	    for (site_ptr = hold_list; site_ptr; site_ptr = site_ptr->site_next)
		if ( ! strcmp (c_ptr, site_ptr->site_name))
		{   site_ptr->site_seen = holding = 1;
		    break;
		}

	    // if not in hold list, try the feed list
	    if (site_ptr == 0)
		for (site_ptr = feed_list;
		     site_ptr;
		     site_ptr = site_ptr->site_next
		    )
		    if ( ! strcmp (c_ptr, site_ptr->site_name))
		    {	site_ptr->site_seen = 1;
			break;
		    }

	    // if not in either list, just push it out for later
	    if (site_ptr == 0)
	    {	write_again (article_line);
		return;
	    }

	    c_ptr = new_c_ptr;
	}

	// if feed list is not complete, just push it out for later
	for (site_ptr = feed_list; site_ptr; site_ptr = site_ptr->site_next)
	    if ( ! site_ptr->site_seen)
	    {	write_again (article_line);
		return;
	    }

	// write the article to the hold list (if necessary)
	if (holding)
	    write_hold (new_article_line);
    }

    // try to add the article to the current batch
    if ((ret = send_article (new_article_line)) < 0)
	FATAL (form ("send_article() returns %d", ret));

    if (ret == 0)
	add_to_log (new_article_line, feed_list);
    else if (ret == 2)	// batch would've overflowed
    {	++batch_finished;
	if (holding)
	{   for (site_ptr = feed_list;
		 site_ptr;
		 site_ptr = site_ptr -> site_next
		)
		// yuck ...
		strcat (new_article_line, form (" %s", site_ptr->site_name));
	    write_again (new_article_line);
	} else
	    write_again (article_line);
    }
}

// This routine does everything necessary to feed news batches from a file.
// The format of each line of the input file looks like:
//	article_file
// or:
//	article_file site1 site2 ... siteN
// In the first case, the site to be fed that article is assumed to be that
// given by the name of the base_file (old batch format).
// Parameters:
//	base_file -> file containing articles to be batched
//	hold_array -> list of sites NOT to be fed
//	max_batch_size -> maximum size of batch (0 => unlimited)
// Return value:
//	exits on error
void	feed_news (const char *base_file,
		   const char **hold_array,
		   off_t max_batch_size
		  )
{   char	*base_input, *base_proc, *cp = rindex (base_file, '/'),
		buffer [BUFFER_SIZE];
    int		proc_fd;

    // make this available to the routine that needs to know
    ::max_batch_size = max_batch_size;

    // in case it's not a MULTIHOST batch ...
    default_site = cp ? cp + 1 : base_file;

    // make our filenames
    base_input = join_string (base_file, ".input");
    base_proc = join_string (base_file, ".proc");
    base_again = join_string (base_file, ".again");
    base_hold = join_string (base_file, ".hold");
    base_cmd = join_string (base_file, ".cmd");

    // open the log file
    open_log (base_file);

    // error recovery from previous run ...
    int			ret;
    if ((ret = append_file (base_input, base_proc)) < 0)
	SYS_FATAL (ret != -2 ? base_input : base_proc);

    // prepare "hold" list
    sitelist	**site_ptr_ptr = &hold_list;
    for (const char **cpp = hold_array; *cpp; ++cpp)
    {	sitelist	*new_entry = new sitelist;
	new_entry->site_name = *cpp;
	*site_ptr_ptr = new_entry;
	site_ptr_ptr = &new_entry->site_next;
    }
    *site_ptr_ptr = 0;

    // OK, time's up!  Any more articles arriving will have to wait until the
    // next time ...
    if (rename (base_file, base_input) == -1 && errno != ENOENT)
	SYS_FATAL (form (rename_error, base_file, base_input));
    if ((ret = append_file (base_input, base_proc)) < 0)
	SYS_FATAL (ret != -2 ? base_input : base_proc);

    // Main processing loop.
    while ((proc_fd = open (base_proc, O_RDONLY, 0)) != -1)
    {	auto_close auto_proc_fd (proc_fd);
	fcntl (proc_fd, F_SETFD, 1);
	istream	stream_proc (proc_fd, 0);
	first_article = 1;

	// give each line of the file to the article feeder
	while ((ret = getline (stream_proc, buffer, BUFFER_SIZE)) == 0)
	    feed_article (buffer);
	if (ret != 1)
	    fatal_input_stream_error (base_proc, ret);

	// end of file ... finish current batch (if any)
	if ( ! first_article && (ret = close_batch ()))
	    FATAL (form ("close_batch() returns %d", ret));
	sync_log ();

	// tidy up the "again" file
	if (fd_again != -1)
	{   (*stream_again).flush ();
	    if ( ! (*stream_again).good ())
		fatal_output_stream_error (base_again);
	    delete stream_again;
	    close (fd_again);
	    fd_again = -1;
	}

	// rename the "again" file to the "proc" file
	if (rename (base_again, base_proc) == -1)
	{   if (errno == ENOENT)
	    {	if (unlink (base_proc) != -1)
		    continue;
		SYS_FATAL (base_proc);
	    }
	    else
		SYS_FATAL (form (rename_error, base_again, base_proc));
	}
    }

    if (errno != ENOENT)
	SYS_FATAL (base_proc);

    // tidy up the "hold" file
    if (fd_hold != -1)
    {	(*stream_hold).flush ();
	if ( ! (*stream_hold).good ())
	    fatal_output_stream_error (base_hold);
	delete stream_hold;
    }

    // close the log file
    close_log ();
}
