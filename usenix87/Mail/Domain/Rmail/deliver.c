/*
**  Deliver.c
**
**  Routines to effect delivery of mail for rmail/smail. 
**
*/

#ifndef lint
static char 	*sccsid="@(#)deliver.c	1.13   (UUCP-Project/CS)   7/6/86";
#endif

# include	<sys/types.h>
# include	<stdio.h>
# include	<pwd.h>
# include	"defs.h"

extern int exitstat;		/* set if a forked mailer fails */
extern enum edebug debug;	/* how verbose we are 		*/ 
extern char hostname[];		/* our uucp hostname 		*/
extern enum ehandle handle;	/* what we handle		*/


/*
**
**  deliver():  hand the letter to the proper mail programs.
**
**  Copies stdin to a temp file, if more than one mailer 
**  will be invoked.  Issues one command for each different 
**  host of <hostv>, constructing the proper command for
**  LOCAL or UUCP mail.  Note that LOCAL mail has blank
**  host names.  The <userv> names for each host are arguments 
**  to the command.  Reads the "From_" and ">From_" lines 
**  of the letter to build a <from> argument.  Finally, 
**  prepends a "From" line to the letter just before going 
**  out, with a "remote from <hostname>" if it is a UUCP letter.
**
*/

deliver( argc, hostv, userv, formv )
int argc;				/* number of addresses		*/
char *hostv[];				/* host names			*/
char *userv[];				/* user names			*/
enum eform formv[];			/* form for each address	*/
{
	FILE *file;			/* source (stdin or temp file)	*/
	FILE *out;			/* pipe to mailer		*/
	FILE *popen();			/* to fork a mailer 		*/
#ifdef RECORD
	FILE *record();			/* snoopy mailer		*/
#endif
	char *tmpf = "/tmp/rmXXXXXX";	/* temp file name		*/
	char lcommand[SMLBUF];		/* local command issued 	*/
	char rcommand[SMLBUF];		/* remote command issued	*/
	char *command;			/* actual command		*/
	char from[SMLBUF];		/* accumulated from argument 	*/
	char line[SMLBUF];		/* one line of the letter 	*/
	int plural = 0;			/* more than one mailer? 	*/
	enum eform form;		/* holds form[i] for speed 	*/
	long size;			/* number of bytes of message 	*/
	time_t now, time();		/* the time 			*/
	char *nows;			/* the time in a string		*/
	int i, j, stat;
	char *c, *ctime();
	int failcount = 0;

	(void) strcpy( from, "" );
	(void) time( &now );		/* set the time */
	nows = ctime( &now );

/*
**  Decide if we should copy the letter to a temp file.  We do this if we
**  are sending to more than one mailer or if we are making a record.  We
**  check to see if there is more than one different host in hostv.
*/

# ifdef RECORD
	plural++;
# else
	for( i = 1; i < argc; i++ )
		if( strcmp( hostv[0], hostv[i] ) && ++plural )
			break;		/* shouldn't be case sensitive */
# endif

/*
**  If plural (more than one copy), copy stdin to a file.  Otherwise, just
**  set file pointer to stdin.  This way, we avoid an extra copy of the
**  file for the 99% of the cases with only one destination.  We also assume
**  that if we must return a failure to the sender, that stdin is a file
**  we can rewind and seek on, which is true of uuxqt.
*/
	if( plural ) 
	{
		char *mktemp();

		( void ) mktemp( tmpf );
		if( ( file = fopen( tmpf, "w+" ) ) == NULL )
			error( EX_CANTCREAT, "can't create %s.\n", tmpf );
		while( fgets( line, SMLBUF, stdin ) != NULL )
			(void) fputs( line, file );
	} else {
		file = stdin;
	} 
/*
**  We pass through the list of addresses.
*/
	for( i = 0; i < argc; i++ )
	{
/*
**  If form == ERROR, either the address was bad or it has been sent on a
**  previous pass.  So we break out.
*/
		form = formv[i];
		if ( form == ERROR )
			continue;
/*
**  Rewind the input file for multiple copies (won't be stdin).  Call
**  rline() to read the first few lines to collapse a From argument
**  from the From_ and >From_ lines.  Rline() leaves the first unused
**  line in line.
*/
		if ( plural )
			rewind( file );
		rline( file, line, from );
/*
**  Build the command name based on whether this is local mail or
**  uucp mail.  Someday, this will use a form->mailer table.
*/
		(void) sprintf( lcommand, LMAIL( from, hostv[i] ) );
		(void) sprintf( rcommand, RMAIL( from, hostv[i] ) );
/*
**  For each address with the same host name and form, append the user
**  name to the command line, and set form = ERROR so we skip this address
**  on later passes. 
*/
		for ( j = argc - 1; j >= i; j-- ) {
			if ( formv[j] != form || strcmp( hostv[i], hostv[j] ) )
				continue;
			c = lcommand + strlen( lcommand );
			if (form == LOCAL)
				(void) sprintf( c, LARG( userv[j] ) );
			else
				(void) sprintf( c, RLARG(hostv[i], userv[j]) );
			c = rcommand + strlen( rcommand );
			(void) sprintf( c, RARG( userv[j] ) );
			formv[j] = ERROR;
		}
retry:
		if (form == LOCAL)
			command = lcommand;
		else
			command = rcommand;
		ADVISE( "COMMAND: %s\n", command );
/*
** Fork the mailer and set it up for writing so we can send the mail to it,
** or for debugging divert the output to stdout.
*/
		if ( debug == YES )
			out = stdout;
		else {
			failcount = 0;
			do {
				out = popen( command, "w" );
				if (out) break;
				/*
				 * Fork failed.  System probably overloaded.
				 * Wait awhile and try again 10 times.
				 * If it keeps failing, probably some
				 * other problem, like no uux or sendmail.
				 */
				(void) sleep(60);
			} while (++failcount < 10);
		}
		if( out == NULL )
		{
			exitstat = EX_UNAVAILABLE;
			(void) printf( "couldn't execute %s.\n", command );
			continue;
		}
/*
**  Output our From_ line.
*/
	if ( form == LOCAL ) {
#ifdef SENDMAIL
		(void) fprintf( out, LFROM( from, nows, hostname ) );
#else
		char *p;
		if((p=index(from, '!')) == NULL) {
			(void) fprintf( out, LFROM( from, nows, hostname ) );
		} else {
			*p = NULL;
			(void) fprintf(out, RFROM( p+1, nows, from));
			*p = '!';
		}
#endif
	} else {
		(void) fprintf( out, RFROM( from, nows, hostname ) );
	}
/*
**  Copy input.  Remember that line from rline().
*/
		size = 0;
		do {
			(void) fputs( line, out );
			size += strlen( line );
		} while( fgets( line, SMLBUF, file ) != NULL );

/*
**  Get exit status and if non-zero, set global exitstat so when we exit
**  we can indicate an error.
*/
		if ( debug != YES ) {
			if ( stat = pclose( out ) )
				exitstat = stat >> 8;
			/*
			 * handle==ALL means we're smail, else we're rmail.
			 * The check here prevents a smail<=>sendmail loop.
			 * The form check prevents an internal smail loop.
			 */
			if (handle != ALL && form != LOCAL && exitstat != 0) {
				/*
				 * RMAIL failed, probably because the host
				 * being uux'ed isn't in L.sys.  Try again
				 * using sendmail.  If there is no sendmail,
				 * we should include code ala the old rmail
				 * which mails it back to the sender.
				 */
				rewind(file);	/* assume uuxqt file */
				rline( file, line, from );
				form = LOCAL;
				exitstat = 0;	/* don't bother uuxqt */
				ADVISE("uux failed %d, trying sendmail\n", exitstat);
				goto retry;
			}
		}
	}
/*
**  Update logs and records.  Blech.
*/

# ifdef LOG
	log( command, from, size ); /* */
# endif
# ifdef RECORD
	rewind( file );
	out = record( command, from, size );
	if(out != NULL) {
		while( fgets( line, SMLBUF, file ) != NULL ) {
			(void) fputs( line, out );
		}
		(void) fclose( out );
	}
# endif

/*
**  Close and unlink temp file if we made one.
*/
	if ( plural )
	{
		(void) fclose( file );
		(void) unlink( tmpf );
	}
}


/*
**
**  rline(): collapse From_ and >From_ lines.
**
**  Same idea as the old rmail, but also turns user@domain to domain!user. 
**  Leaves the first line of the letter in "line".
**
*/

rline( file, line, from )
FILE *file;				/* source file			*/
char *line;				/* return first unused line	*/
char *from;				/* return accumulated from arg 	*/
{
	int parts;			/* for cracking From_ lines ... */
	char *partv[16];		/* ... apart using ssplit() 	*/
	char user[SMLBUF];		/* for rewriting user@host	*/
	char domain[SMLBUF];		/* "   "         "          	*/
	char addr[SMLBUF];		/* "   "         "          	*/
	enum eform form, parse();	/* "   "         "          	*/
	extern build();			/* "   "         "          	*/
	struct passwd *pwent, *getpwuid();		/* to get default user		*/
	char *c;

	(void) strcpy( from, "" );
	(void) strcpy( addr, "" );
	(void) strcpy( line, "" );
/*
**  Read each line until we hit EOF or a line not beginning with "From "
**  or ">From " (called From_ lines), accumulating the new path in from
**  and stuffing the actual sending user (the user name on the last From_ 
**  line) in addr.
*/
	for( ;; )
	{
		if ( fgets( line, SMLBUF, file )==NULL )
			break;
		if ( strncmp( "From ", line, 5 ) 
		    && strncmp( ">From ", line, 6 ) )
			break;
/*
**  Crack the line apart using ssplit.
*/
		if( c = index( line, '\n' ) );
			*c = '\0';
		parts = ssplit( line, ' ', partv );
/*
**  Tack host! onto the from argument if "remote from host" is present.
*/

		if ( parts > 3 
		    && !strncmp( "remote from ", partv[parts-3], 12 ) )
		{
			(void) strcat( from, partv[parts-1] );
			(void) strcat( from, "!" );
		} 
/*
**  Stuff user name into addr, overwriting the user name from previous 
**  From_ lines, since only the last one counts.  Then rewrite user@host 
**  into host!user, since @'s don't belong in the From_ argument.
*/
		(void) strncpy( addr, partv[1], partv[2]-partv[1]-1 ); 
		addr[partv[2]-partv[1]-1] = '\0';	/* ugh */

		(void) parse( addr, domain, user );
		if(*domain == '\0') {
			form = LOCAL;
		} else {
			form = UUCP;
		}

		build( domain, user, form, addr );
	}
/*
**  Now tack the user name onto the from argument.
*/
	(void) strcat( from, addr );
/*
**  If we still have no from argument, we have junk headers, but we try
**  to get the user's name using /etc/passwd.
*/
	if ( !*from )
		if ( ( pwent = getpwuid( getuid() ) ) == NULL )
			(void) strcpy( from, "nowhere" );	/* bad news */
		else
			(void) strcpy( from, pwent->pw_name );
}


# ifdef LOG
log(command, from, size)
char *command, *from;
long size;
{
	FILE *fd;
	char *logtime, *ctime();
	time_t t;
	int cmask;

	( void ) time( &t );
	logtime = ctime( &t );
	logtime[16] = 0;
	logtime += 4;

	cmask = umask(0);
	fd = fopen( LOG, "a" );
	(void) umask(cmask);

	if ( fd != NULL ) {
		(void) fprintf( fd, "%s: %s, from %s, %ld bytes\n", 
			logtime, command, from, size);
		(void) fclose( fd );
	}
}
# endif

# ifdef RECORD
FILE *
record( command, from, size)
char *command, *from;
long size;
{
	FILE *fd;
	char *logtime, *ctime();
	long t;
	int cmask;

	( void ) time( &t );
	logtime = ctime( &t );
	logtime[16] = 0;
	logtime += 4;

	cmask = umask(0);
	fd = fopen( RECORD, "a" );
	(void) umask(cmask);

	if ( fd != NULL ) {
		(void) fprintf( fd, "%s: %s, from %s, %ld bytes\n", 
			logtime, command, from, size);
	}
	return(fd);
}
# endif
