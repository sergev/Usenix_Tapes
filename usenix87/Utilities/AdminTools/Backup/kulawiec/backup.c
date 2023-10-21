/*
*	backup.c		Rich Kulawiec, PUCC, 1985
*
*	Run dumps according to a fixed schedule encoded in file CONTROL.
*
*	File contains:
*
*	main()		-- main program, of course
*	check_sched()	-- examine a schedule line for information
*	already_done()	-- check to see if dumps already done today
*	ask_for_tape()	-- ask the operator to mount the appropriate tape
*	verify()	-- prompt operator to confirm tape load
*	verifytape()	-- check tape label file for correctness
*	write_label()	-- write a tape label file
*	rundump()	-- build a dump command and run it
*	query()		-- ask a question and get an answer
*	tapeskip()	-- skip one file on the tapedrive
*	offline()	-- take tapedrive offline
*	message()	-- print a message on the terminal
*
*	Defining DEBUG changes backup's idea of the tape drive, and
*	causes it to echo "dump" and "mt" commands instead of running them.
*	It will still run dump W as needed, though.
*	Depending on what's left in here, it may also print some
*	additional information as  it runs.
*
*	Defining ALLINONE causes all partial dumps to be placed on
*	the same tape.  It would probably be a bad idea to mix partial
*	dump levels on that tape.
*
*	Defining VERIFYTAPE causes backup to check the tape for a header
*	indicating whether or not it's the right tape.  Note that leaving
*	this undefined will still let backup query the operator before
*	firing up dump to scribble on the tape.
*/

#ifndef lint
static char *rcsid = "$Header: /usr/src/etc/RCS/backup.c,v 1.13 85/08/19 19:49:42 rsk Exp $";
#endif lint
/*
 * $Log:	backup.c,v $
 * Revision 1.13  85/08/19  19:49:42  rsk
 * Changed the handling of offline() to fix a buglet; if backuup
 * ran epoch dumps using a schedule file whose last line did not
 * have a '0' in the same position as the '0' for the epoch dumps
 * that were run (confusing isn't it?) then what happened was that
 * Level was set to, say, 'x', and the test at the end of main
 * succeeded...and backup tried to take the tapedrive offline after
 * already doing so.
 * Therefore, the following was done:
 * rundump() now calls offline() (1) all the time if ALLINONE is
 * not defined (2) for all epoch dumps if ALLINONE is defined.
 * To take care of multiple partial dumps, a variable "Rewind"
 * was created; if it is set, then main() will call offline()
 * just prior to exiting.  The only place that sets Rewind is
 * inside verifytape() -- and it sets it iff it's looking at
 * a partial tape and is running with ALLINONE defined.
 * 
 * Revision 1.12  85/08/18  20:34:51  rsk
 * Added several (void)'s in front of calls to offline()
 * to satisfy lint.
 * 
 * Revision 1.11  85/08/18  16:17:35  rsk
 * Several changes.
 * If DEBUG is defined, then MT_OFFLINE is fixed so that
 * it just rewinds the tape rather than taking it offline.
 * The series of if statements in verifytape() are now
 * chained with else clauses.
 * When write_label() is called in verifytape(), verifytape()
 * calls itself to check the results.
 * Added several calls to offline() in verifytape() to
 * make sure bad tape gets noticed.
 * Took out the level sanity check in write_label() since it
 * really didn't do much.
 * Fixed a small bug in write_label(); if ALLINONE is defined
 * and this is a partial tape, then should write PARTIAL_LABEL
 * rather than the name of the filesystem.  This only shows up
 * if a SPARE tape is used for a partial and then the order
 * of partial dumps gets re-arranged later, but it's fixed
 * anyhow.
 * 
 * Revision 1.10  85/08/17  13:14:58  rsk
 * Updated list of functions at top to reflect reality.
 * 
 * Revision 1.9  85/08/16  13:01:13  rsk
 * Rearranged functions to reflect logical order.
 * 
 * Revision 1.8  85/08/16  11:29:22  rsk
 * Added a little debugging code to already_done(); fixed up the
 * function headers to more accurately reflect what goes on in
 * each function.
 * 
 * Revision 1.7  85/08/16  10:58:42  rsk
 * Ripped out pseudo-exit routine from last night.  Simplified
 * exit codes.  Switched calls to rundump() and verify(), since
 * an accidental change last night had backup trying to verify
 * the tape after running the dump.  Made verify return a value
 * even though it's ignored, 'cause (a) might use it later,
 * and (b) couldn't figure out how to recursively return a void.
 * 
 * Revision 1.6  85/08/16  02:01:24  rsk
 * Added a new routine that's called instead of exit() with
 * a wealth (10, actually) of exit codes; might be handy if
 * this is run by a script someday.  Will probably trim the
 * list tomorrow, as well as fixing a known bug.
 * The program is definitely easier manage now that it's
 * broken up into chunks, although the growth in the number
 * of (void) functions and global variables isn't very aesthetic.
 * 
 * Revision 1.5  85/08/16  01:31:44  rsk
 * Many changes.  Several new functions, which have taken chunks
 * of the large loop that used to be in main() and broken them
 * up into little pieces.  Tapeok is now a global, among other
 * things, and is used when putting multiple partial dumps on
 * one tape to avoid attempting to reverify the tape after it's
 * been done once.   Most new functions are (void) since if they
 * fail we have to abort anyway; no sense testing for a return
 * value and then aborting.
 * 
 * Revision 1.4  85/08/16  00:00:42  rsk
 * Made Filesys, Tapedrive, and Schedline global
 * prepatory to breaking the large loop in main()
 * up into several function calls.
 * 
 * Revision 1.3  85/08/14  18:45:52  rsk
 * Parameterized the sizes of a number of characters arrays.
 * Used STRSIZE (255) for devices, filesystems, command lines,
 * hostnames, lines from the schedule file, and replies from
 * the operator.
 * Used DATESTRSIZE (30) for dates...since all dates are in
 * ctime() format, which takes 26 characters, this should
 * be sufficient.
 * Only non-parameterized array is "weekmask", which is
 * set to 7.  This will need to be changed if another
 * day is added to the week.
 * 
 * Revision 1.2  85/08/12  20:16:00  rsk
 * Added changes for "spare" tapes, including auto-labelling.
 * Slightly changed what happens when dump fails if DEBUG is defined.
 * Took #ifdef DEBUG off write_label() routine.
 * 
 * Revision 1.1  85/08/09  16:13:05  rsk
 * Initial revision
 * 
*/

#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>

#define	OKAY		0
#define STRSIZE		255	/* Should be large enough for most everything */
#define DATESTRSIZE	30	/* Should only need to be 26 (for ctime) */
#define	FAIL		-1
#define	DUMPFLAGS	"ufd"
#define	DENSITY		"6250"
#define	DUMPINFOFLAG	"W"
#define	MT_SKIP		"fsf 1"
#define	MYNAME		"BACKUP"
#define	SPARE_LABEL	"SPARE"

#ifdef ALLINONE
#define PARTIAL_LABEL	"PARTIAL"
#endif ALLINONE

#ifdef DEBUG
#define	DUMP		"echo /etc/dump"
#define	TAPE		"dev.rmt8"
#define	NTAPE		"dev.nrmt8"
#define	CONTROL		"backup.list"
#define	MT		"/bin/echo mt -f"
#define	MT_OFFLINE	"rewind"
#else DEBUG
#define	DUMP		"/etc/dump"
#define	TAPE		"/dev/rmt8"
#define	NTAPE		"/dev/nrmt8"
#define	CONTROL		"/etc/backup.list"
#define	MT		"/bin/mt -f"
#define	MT_OFFLINE	"offline"
#endif DEBUG

#define	GO		"go"
#define STOP		"stop"

/*
* Different classes of errors; provides additional error message on exit.
*/

#define ST_OKAY		0
#define ST_NOWORK	1
#define ST_DUMP		2
#define ST_ABORT	3
#define ST_CTL		4
#define ST_TAPE		5
#define ST_SYSCALL	6
#define ST_INTERNAL	7

char	Command[STRSIZE];	/* constructed shell commands */
char	Date[DATESTRSIZE];	/* current date/time in ctime() format */
char	Hostname[STRSIZE];	/* the name of this host */
char	Filesys[STRSIZE];	/* name of filesystem(s) to dump */
char	Tapedrive[STRSIZE];	/* name of tapedrive to use */
char	Schedline[STRSIZE];	/* a line read out of schedule file */
char	Weekmask[7];		/* one slot per day of week */
char	Level;			/* level of this dump */
#ifdef ALLINONE
int	Tapeok = FAIL;		/* Has this tape been verified? */
int	Rewind = FAIL;		/* Does tape need to be rewound at end? */
#endif ALLINONE

struct tm	*localtime();
FILE	*popen();
void	message();
void	check_sched();
void	ask_for_tape();
void	rundump();
char	*strcpy();
char	*strncpy();
char	*index();
char	*rindex();
char	*gets();
long	time();

main()
{
	FILE	*fp;
	long	timebuf;		/* seconds since epoch */
	struct	tm *ldate;		/* date broken down into fields */

	if(gethostname(Hostname,sizeof(Hostname)) == FAIL) {
		message("gethostname() failed");
		exit(ST_SYSCALL);
	}

	(void) time(&timebuf);
	(void) strcpy(Date,ctime(&timebuf));
	ldate = localtime(&timebuf);

	if( (fp = fopen(CONTROL,"r")) == NULL) {
		message("can't open control file");
		exit(ST_SYSCALL);
	}

	/*
	* Read the schedule file; make rudimentary checks for integrity.
	*/

	while( fscanf(fp,"%s",Schedline) != FAIL) {

	/*
	* Break out the fields of this line of the schedule.
	*/

		check_sched(ldate->tm_wday);

	/*
	* If this line of the schedule file pertains to this machine,
	* and if there's a digit in the dump schedule for today,
	* then we probably have some work to do.
	*/

		if( (strncmp(Schedline,Hostname,strlen(Hostname)) == OKAY)
			&& isascii(Level) && isdigit(Level) ) {

	/*
	* Check to see if we've already dumped this filesystem today.
	*/

			if( already_done() == OKAY ){
				(void) printf("%s: filesystem %s has already been dumped today\n",MYNAME,Filesys);
				if(query ("Do you want to dump it again?") != OKAY)
					continue;
			}
	/*
	* Request the proper tape from the operator.
	*/
			ask_for_tape();

	/*
	* If ALLINONE is defined, then first pass through here will
	* set Tapeok to OKAY, assuming it's the right tape,
	* when we're putting multiple partial dumps on one tape.
	* Successive passes won't bother to verify the tape.
	*/

#ifdef ALLINONE
			if(Level == '0' )
				(void) verify();
			else 
				if(Tapeok == FAIL)
					(void) verify();
#else ALLINONE
			(void) verify();
#endif ALLINONE

	/*
	* Build the call the dump, and run it.  This does the real work.
	* Note that rundump() also has the responsibility for rewinding
	* the tape and taking the drive offline if needed.
	*/
			rundump();
		}
	}
	message("finished");
	(void) fclose(fp);

	/*
	* If in ALLINONE mode, then take tape drive offline at the
	* end of series of partials.  If these were epoch dumps,
	* the drive will already be offline...see above.
	* Note: this is one of the places where assumptions about
	* NOT mixing epoch and partial dumps are important.
	*/

#ifdef ALLINONE
	if (Rewind == OKAY)
		(void) offline();
#endif ALLINONE

	exit(ST_OKAY);
}
/*
* check_sched() -- examine line of schedule file for work to do
*
* Parameters: day_of_week -- part of ldate structure
*
* Global Variables:	Schedline, Filesys, Weekmask, Level
*
* Returns: nothing
*
* Side Effects: sets Filesys, Weekmask, and Level according to schedule line
*
*/
void check_sched(day_of_week)
int day_of_week;
{
	char	*charptr;		/* scratch character pointer */


	if( (charptr = rindex(Schedline,':')) == NULL) {
		message("Garbled control file");
		exit(ST_CTL);
	}
	(void) strcpy(Filesys,++charptr);	/* got filesystem */

	if( (charptr = index(Schedline,':')) == NULL) {
		message("Garbled control file");
		exit(ST_CTL);
	}
	(void) strncpy(Weekmask,++charptr,7);	/* got dump levels */

	/*
	* Get today's dump level.
	*/

	Level = Weekmask[day_of_week];
}
/*
* already_done() -- see if this filesystem was dumped today
*
* Parameters:	none
*
* Global Variables: Command, Date, Filesys, Level
*
* Returns:	OKAY -- if already dumped Filesystem at level Level today.
*		FAIL -- otherwise
*
* Side Effects: runs "dump W" to gather information
*
*/
already_done()
{
	char	last_device[STRSIZE];	/* device filesystem is mounted on */
	char	last_filesys[STRSIZE];	/* name of filesystem */
	char	last_level;		/* level of last dump */
	char	last_date[DATESTRSIZE];	/* ctime format date of last dump */
	char	replybuf[STRSIZE];	/* holds a line of reply from dump W */
	FILE	*sp;			/* stream pointer */
	int	returnval;		/* temporary place for return value */

	returnval = FAIL;

	/*
	* Build a command line for dump that will cause it to print
	* its idea of which filesystems need to be dumped.
	* Do this rather than reading /etc/dumpdates and /etc/fstab
	* and deciphering everything ourself.
	*/

	(void) sprintf(Command,"%s %c%s",DUMP,Level,DUMPINFOFLAG);
	(void) fflush(stdout);

	/*
	* Run the command, and read its output.
	*/

	if( (sp = popen(Command,"r")) == NULL) {
		message("popen() failed");
		exit(ST_SYSCALL);
	}

	/*
	* Eat first line of dump's verbose output
	*/
	
	if(fscanf(sp,"%[^\n]\n",replybuf) == FAIL) {
		message("can't fscanf() reply from dump");
		exit(ST_SYSCALL);
	}

	/*
	* Grab all necessary fields out of dump's output.
	* Slight trickiness; must eat everything up to the first '('
	* in order to get filesystem name; thus, last_device
	* might contain some garbage characters.  We don't care, since
	* we don't really use it.  Then we eat everything to ')'
	* in order to get filesystem name.
	*/

	while (fscanf(sp,"%[^(]( %[^)]) Last dump: Level %c, Date %[^\n]\n",last_device,last_filesys,&last_level,last_date) != FAIL) {

	/*
	* If this line of "dump W" output pertains to the filesystem in
	* question, and if the dump level matches the current one,
	* AND if the date of the last dump appears to be today,
	* then it would seem that this filesystem has been dumped today.
	* Note that fact, and just keep going; must read all of dump's
	* output to avoid causing SIGPIPE on early pclose().
	*/
		if(strncmp(last_filesys,Filesys,strlen(Filesys)) == OKAY 
			&& last_level == Level
			&& strncmp(last_date,Date,10) == OKAY)
			returnval = OKAY;
	}

	if( pclose(sp) == FAIL) {
		message("pclose() failed");
		exit(ST_SYSCALL);
	}

#ifdef DEBUG
	(void) printf("already_done() returning %s\n",returnval == OKAY ? "OKAY" : "FAIL");
	return(query("Type go for OKAY(yes), stop for FAIL(no)."));
#else DEBUG
	return(returnval);
#endif DEBUG
}
/*
* ask_for_tape() -- request the operator to mount the proper tape
*
* Parameters: none
*
* Global Variables: Tapedrive, Hostname, Filesys, Date
*
* Returns: nothing
*
* Side Effects: none
*
*/
void ask_for_tape()
{

	/*
	* Ask for the proper tape; use rewind device for epochs,
	* no-rewind device for incrementals.
	* Note that if this tape has already been verified, i.e.
	* we compiled with ALLINONE defined and Tapeok has been
	* set to OKAY by a previous call to verify(), then we do nothing.
	*/

	if(Level == '0') {
		(void) strcpy(Tapedrive,TAPE);
		(void) printf("%s: get the FULL backup tape for machine %s, filesystem %s for %.3s\n",MYNAME,Hostname,Filesys,Date);
	}
	else {
		(void) strcpy(Tapedrive,NTAPE);
#ifdef ALLINONE
		if (Tapeok == FAIL)
			(void) printf("%s: get the PARTIAL backup tape for machine %s for %.3s\n",MYNAME,Hostname,Date);
		
#else ALLINONE
		(void) printf("%s: get the PARTIAL backup tape for machine %s, filesystem %s for %.3s\n",MYNAME,Hostname,Filesys,Date);
#endif ALLINONE

	}
}
/*
* verify() -- prompt operator to mount tape
*
* Parameters:	none
*
* Global Variables: none
*
* Returns:	OKAY if tape verified.
*
* Side Effects: exits if operator wants to quit
*
*/
verify()
{
	if( query("Is the correct tape mounted?") == OKAY) {
		if( verifytape() == OKAY) {
			message("tape verified");
			return(OKAY);
		}
		else {
			message("\007\007\007INCORRECT TAPE MOUNTED!!");
			message("Get the correct tape!!");
			return(verify());
		}
	}
	else {
		if( query ("Do you want to continue the backup?") == OKAY) {
			return(verify());
		}
		else {
			message("aborting on operator command");
			exit(ST_ABORT);
		}
	}
	/*
	* We should never get here; abort if it happens.
	*/

	message("Something very bad has happened in verify()");
	exit(ST_INTERNAL);
	
}
/*
* verifytape() -- check tape label vs. current filesystem
*		  Is smart enough to recognize a spare tape; also
*		  recognizes partial tape labels if in ALLINONE mode.
*
* Parameters:	none
*
* Global Variables: Hostname, Date, Filesys, Level
*
* Returns:	OKAY -- if this is the correct w.r.t. filesys & level
*		FAIL -- otherwise
*
* Side Effects: leaves tape positioned at end of label file if correct tape.
*		writes a new label if this is a spare tape
*		set the Rewind flag if ALLINONE is defined and this is
*			a multiple partial dump tape
*
*/
verifytape()
{
#ifdef VERIFYTAPE
	char	last_hostname[STRSIZE];	/* hostname as written on tape */
	char	last_date[DATESTRSIZE];	/* date as written on tape */
	char	last_filesys[STRSIZE];	/* filesystem as written on tape */
	char	last_level;		/* dump level as written on tape */
	FILE	*fp;

	if( (fp = fopen(TAPE,"r")) == NULL) {
		message("can't open tapedrive");
		exit(ST_SYSCALL);
	}

#ifdef DEBUG
	(void) printf("%s: searching for label: %s %.3s %s %c\n",MYNAME,Hostname,Date,Filesys,Level);
#endif DEBUG

	if(fscanf(fp,"%s %s %s %c",last_hostname,last_date,last_filesys,&last_level) == FAIL) {
		message("can't read tape label");
		exit(ST_SYSCALL);
	}

#ifdef DEBUG
	(void) printf("%s:    found this label: %s %.3s %s %c\n",MYNAME,last_hostname,last_date,last_filesys,last_level);
#endif DEBUG

	(void) fclose(fp);

	/*
	* If the label appears to be okay, i.e. all four fields match
	* our idea of what should be there, then skip over the label
	* file and get ready to run dump.
	*/

	if( strcmp(Hostname,last_hostname) == OKAY
		&& strncmp(Date,last_date,3) == OKAY
		&& strcmp(Filesys,last_filesys) == OKAY
		&& Level == last_level) {
		(void) tapeskip();
		return(OKAY);
	}

#ifdef ALLINONE

	/*
	* If we're dealing with multiple partial dumps on one tape,
	* then we need to check for the special label marking this
	* tape; therefore, a slightly modified test is made.
	*
	* Must also set Rewind so that tape will be taken care of
	* when all dumps are done, just before we exit...see the
	* end of main().
	*/

	else if( strcmp(Hostname,last_hostname) == OKAY
		&& strncmp(Date,last_date,3) == OKAY
		&& strcmp(PARTIAL_LABEL,last_filesys) == OKAY
		&& Level != '0'
		&& Level == last_level) {
		Tapeok = OKAY;
		Rewind = OKAY;
		(void) tapeskip();
		return(OKAY);
	}

#endif ALLINONE

	/*
	* This might be a spare tape; if so, ignore the date and dumplevel
	* fields; just check the host, and look for SPARE_LABEL in place
	* of the filesystem name.
	*/

	else if (strcmp(Hostname,last_hostname) == OKAY
		&& strncmp(last_filesys,SPARE_LABEL,strlen(SPARE_LABEL)) == OKAY) {
		message("This looks like a spare tape.");
		if( query("Go ahead and use it?") == OKAY) {
			if( write_label() == OKAY) {
				return(verifytape());
			}
			else {
				(void) offline();
				return(FAIL);
			}
		}
		else {
			(void) offline();
			return(FAIL);
		}
	}
	else {
		(void) offline();
		return(FAIL);
	}

#else VERIFYTAPE
	return(OKAY);
#endif VERIFYTAPE
}
/*
* write_label() -- write a label on a spare tape
*
* Parameters:	none
*
* Global Variables:	Hostname, Date, Filesys, Level
*
* Returns:	OKAY if label successfully written
*		FAIL otherwise
*
* Side Effects: writes a tiny label file on the beginning of the tape
*
*/
write_label()
{
	FILE	*fp;

	if( (fp = fopen(TAPE,"w")) == NULL) {
		message("can't open tapedrive");
		exit(ST_SYSCALL);
	}

#ifdef ALLINONE
	if( Level == '0')
		fprintf(fp,"%s %.3s %s %c\n",Hostname,Date,Filesys,Level);
	else
		fprintf(fp,"%s %.3s %s %c\n",Hostname,Date,PARTIAL_LABEL,Level);
#else ALLINONE
	fprintf(fp,"%s %.3s %s %c\n",Hostname,Date,Filesys,Level);
#endif ALLINONE

	(void) fclose(fp);

	return(OKAY);
}
/*
* rundump() -- call the dump program to do the real work
*
* Parameters: none
*
* Global Variables: Level, Tapedrive, Filesys, Command
*
* Returns: nothing
*
* Side Effects: runs the actual dump
* 		queries the operator for advice if dump fails
*
*/
void rundump()
{
	int	status;			/* return status of dump command */

	/*
	* Build the proper command line to call dump.
	*/

	(void) sprintf(Command,"%s %c%s %s %s %s",DUMP,Level,DUMPFLAGS,Tapedrive,DENSITY,Filesys);

	(void) printf("%s: running %s\n",MYNAME,Command);
	(void) fflush(stdout);

	/*
	* Call dump; check the return status to make sure it's alright.
	*/

	if( (status = system(Command)) != OKAY) {
		(void) printf("%s: dump failed, status=%d\n",MYNAME,(status>>8));
#ifdef DEBUG
		if( query("Want to go on?") == FAIL) {
			(void) offline();
			exit(ST_ABORT);
		}
#else DEBUG
		(void) offline();
		exit(ST_DUMP);
#endif DEBUG
	}

#ifdef ALLINONE
	if(Level == '0')
		(void) offline();
#else ALLINONE
	(void) offline();
#endif ALLINONE
}
/*
* query(question) -- prints "question" on the tty, then waits for answer
*
* Parameters:	question -- a null-terminated string
*
* Global Variables: none
*
* Returns:	OKAY -- if question eventually answered with GO
*		FAIL -- if question eventually answered with STOP
*
* Side Effects: recursive routine
*
*/
query(question)
char	*question;
{
	char	reply[STRSIZE];		/* Whatever the user types in reply */

	(void) printf("%s: %s <Answer %s or %s only>: ",MYNAME,question,GO,STOP);
	/*
	* Print the question and wait for an answer.
	* If we don't understand the answer, retry (recursively).
	*/

	if( gets(reply) == NULL) {
		message("can't talk to console");
		exit(ST_SYSCALL);
	}
	else if( strcmp(reply,GO) == 0 ) {
		return(OKAY);
	}
	else if( strcmp(reply,STOP) == 0) {
		return(FAIL);
	}
	else {
		message("incorrect response, try again");
		return(query(question));
	}

	/*
	* We should never get here; abort if it happens.
	*/

	message("Something very bad has happened in query()");
	exit(ST_INTERNAL);
}
/*
* takeskip() -- skip forward one file on tape
*
* Parameters: none
*
* Global Variables:	Command
*
* Returns:	OKAY
*
* Side Effects: skips forward one file on the tape via "mt"
*
*/
tapeskip()
{
	int	status;

	(void) sprintf(Command,"%s %s %s",MT,NTAPE,MT_SKIP);

	if( (status = system(Command)) != OKAY) {
		(void) printf("%s: mt failed, status=%d\n",MYNAME,(status>>8));
		exit(ST_TAPE);
	}

	return(OKAY);
}
/*
* offline() -- takes tapedrive offline
*
* Parameters:	none
*
* Global Variables:	Command, Tapedrive
*
* Returns:	OKAY
*
* Side Effects:	calls "mt" to do the work; leaves tape drive off line.
*
*/
offline()
{
	int	status;

	(void) sprintf(Command,"%s %s %s",MT,Tapedrive,MT_OFFLINE);

	if( (status = system(Command)) != OKAY) {
		(void) printf("%s: mt failed, status=%d\n",MYNAME,(status>>8));
		exit(ST_TAPE);
	}

	return(OKAY);
}
/*
* message(msg) -- print a message labelled with name of this program on tty
*
* Parameters:	msg -- the string to print
*
* Global Variables: none
*
* Returns: nothing
*
* Side Effects: none
*
*/
void message(msg)
char	*msg;
{
	(void) printf("%s: %s\n",MYNAME,msg);
}
