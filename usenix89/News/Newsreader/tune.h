/*
**	buffer size needed for tmpnam()
*/
#ifndef L_tmpnam
#define L_tmpnam 48
#endif

/*
**	maximum number of columns on terminal.  If made smaller, there
**	will be a savings in the size of the temporary file used
**	for holding displays, at the penalty of not being able to use
**	the entire screen width on terminals actually possessing more
**	columns than this.  A block roughly on the order of this value
**	times the number of lines the terminal has is maintained in
**	the temp file, and read / written as displays are interacted
**	with.  MIN_C put here because MAX_C > MIN_C.  MIN_C is the minumum
**	number of columns for which a "reasonable" display can be produced.
**	before making it smaller, look at all uses of C_allow and variable
**	to see that a setting that small won't screw up array bounds.
*/
#define MAX_C 132
#define MIN_C 36

/*
**	large size for general purpose local buffers.  only used in automatic
**	variable declarations.  Used with fgets for buffer size when reading
**	file records, to hold pathnames, commands, etc.  Reduce if you blow
**	out stack storage.  If reduced too far, will eventually show up
**	as syntax errors on reading .newsrc's and the active list, and
**	scrozzled article information arising from truncated header lines.
**	The reply path line will probably be the first thing to cause trouble.
**	Look through the reader to find the worst case chain of declarations
**	(on the order of 12 or so is probably the max).
*/
#define RECLEN 1200

/*
**	to protect against reading entire articles to find non-existent header
**	lines if an article should be hosed, only a limited number of records
**	are searched.  Should be big enough to get down to the "Lines" header
**	entry on legitimate articles.
*/
#define HDR_LINES 18	/* records of article to search for header line */

/* these determine some static array sizes */
#define OPTLINES 60	/* maximum number of option lines in .newsrc */
#define NUMFILTER 24	/* max number of filters on articles */

/* block sizes for allocation routines */
#define STRBLKSIZE 1800	/* string storage allocation block */
#define NDBLKSIZE 50	/* NODE structures to allocate at a time */
