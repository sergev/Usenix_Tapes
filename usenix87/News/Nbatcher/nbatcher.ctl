*
*	NBATCHER.CTL
*	Edit and install in your NEWSLIB directory.
*
*  Comments start with *; data lines look like:
*	site:hours:bits:queue_size:command
*  Where
*	site		= name of the remote site
*	hour		= when to do work (* is all, off is never, 22-4 is ok)
*	bits		= passed on to compress via -b; null gets default
*	queue_size	= Max # bytes allowed in UUCP queue before postponing
*	command		= optional command line
*
*  See manpage and README for more info.
