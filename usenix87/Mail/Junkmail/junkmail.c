
/*==========================================================
 *
 * junkmail - delete undesired mail from /usr/spool/mail/*
 *
 * Copyright, Purdue University Computing Center, 1982, all
 * rights reserved.
 *
 * Author: V. Abell
 * Modifications: R. Kulawiec 8/83 added -u flag.
 *		  Rewrote a lot of the code to handle buffering.
 *		  R. Kulawiec 10/84 added audit trail for -u flag.
 *				    added "reason for deletion"
 *				    added extra line on rewritten headers
 *		  R. Kulawiec 6/85  rewritten headers now conform to RFC822.
 *
 *	junkmail [-a] [-w] [-dn[-m]] [-u username] [directory]
 *
 *		-a selects an audit trail.
 *
 *		-w specifies that shortened files are to
 *		   be written, containing letters that
 *		   indicate what has been deleted.
 *		   Otherwise, only an audit is performed.
 *
 *		-d specifies that the following number, n,
 *		   or range of numbers, n-m, defines the
 *		   age of discarded letters. A letter
 *		   newer than n days will only be discarded
 *		   if the mail file size exceeds the limit
 *		   by 100%. A letter older than m days will
 *		   be discarded. n defaults to 7; m, to 21.
 *		   If a single value is specified, it is
 *		   taken to be m, and n is assumed to be 7.
 *		   Of course, m must be > n.
 *
 *		-u username specifies that all mail from user
 *		   "username" be deleted, regardless of age
 *		   or size.
 *
 *		[directory] is the optional mail directory
 *		   name - "/usr/spool/mail" is the default.
 *		
 *
 *==========================================================
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<time.h>

#define	LINE_LEN	1024		/* Maximum length of a line */
#define LOC_TIME_ZONE 	5*60		/* local time zone offset (EST) */
#define	FILENAME_LEN	128		/* Length of individual filenames */
#define	DIRECTORY_LEN	128	 	/* Length of directory name */
#define USERNAME_LEN	128		/* Maximum length of any originator */


#define	FAILED		-1		/* Failure return code from call */	
#define FAILED_LONG	-1l		/* Failure return code from call */
#define SUCCEED		0		/* Success return code from call */
#define SUCCEED_LONG	0l		/* Success return code from call */


#define DIRECT	"/usr/spool/mail/"	/* directory name for mail */

#define SENDER	"MAILER-DAEMON"		/* audit letter sender */

#define MAX_AGE  	21L	    	/* maximum letter age (default) */
#define MIN_AGE  	7L	    	/* minimum letter age (default) */

#define PROTECT_MODE 	0600		/* protection mode */

#define MAX_SIZE  	50000	    	/* nominal maximum mailbox size */
#define ABS_MAX_SIZE 	2*MAX_SIZE	/* absolute maximum mailbox size */

#define NEWLINE		'\n'		/* Ascii newline character */

char Selectname[USERNAME_LEN];		/* name of user whose mail to blast */
char Directory[DIRECTORY_LEN] = DIRECT;	/* working directory (default)	*/
char Filename[FILENAME_LEN];		/* individual files as we go */

int	audit_flag = 0;			/* Audit trail is selected */
int	rewrite_flag = 0;		/* Rewriting of files will be done */
int	select_flag = 0;		/* Individual user has been selected */
int	date_flag = 0;			/* User has overridden a date(s) */

long max_age = MAX_AGE;			/* maximum letter age */
long min_age = MIN_AGE;			/* minimum letter age */

char	*day_table[7]	= {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

int	month_size[12]	= {31,28,31,30,31,30,31,31,30,31,30,31};

char	*month_table[12] = {"Jan","Feb","Mar","Apr","May","Jun",
			    "Jul","Aug","Sep","Oct","Nov","Dec"};
struct zone {
	int hours_west;		/* hours west of Greenwich */
	char *st;		/* standard time zone name */
	char *dl;		/* daylight time zone name */
} z_tab[] = {
	4*60, "AST", "ADT",	/* Atlantic */
	5*60, "EST", "EDT",	/* Eastern */
	6*60, "CST", "CDT",	/* Central */
	7*60, "MST", "MDT",	/* Mountain */
	8*60, "PST", "PDT",	/* Pacific */
	0000, "GMT", "GMT",	/* Greenwich */
	-1			/* end of table indicator */
};

struct 	stat	Sbuf;		/* for return values from stat() call */
struct	direct	Dir_entry;	/* directory structure */

#ifdef vax
DIR	*Dfd;
#else
int	Dfd;			/* directory file descriptor */
#endif

FILE	*Mfp;				/* mail file pointer	     */
FILE	*Tfp;				/* temp file pointer	     */

char	*Tempfile = "/tmp/junkXXXXXX"; /* Temporary file for copying */

char	sndl[512];			/* "MAIL~SYSTEM" sender line */
int	compare_length = USERNAME_LEN;	/* longest originator length */
char	Buffer[LINE_LEN];		/* Input/output mail buffer  */
char	Headbuf[LINE_LEN];		/* Save area for mail header */
long	File_length	= 0l;		/* Length of input mail file */

char	*ctime();			/* Time conversion	     */
char	*mktemp();			/* Temporary filename maker  */

#ifndef vax
char	machine();			/* Machine identifier	     */
#endif

int	Copy_flag	= 1;	/* Whether or not to copy bytes through */

long	bytes_del;		/* Number of bytes deleted from this file */
				/* (or that could be deleted).  Note that */
				/* this does NOT account for bytes written */
				/* by junkmail itself.			*/
long	tot_bytes_del	= 0;	/* Total number of bytes deleted in run	*/
int	files_short	= 0;	/* Total number of files shortened	*/
int	files_del	= 0;	/* Total number of zero-length files deleted */

char	Loginname[255];		/* The name of the file in /usr/spool/mail. */
				/* Set to 255 to avoid unpleasant surprises */
				/* should a filename > 8 chars creep in.    */

main(argc, argv)
int argc;
char *argv[];
{
	int i;
	long time();
	long time_buf;
	char	hoststr[32];

	i = 1;

	do {
	if(argc != 0)
		if(argc > 1 && strncmp(argv[i],"-",1) == 0 ) {
			if(strncmp(argv[i],"-a",2) == 0) {	/* audit */
				audit_flag = 1;
				continue;
			}
			if(strncmp(argv[i],"-w",2) == 0) {	/* rewrite */
				rewrite_flag = 1;
				continue;
			}
			if(strncmp(argv[i],"-d",2) == 0) {	/* date ch. */
				date_flag = 1;
				if( agerange(argv[i]) == FAILED)
					usage();
				continue;
			}
			if(strncmp(argv[i],"-u",2) == 0) {	/* target */
				if( get_selectname(argv[++i]) == FAILED )
					usage();
				select_flag = 1;
				rewrite_flag = 1;
				continue;
			}
			usage();
		}
		if(argc > 1) {					/* chdir */
			if( get_dirname(argv[i])  == FAILED )
				usage();
			continue;
		}
	}
	while(++i < argc);

	if(select_flag && date_flag) {
		fprintf(stderr,"Use of -u and -d together not allowed\n");
		usage();
	}

	time(&time_buf);

#ifdef vax
	if(gethostname(hoststr,sizeof(hoststr)) == FAILED)
		fprintf(stderr,"Gethostname() failed!\n");	
#else
	sprintf(hoststr,"pucc-%s",machine());
#endif

	if(audit_flag)
		printf("\n%s, %s\n",hoststr,ctime(&time_buf));

	if(rewrite_flag)
		sprintf(sndl,"From %s %sReceived: by %s, %sDate: %sFrom: <pucc> %s\nTo: ",SENDER,ctime(&time_buf),hoststr,ctime(&time_buf),ctime(&time_buf),SENDER);

	if(rewrite_flag)
		Tempfile = mktemp(Tempfile);

	if(check_dirname() == FAILED)
		usage();

	if(select_flag)
		printf("Deleting all mail from user %s\n\n",Selectname);

	while( get_filename() != FAILED) {
		if(select_flag) {
			if(del_user()==FAILED)
				exit(1);
		}
		else {
			if(del_oldbig() == FAILED)
				exit(1);
		}
		if(audit_flag && (bytes_del != 0) ) {
			printf("\t%s",Filename);
			if(rewrite_flag)
				printf(" was shortened by "); 
			else
				printf(" could be shortened by ");
			printf("%ld bytes\n",bytes_del);
		}
	}
	if(audit_flag) {
		printf("----------\n");
		printf("%d files",files_short);
		if(rewrite_flag)
			printf(" were shortened by "); 
		else
			printf(" could be shortened by ");
		printf("%ld bytes\n",tot_bytes_del);
		if( !select_flag) {
			printf("%d zero-length files",files_del);
			if(rewrite_flag)
				printf(" were deleted\n"); 
			else
				printf(" could be deleted\n");
		}
		printf("----------\n");
	}
}
/*==================================================
 *
 * usage() - print argument usage message and exit. 
 *
 *
 *==================================================
 */

usage()
{
	fprintf(stderr,"Usage: junkmail [-a] [-w] [-dn[-m]] [-u username] [directory]\n");
	exit(1);
}

/*==================================================
 *
 * get_selectname(p) - Get the username pointed to.
 *
 *
 *==================================================
 */

get_selectname(p)
char *p;
{
	if(strlen(p) > USERNAME_LEN)		/* Username can't be more */
		return(FAILED);			/* than this long         */
	if(strlen(p) <= 0 )			/* Username can't this    */
		return(FAILED);			/* long either 		  */
	else {
		strcpy(Selectname,p);
		return(SUCCEED);
	}
}

/*==================================================
 *
 * get_dirname(p) - Get the directory name pointed to.
 *
 *
 *==================================================
 */

get_dirname(p)
char *p;
{
	if(strlen(p) > DIRECTORY_LEN)		/* Username can't be more */
		return(FAILED);			/* than this long         */
	if(strlen(p) <= 0 )			/* Username can't this    */
		return(FAILED);			/* long either 		  */
	else {
		strcpy(Directory,p);
		return(SUCCEED);
	}
}

/*==========================================================
 *
 * agerange(p) - assemble the age range represented by the
 *		 characters at (p).
 *
 *	exit	return = 0 if range assembled
 *			-1 if assembly error detected
 *		(min_age) = minimum age
 *		(max_age) = maximum age
 *
 *==========================================================
 */

agerange(p)
char *p;
{
	int n;			/* number being assembled */
/*
 *	Assemble first number.
 */
	p += 2;			/* Skip over the "-d" flag	*/

	for(n = 0; *p >= '0' && *p <= '9'; p++)
	    n = 10 * n + *p - '0';
	if(*p != '\0') {
	    if(*p++ != '-')
		return(FAILED);
/*
 *	If the first number is terminated by a minus sign ("-"),
 *	assemble the second number of an "n-m" range.
 */
	    min_age = (long) n;
	    for(n = 0; *p >= '0' && *p <= '9'; p++)
		n = 10 * n + *p - '0';
	    }
	max_age = (long) n;
	if(min_age <= 0l || max_age <= 0l || min_age >= max_age)
	    return(FAILED);
	return(SUCCEED);
}

/*======================================================================
 *
 * long dysince(a) - convert the ctime() format date, addressed by (a)
 *		     to the number of days since that date.
 *
 *	exit	return = -1L if the date cannot be converted
 *
 *======================================================================
 */
long dysince(a)
char	*a;
{

	int day;		/* day */
	int hours;		/* hours */
	int minutes; 		/* minutes */
	int month; 		/* month */
	int seconds;		/* seconds */
	int year;		/* year */
	int z = LOC_TIME_ZONE;	/* time zone offset */
	long time_buf;		/* time buffer */
	long time_sec = 0l;	/* time in seconds */
	register int i; 	/* temporary index */
/*
 *	Check and skip day of week.
 */
	if(ckalpha(a,3) == 0)
	    return(FAILED_LONG);
	for(i = 0; i < 7; i++)
	    if(strncmp(a,day_table[i],3) == 0)
		break;
	if(i > 6)
	    return(FAILED_LONG);
	a += 3;
	if(*a++ != ' ')
	    return(FAILED_LONG);
/*
 *	 Check month and convert to integer.
 */
	if(ckalpha(a,3) == 0)
	    return(FAILED_LONG);
	for(month = 0; month < 12; month++)
	    if(strncmp(a,month_table[month],3) == 0)
		break;
	if(month > 11)
	    return(FAILED_LONG);
	a += 3;
	while(*a == ' ') {
	    if(*a == '\0')
		return(FAILED_LONG);
	    a++;
	    }
/*
 *	Convert day number.
 */
	day = atoi(a);
	while(*a >= '0' && *a <= '9')
	    a++;
	while(*a == ' ') {
	    if(*a == '\0')
		return(FAILED_LONG);
	    a++;
	    }
/*
 *	Convert time.
 */
	hours = atoi(a);
	while(*a >= '0' && *a <= '9')
	    a++;
	if(hours < 0 || hours > 23 || *a++ != ':')
	    return(FAILED_LONG);
	minutes = atoi(a);
	while(*a >= '0' && *a <= '9')
	    a++;
	if(minutes < 0 || minutes > 59 || *a++ != ':')
	    return(FAILED_LONG);
	seconds = atoi(a);
	while(*a >= '0' && *a <= '9')
	    a++;
	if(seconds < 0 || seconds > 59 || *a++ != ' ')
	    return(FAILED_LONG);
/*
 *	Convert time zone and year.
 */
	if((*a >= 'a' && *a <= 'z') || (*a >= 'A' && *a <= 'Z')) {
	    if(ckalpha(a,3) == 0)
		return(FAILED_LONG);
	    for(i = 0; (z = z_tab[i].hours_west) >= 0; i++) {
		if(strncmp(a,z_tab[i].st,3) == 0
		   || strncmp(a,z_tab[i].dl,3) == 0)
		    break;
		}
	    if(z == -1)
		return(FAILED_LONG);
	    while(*a < '0' || *a > '9') {
		if(*a == '\0')
		    return(FAILED_LONG);
		a++;
		}
	    }
	minutes += z;
	year = atoi(a);
/*
 *	Check year.
 */
	if(year < 1970)
	    return(FAILED_LONG);
/*
 *	Cope with February in leap/non-leap years.
 */
	if(leap(year))
	    month_size[1] = 29;
	else
	    month_size[1] = 28;
	if(day < 1 || day > month_size[month])
	    return(FAILED_LONG);
/*
 *	Calculate seconds since the epoch.
 */
	for(i = 1970; i < year; i++)
	    time_sec += (leap(i) ? 366 : 365);
	while(--month >= 0)
	    time_sec += month_size[month];
	time_sec += (long) day - 1l;
	time_sec = time_sec*24l + (long) hours;
	time_sec = time_sec*60l + (long) minutes;
	time_sec = time_sec*60l + (long) seconds;
/*
 *	Return days from today as (seconds from today).
 *				   ------------------
 *				      24 * 60 * 60
 */
	time(&time_buf);
	if(time_sec > time_buf)
	    return(FAILED_LONG);
	return((time_buf - time_sec) / 86400l);
}

/*=============================================
 *
 * leap(y) - test (y) for a leap year
 *
 *	exit	return = 0 if not a leap year
 *			 1 if a leap year
 *
 *=============================================
 */

leap(y)
int y;
{
	return(y%4 == 0 && y%100 != 0 || y%400 == 0);
}

/*====================================================================
 *
 * ckalpha(a,n) - check for n consecutive alpha characters, starting 
 *		  at (a).
 *
 *	exit	return = 0 if non-alpha character encountered
 *			 1 if characters available
 *
 *====================================================================
 */

ckalpha(a,n)

char *a;
int n;

{
	register int i; 	/* temporary index */
	
	for(i = 0; i < n; i++)
		if( (! isascii(*a)) || (! isalpha(*a)) )
			return(0);
	return(1);
}

/*====================================================================
 *
 * check_dirname() - Verify that "Directory" exists, is a 
 *			directory, and is readable.
 *
 *	exit	return = 0 if a legitimate directory has been found
 *			-1 otherwise
 *
 *====================================================================
 */
check_dirname()
{

	if( stat(Directory,&Sbuf) == FAILED) {
		fprintf(stderr,"Can't stat %s\n",Directory); 
		return(FAILED);
	}
	if( ((Sbuf.st_mode & S_IFDIR) == 0) || Sbuf.st_ino == 0 ) {
		fprintf(stderr,"%s is not a directory\n",Directory);
		return(FAILED);
	}
#ifdef vax
	if( (Dfd = opendir(Directory)) == NULL) {
#else
	if( (Dfd = open(Directory,0)) == FAILED) {
#endif
		fprintf(stderr,"Can't open directory %s\n",Directory);
		return(FAILED);
	}
	return(SUCCEED);
}	
/*====================================================================
 *
 * get_filename() - Get the next file in directory "Directory".
 *
 *	exit	return = 0 if a legitimate file has been found
 *			-1 otherwise
 *
 *		Note: "Reads through" '.' and '..' transparently.
 *
 *====================================================================
 */
get_filename()
{
	int	pathlength;
#ifdef vax
	struct direct *dirent;
#endif
	/* Add tailing slash to directory name if user forgot it */

	if(Directory[strlen(Directory)-1] != '/')
		strcat(Directory,"/");

	strcpy(Filename,Directory);
	pathlength = strlen(Filename);

#ifdef vax
	while( (dirent = readdir(Dfd)) != NULL) {
		if(dirent->d_ino == 0)		/* File has been deleted */
			continue;
		if(strncmp(dirent->d_name,".",1) == 0)		/* Skip "." */
			continue;
		if(strncmp(dirent->d_name,"..",2) == 0)		/* Skip ".." */
			continue;
		strncpy(&Filename[pathlength],dirent->d_name,sizeof(dirent->d_name));
		strcpy(Loginname,dirent->d_name);
		return(SUCCEED);
	}
	closedir(Dfd);
	return(FAILED);
#else
	while(read(Dfd,&Dir_entry,sizeof(Dir_entry)) > 0) {
		if(Dir_entry.d_ino == 0)	/* File has been deleted */
			continue;
		if(strncmp(Dir_entry.d_name,".",1) == 0)	/* Skip "." */
			continue;
		if(strncmp(Dir_entry.d_name,"..",2) == 0)	/* Skip ".." */
			continue;
		strncpy(&Filename[pathlength],Dir_entry.d_name,sizeof(Dir_entry.d_name));
		strcpy(Loginname,Dir_entry.d_name);
		return(SUCCEED);
	}
	if( close(Dfd) == FAILED)
		fprintf(stderr,"Couldn't close %s\n",Directory);

	return(FAILED);				/* Have read entire dir. */
#endif
}	
/*==================================================
 *
 * del_oldbig() - remove letters that are either too old or too big.
 *		  remove mailboxes that are zero length.
 *		exit -    0 if no problems encountered
 *			  1 if couldn't open or other nastiness
 *
 *==================================================
 */
del_oldbig()
{
	long	finalsize;
	long	size_date();
	long	filemark;		/* Marks beginning of current letter */
	int	onetrip;		/* Whether or not we've made first   */
					/* trip through mail scanning loop   */
	int	copy;			/* Temporary loop index for saving   */
					/* mail headers 		     */
	int	wrote_sndl;		/* Whether or not we've written our  */
					/* "From" line to temp file	     */

	if( stat(Filename,&Sbuf) == FAILED) {
		fprintf(stderr,"Can't stat %s\n",Filename); 
		return(FAILED);
	}

	/* Reset delete counter in case we exit before setting it below */

	bytes_del = 0l;

	/* If file is zero length, mark it for deletion */

	if(Sbuf.st_size == 0) {
		++files_del;
		if(rewrite_flag)
			unlink(Filename);
		if(audit_flag && rewrite_flag)
			printf("\t%s was zero-length, deleted\n",Filename);
		if(audit_flag && !rewrite_flag)
			printf("\t%s is zero-length\n",Filename);
		return(SUCCEED);
	}

	/* Determine what the final size for this file should be */

	if( (finalsize = size_date()) == FAILED_LONG) {
		fprintf(stderr,"Error determining date/size of %s\n",Filename);
		return(FAILED);
	}
		
	if(Sbuf.st_size <= finalsize)		/* File is smaller than it */
		return(SUCCEED);		/* has to be, no problem.  */

	++files_short;				/* Another one shortened */

	File_length = Sbuf.st_size;
	filemark = File_length;

	onetrip = FAILED;
	wrote_sndl = FAILED;

	if( (Mfp = fopen(Filename,"r+")) == NULL) {
		fprintf(stderr,"Can't open %s\n",Filename);
		return(FAILED);
	}
	if(rewrite_flag) {
		if( (Tfp = fopen(Tempfile,"w+")) == NULL) {
			fprintf(stderr,"Can't open tempfile %s\n",Tempfile);
			return(FAILED);
		}
	}

	while( readline() == SUCCEED && File_length > finalsize) {
		if(is_a_header() == SUCCEED) {
			if(rewrite_flag && (from_system() == FAILED) ) {
					if(wrote_sndl == FAILED) {
	fprintf(Tfp,"%s",sndl);
	if( rindex(Filename,'/') == NULL) {
		fprintf(Tfp,"%s\nSubject: Mail deleted by MAILER-DAEMON\n\n",Filename);
	}
	else {
		fprintf(Tfp,"%s\nSubject: Mail deleted by MAILER-DAEMON\n\n",rindex(Filename,'/') + (char *) (1));
	}
						wrote_sndl = SUCCEED;
					}
				fprintf(Tfp,"  Letter deleted: ");
				writeline();
			}
			if(audit_flag && (onetrip == SUCCEED) ) {
				printf("%s: ",Loginname);
				printf("%8ld bytes: ",filemark-File_length);
				filemark = File_length;
				printhead();
			}
			if (audit_flag) {
				for(copy=0 ; copy < LINE_LEN ; copy++) {
					Headbuf[copy]= Buffer[copy];
				}
			}
			onetrip = SUCCEED;
		}
	}

	/* Eat up rest of last message deleted; wait for next letter header */

	while(readline() == SUCCEED && is_a_header() == FAILED);

	bytes_del = Sbuf.st_size - File_length;
	tot_bytes_del += bytes_del;

	/* Write out this new header, but be careful to check that */
	/* it definitely is one. (I.e., one-line loop above may have */
	/* exited due to EOF and not a "From" line.		*/

	/* Also make sure to add a newline at the end of our message */
	/* so as not to confuse any mailers.			*/

	if(rewrite_flag && is_a_header() == SUCCEED ) {
		if(finalsize == 0)
			fprintf(Tfp,"\n  Reason: system mailbox older than %d days\n\n",max_age);
		else if (finalsize == MAX_SIZE)
			fprintf(Tfp,"\n  Reason: system mailbox older than %d days and larger than %ld bytes\n\n",min_age,MAX_SIZE);
		else if (finalsize == ABS_MAX_SIZE)
			fprintf(Tfp,"\n  Reason: system mailbox larger than %ld bytes\n\n",ABS_MAX_SIZE);
		else 
			fprintf(Tfp,"\n  Reason: unknown\n\n");
		writeline();
	}
	if(audit_flag) {
		printf("%s: ",Loginname);
		printf("%8ld bytes: ",filemark-File_length);
		filemark = File_length;
		printhead();
		for(copy=0 ; copy < LINE_LEN ; copy++) {
			Headbuf[copy]= Buffer[copy];
		}
	}

	while(readline() == SUCCEED) {
		if(rewrite_flag)
			writeline();
	}

	if(cleanup() == FAILED)
		return(FAILED);
	return(SUCCEED);
}
/*==================================================
 *
 * from_system() - returns true if this letter was
 * 		from a previous invocation of this
 *		program; causes mail files to decay
 *		to zero length eventually.
 *
 *==================================================
 */
from_system()
{
	if( strncmp(&Buffer[5],&sndl[5],strlen(SENDER) ) == 0)
		return(SUCCEED);
	else
		return(FAILED);
}

/*==================================================
 *
 *
 * del_user() - remove letters that are from user "Selectname" 
 *		exit -    0 if no problems encountered
 *			  1 if couldn't open or other nastiness
 *
 *
 *==================================================
 */
del_user()
{
	int	file_trunc;		/* Set if this file truncated */

	long	mark_beg;		/* marks beginning of current letter */
	long	mark_end;		/* marks end of current letter */
	int	copy;			/* Temporary loop index for saving */
					/* mail headers */
	file_trunc = 0;			/* Zero out audit counters */
	bytes_del = 0;

	if( stat(Filename,&Sbuf) == FAILED) {
		fprintf(stderr,"Can't stat %s\n",Filename); 
		return(FAILED);
	}

	/* If file is zero length, don't bother with it.	*/
	if(Sbuf.st_size == 0)
		return(SUCCEED);

	if( (Mfp = fopen(Filename,"r+")) == NULL) {
		fprintf(stderr,"Can't open %s\n",Filename);
		return(FAILED);
	}
	if( (Tfp = fopen(Tempfile,"w+")) == NULL) {
		fprintf(stderr,"Can't open tempfile %s\n",Tempfile);
		return(FAILED);
	}

	File_length = Sbuf.st_size;
	mark_beg = File_length;
	mark_end = File_length;
	Copy_flag = 1;

	/* The idea of mark_beg and mark_end is that mark_end continually */
	/* keeps track of the end of the previous line, and that mark_beg */
	/* keeps track of the end of the last line of the last letter, so */
	/* the distance between reflects the number of bytes deep we are */
	/* into the current letter.  When we finish with a letter, we know */
	/* how long it was. */

	while(readline() == SUCCEED) {
		if(is_a_header() == SUCCEED) {
			if(audit_flag && !Copy_flag) {
				printf("%s: ", Loginname);
				printf("%8ld bytes: ", mark_beg - mark_end);
				printhead();
				bytes_del += (mark_beg - mark_end);
				mark_beg = mark_end;
			}
			if(audit_flag && Copy_flag)
				mark_beg = mark_end;
			if(is_from_user() == SUCCEED) {
				Copy_flag = 0;
				file_trunc = 1;
				if(audit_flag)
					for(copy = 0; copy < LINE_LEN; copy++)
						Headbuf[copy] = Buffer[copy];
			}
			else
				Copy_flag = 1;
		}
		if(Copy_flag)
			writeline();
		mark_end = File_length;
	}

	if( !Copy_flag) {			/* Last letter was hit */
		if(audit_flag) {
			printf("%s: ", Loginname);
			printf("%8ld bytes: ", mark_beg - mark_end);
			printhead();
			bytes_del += (mark_beg - mark_end);
		}
	}

	if(file_trunc == 1)
		++files_short;

	tot_bytes_del += bytes_del;

	if(cleanup() == FAILED)
		return(FAILED);
	return(SUCCEED);
}
/*
*******************************************************************
*	readline() - read a line from input mail file
*		returns 0 if line successfully read
*			-1 if out of characters to read
*******************************************************************
*/
readline()
{
	int	how_many_read = 0;
	char	*bufptr	= Buffer;
	char	c;

	if(File_length == 0)
		return(FAILED);
	do{
		c = getc(Mfp);
		*bufptr++ = c;
		++how_many_read;
	}
	while( (c != NEWLINE)
		&& (how_many_read < LINE_LEN)
		&& (how_many_read < File_length) );

	File_length -= how_many_read;
	return(SUCCEED);
}
/*
*******************************************************************
*	writeline() - Write out a line to the temp file.
*		returns 0 ALWAYS
*******************************************************************
*/
writeline()
{
	int	i;
	char	*bufptr	= Buffer;
	char 	c;

	for( i = 0; i < LINE_LEN; i++) {
		c = *bufptr++;
		putc(c,Tfp);
		if(c == NEWLINE)
			return(SUCCEED);
	}
	return(SUCCEED);
}
/*
*******************************************************************
*	printhead() - Write out a line to stdout.
*		returns 0 ALWAYS
*******************************************************************
*/
printhead()
{
	int	i;
	char	*bufptr	= Headbuf;
	char 	c;

	for( i = 0; i < LINE_LEN; i++) {
		c = *bufptr++;
		putchar(c);
		if(c == NEWLINE)
			return(SUCCEED);
	}
	return(SUCCEED);
}
/*
*******************************************************************
*	is_a_header() - Is this line a mail letter header?
*			(i.e. Does it begin with "From "?
*		returns 0 if it is
*			-1 if not
*******************************************************************
*/
is_a_header()
{
	if( Buffer[0] != 'F' )
		return(FAILED);
	if( Buffer[1] == 'r'
	  && Buffer[2] == 'o'
	  && Buffer[3] == 'm'
	  && Buffer[4] == ' ')		/* If succeeds, must be header */
		return(SUCCEED);
	else
		return(FAILED);
}
/*
*******************************************************************
*	is_from_user() - Is this file from user "Selectname"?
*		returns 0 if it is
*			-1 if not
*******************************************************************
*/
is_from_user()
{
	int	j;
	for(j=0;j<strlen(Selectname);j++)
		if(Buffer[j+5] != Selectname[j])
			return(FAILED);
	return(SUCCEED);
}
/*
*******************************************************************
*	size_date() - Look at modify time and size of this file
*			to determine to what length it should be cut.
*		returns 0 if everything goes ok
*			-1 if time didn't make sense.
*******************************************************************
*/
long
size_date()
{
	long	finalsize;
	long	days_elapsed;
	char	*filetime;

	filetime = ctime(&Sbuf.st_mtime);

	days_elapsed = dysince(filetime);

	if(days_elapsed > max_age)
		finalsize = 0;
	if(days_elapsed <= max_age && days_elapsed > min_age)
		finalsize = MAX_SIZE;
	if(days_elapsed <= min_age && days_elapsed >= 0l)
		finalsize = ABS_MAX_SIZE;
	if(days_elapsed < 0l)
		return(FAILED_LONG);
	return(finalsize);
}
/*
*******************************************************************
*	cleanup() - Copy results back to original, blast temp file.
*		returns 0 if everything goes ok
*			-1 if closes/(un)links/chown/chmod fail
*******************************************************************
*/
cleanup()
{
	int	c;			/* Character for copying	*/

	if( !rewrite_flag) {		/* If not writing, then stop here */
		if( fclose(Mfp) != SUCCEED) {
			fprintf(stderr,"Couldn't close %s\n",Filename);
			return(FAILED);
		}
		return(SUCCEED);
	}

	if( fflush(Mfp) != SUCCEED) {
		fprintf(stderr,"Couldn't fflush %s\n",Filename);
		return(FAILED);
	}
	if( fflush(Tfp) != SUCCEED) {
		fprintf(stderr,"Couldn't fflush tempfile %s\n",Tempfile);
		return(FAILED);
	}
	if( (Mfp=freopen(Filename,"w",Mfp)) == NULL) {
		fprintf(stderr,"Couldn't freopen %s\n",Filename);
		return(FAILED);
	}
	if( rewind(Tfp) != SUCCEED) {
		fprintf(stderr,"Couldn't rewind tempfile %s\n",Tempfile);
		return(FAILED);
	}

	while( (c=getc(Tfp)) != EOF)
		putc(c,Mfp);

	if( fclose(Mfp) != SUCCEED) {
		fprintf(stderr,"Couldn't close %s\n",Filename);
		return(FAILED);
	}
	if( fclose(Tfp) != SUCCEED) {
		fprintf(stderr,"Couldn't close tempfile %s\n",Tempfile);
		return(FAILED);
	}
	if( unlink(Tempfile) == FAILED) {
		fprintf(stderr,"Couldn't unlink tempfile %s\n",Tempfile);
		return(FAILED);
	}
	return(SUCCEED);
}
