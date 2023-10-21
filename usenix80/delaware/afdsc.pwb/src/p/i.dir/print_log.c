#
#define max_uid 256

 /* routine to print the indent log */


char   *unms[max_uid];				/* pointers to user names */


main () {
struct logtmpl {				/* structure of a log entry */
    int     tvec[2];				/* time of execution */
    char    inp;				/* input fid */
    char    outp;				/* output fid */
    int     nout;				/* # output lines */
    int     ncom;				/* # comments */
    int     wcom;				/* # lines w/ comments */
    int     wcode;				/* # lines w/code */
    char    mc;					/* max line size */
    char    ci;					/* comment indentation */
    char    inds;				/* indent size */
    char    dci;				/* decl comment indentation */
    char    verb;				/* verbose */
    char    ljus;				/* left just */
    char    lvcom;				/* leave commas */
    char    unin;				/* unindented comment indentation */
    char    uid;				/* the user id */
    char    btype_2;				/* true if braces on right */
    int     reserved[2];
};

struct logtmpl  logent;


int     log_fid;				/* fid of log file */
int	id;
register char  *pwptr;
char    buf[256];
char   *ptime ();
char   *getpw ();
char    text[5000];
register char  *free;



    free = text;
    log_fid = open ("/mnt/net/willcox/indent/indent_log", 0);
    if (log_fid < 0) {
	printf ("Can't open log file\n");
	exit ();
    }

    while (read (log_fid, &logent, sizeof logent) > 0) {
	id = logent.uid & 0377;

	if (unms[id] == 0) {			/* check if uid has been encountered before */
	    getpw (id, buf);
	    unms[id] = free;
	    pwptr = buf;
	    do {
		*free++ = *pwptr;
	    } while (*pwptr++ != ':');
	    *free++ = '\0';
	}

	printf ("%s %s", unms[id], ctime (logent.tvec));
	if (logent.inp != 0) {
	    printf ("    Non-standard input");
	    if (logent.outp != 1)
		printf ("   Non-standard output\n");
	    else
		printf ("\n");
	}

	printf ("    Lines: %d, coms: %d, lines w/coms %d, lines w/code %d\n",
		logent.nout, logent.ncom, logent.wcom, logent.wcode);
	printf ("    Args: ");
	printf ("-l%d -c%d -i%d -dc%d -d%d", logent.mc, logent.ci, logent.inds,
		logent.dci, logent.unin);
	if (logent.verb)
	    printf (" -v");
	if (logent.ljus)
	    printf (" -dj");
	if (logent.lvcom)
	    printf (" -nbc");
	if (logent.btype_2)
	    printf (" -br");
	printf ("\n\n");
    }
}
