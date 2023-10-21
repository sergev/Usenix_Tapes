/**			fastmail.c			**/

/** This program is specifically written for group mailing lists and
    such batch type mail processing.  It does NOT use aliases at all,
    it does NOT read the /etc/password file to find the From: name
    of the user and does NOT expand any addresses.  It is meant 
    purely as a front-end for either /bin/mail or /usr/lib/sendmail
    (according to what is available on the current system).

         **** This program should be used with CAUTION *****

    (C) Copyright 1985 Dave Taylor
**/

/** The calling sequence for this program is:

	fastmail {args}  filename full-email-address 

   where args could be any (or all) of;

	   -b bcc-list		(Blind carbon copies to)
	   -c cc-list		(carbon copies to)
	   -d			(debug on)
	   -f from 		(from name)
	   -r reply-to-address 	(Reply-To:)
	   -s subject 		(subject of message)
**/

#include <stdio.h>

#ifdef BSD
# ifdef BSD4.1
#   include <time.h>
#   include <sys/types.h>
#   include <sys/timeb.h>
# else
#   include <sys/time.h>
# endif
#else
#  include <time.h>
#endif

#include "defs.h"

static char ident[] = { WHAT_STRING };

#define  binrmail	"/bin/rmail"
#define  temphome	"/tmp/fastmail."

#define DONE		0
#define ERROR		-1

char *optional_arg;			/* optional argument as we go */
int   opt_index;			/* argnum + 1 when we leave   */

char *arpa_dayname[] = { "Sun", "Mon", "Tue", "Wed", "Thu",
		  "Fri", "Sat", "" };

char *arpa_monname[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", ""};

char *get_arpa_date();

#ifdef BSD
  char *timezone();
#else
  extern char *tzname[];
#endif

main(argc, argv)
int argc;
char *argv[];
{

	FILE *tempfile;
	char hostname[NLEN], username[NLEN], from_string[SLEN], subject[SLEN];
	char filename[SLEN], tempfilename[SLEN], command_buffer[256];
	char replyto[SLEN], cc_list[SLEN], bcc_list[SLEN], to_list[SLEN];
	int  c, sendmail_available, debug = 0;

	replyto[0] = '\0';
	cc_list[0] = '\0';
	bcc_list[0] = '\0';

	while ((c = get_options(argc, argv, "b:c:df:r:s:")) > 0) {
	  switch (c) {
	    case 'b' : strcpy(bcc_list, optional_arg);		break;
	    case 'c' : strcpy(cc_list, optional_arg);		break;
	    case 'd' : debug++;					break;	
	    case 'f' : strcpy(from_string, optional_arg);	break;
	    case 'r' : strcpy(replyto, optional_arg);		break;
	    case 's' : strcpy(subject, optional_arg);		break;
 	  }
	}	

	if (c == ERROR) {
	  fprintf(stderr,"Usage: fastmail {args} filename address(es)\n");
	  fprintf(stderr, "   where {args} can be;\n");
	  fprintf(stderr,"\t-b bcc-list\n\t-c cc-list\n\t-d\n\t-f from-name\n");
	  fprintf(stderr, "\t-r reply-to\n\t-s subject\n\n");
	  exit(1);
	}

	if (opt_index > argc) {
	  fprintf(stderr,"Usage: fastmail {args} filename address(es)\n");
	  fprintf(stderr, "   where {args} can be;\n");
	  fprintf(stderr,"\t-b bcc-list\n\t-c cc-list\n\t-d\n\t-f from-name\n");
	  fprintf(stderr, "\t-r reply-to\n\t-s subject\n\n");
	  exit(1);
	}

	strcpy(filename, argv[opt_index++]);

	if (opt_index > argc) {
	  fprintf(stderr,"Usage: fastmail {args} filename address(es)\n");
	  fprintf(stderr, "   where {args} can be;\n");
	  fprintf(stderr,"\t-b bcc-list\n\t-c cc-list\n\t-d\n\t-f from-name\n");
	  fprintf(stderr, "\t-r reply-to\n\t-s subject\n\n");
	  exit(1);
	}

	gethostname(hostname, sizeof(hostname));
	strcpy(username, getlogin());
	if (strlen(username) == 0)
	  cuserid(username);

	if (access(filename, READ_ACCESS) == -1)
	  exit(fprintf(stderr, "Error: can't find file %s!\n", filename));

	sprintf(tempfilename, "%s%d", temphome, getpid());

	if ((tempfile = fopen(tempfilename, "w")) == NULL)
	  exit(fprintf(stderr, "Couldn't open temp file %s\n", tempfilename));

	if (strlen(from_string) > 0)
	  fprintf(tempfile, "From: %s!%s (%s)\n", 
		  hostname, username, from_string);
	else
	  fprintf(tempfile, "From: %s!%s\n", hostname, username);

	fprintf(tempfile, "Date: %s\n", get_arpa_date());

	if (strlen(subject) > 0)
	  fprintf(tempfile, "Subject: %s\n", subject);

	if (strlen(replyto) > 0)
	  fprintf(tempfile, "Reply-To: %s\n", replyto);

	while (opt_index < argc) 
          sprintf(to_list, "%s%s%s", to_list, (strlen(to_list) > 0? ", ":""), 
		  argv[opt_index++]);
	
	fprintf(tempfile, "To: %s\n", to_list);

	if (strlen(cc_list) > 0)
	  fprintf(tempfile, "Cc: %s\n", cc_list);

	if (strlen(bcc_list) > 0)
	  fprintf(tempfile, "Bcc: %s\n", bcc_list);	/* trust xport */

	fprintf(tempfile, "X-Mailer: fastmail [version %s]\n", VERSION);
	fprintf(tempfile, "\n");

	fclose(tempfile);

	/** now we'll cat both files to /bin/rmail or sendmail... **/

	sendmail_available = (access(sendmail, EXECUTE_ACCESS) != -1);

	printf("Mailing to %s%s%s%s%s [via %s]\n", to_list,
		(strlen(cc_list) > 0 ? " ":""), cc_list,
		(strlen(bcc_list) > 0 ? " ":""), bcc_list,
		sendmail_available? "sendmail" : "rmail");

	sprintf(command_buffer, "cat %s %s | %s '%s %s %s'", 
		tempfilename, filename, 
	        sendmail_available? sendmail : mailer,
		to_list, cc_list, bcc_list);

	if (debug)
	  printf("%s\n", command_buffer);

	system(command_buffer);

	unlink(tempfilename);
}


char *get_arpa_date()
{
	/** returns an ARPA standard date.  The format for the date
	    according to DARPA document RFC-822 is exemplified by;

	       	      Mon, 12 Aug 85 6:29:08 MST

	**/

	static char buffer[SLEN];	/* static character buffer       */
	struct tm *the_time,		/* Time structure, see CTIME(3C) */
		  *localtime();
	long	   junk;		/* time in seconds....		 */
#ifdef BSD
#  ifdef BSD4.1
	struct timeb loc_time;	/* of course this is different! */
#  else
	struct  timeval  time_val;		
	struct  timezone time_zone;
#  endif
#endif

#ifdef BSD
#  ifdef BSD4.1
	junk = (long) time((long *) 0);
	ftime(&loc_time);
#  else
	gettimeofday(&time_val, &time_zone);
	junk = time_val.tv_sec;
#  endif
#else
	junk = time(0);	/* this must be here for it to work! */
#endif
	the_time = localtime(&junk);

	sprintf(buffer, "%s, %d %s %d %d:%02d:%02d %s",
	  arpa_dayname[the_time->tm_wday],
	  the_time->tm_mday % 32,
	  arpa_monname[the_time->tm_mon],
	  the_time->tm_year % 100,
	  the_time->tm_hour % 24,
	  the_time->tm_min  % 61,
	  the_time->tm_sec  % 61,
#ifdef BSD
#  ifdef BSD4.1
	  timezone(loc_time.time_zone, the_time->tz_isdst));
#  else
	  timezone(time_zone.tz_minuteswest, time_zone.tz_dsttime));
#  endif
#else
	  tzname[the_time->tm_isdst]);
#endif
	
	return( (char *) buffer);
}

/** Starting argument parsing routine.   

    Called as "get_options(argc, argv, options)" where options is a string
    of the form "abc:d" indicating that 'a' 'b' and 'd' are flags and
    'c' is a flag with a trailing argument.   Optional arguments are
    returned in the external char * variable "optional_arg", and the
    external int "opt_index" is set to the first entry in the argv list
    that wasn't processed (ie after the flags). 

    For example, the C compiler would have something of the form 
    getopt(argc, argv, "Oo:l:") to allow "cc -O -o output -l lib file.c"

    (C) Copyright 1986, Dave Taylor
**/

int  _indx = 1, _argnum = 1;

int
get_options(argc, argv, options)
int argc;
char *argv[], *options;
{
	/** Returns the character argument next, and optionally instantiates 
	    "argument" to the argument associated with the particular option 
	**/
	
	char       *word, *strchr();

	if (_indx >= strlen(argv[_argnum])) {
	  _argnum++;
	  _indx = 1;		/* zeroeth char is '-' */
	}

	if (_argnum >= argc) {
	  opt_index = argc;
	  return(DONE);
	}
	
	if (argv[_argnum][0] != '-') {
	  opt_index = _argnum;
	  return(DONE);
	}

        word = strchr(options, argv[_argnum][_indx++]);

	if (strlen(word) == 0) 
	  return(ERROR);
	
	if (word[1] == ':') {

	  /** Two possibilities - either tailing end of this argument or the 
	      next argument in the list **/

	  if (_indx < strlen(argv[_argnum])) { /* first possibility */
	    optional_arg = (char *) (argv[_argnum] + _indx);
	    _argnum++;
	    _indx = 1;
	  }
	  else {				/* second choice     */
	    if (++_argnum >= argc) 
	      return(ERROR);			/* no argument!!     */

	    optional_arg = (char *) argv[_argnum++];
	    _indx = 1;
	  }
	}

	return((int) word[0]);
}
