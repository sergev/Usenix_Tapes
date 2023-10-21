/**		initialize.c		**/

/***** Initialize - read in all the defaults etc etc 
       (C) Copyright 1985 Dave Taylor
*****/

#include "headers.h"

#ifdef BSD
#  include <sgtty.h>
#else
#  include <termio.h>
#endif

#include <pwd.h>

#ifdef BSD
#  include <sys/time.h>
#else
#  include <time.h>
#endif

#include <signal.h>
#include <ctype.h>
#include <errno.h>

#ifdef BSD
#undef toupper
#undef tolower
#endif

extern int errno;		/* system error number on failure */

char *error_name(), *error_description();

char *expand_logname(), *getenv(), *getlogin(), *strcpy(), *strcat();
unsigned short getgid(), getuid(); 
void exit();

struct   header_rec *malloc();

initialize(initscreen_too)
int initscreen_too;
{
	/** initialize the whole ball of wax.   If "initscreen_too" then
	    call init_screen where appropriate..
	**/
	struct passwd *pass, *getpwnam();

	register int i, j; 
	int      quit_signal(), term_signal(), ill_signal(),
		 fpe_signal(),  bus_signal(),  segv_signal(),
	         alarm_signal(), pipe_signal();
	char     buffer[SLEN], *cp;
	

	userid  = getuid();
	groupid = getgid();	

	strcpy(home,((cp = getenv("HOME")) == NULL)? "" : cp);
	strcpy(shell,((cp = getenv("SHELL")) == NULL)? "" : cp);
	strcpy(pager,((cp = getenv("PAGER")) == NULL)? default_pager : cp);

	if (debug) {		/* setup for dprintf statements! */
	  char newfname[SLEN], filename[SLEN];

	  sprintf(filename, "%s/%s", home, DEBUG);
	  if (access(filename, ACCESS_EXISTS) == 0) {	/* already one! */
	    sprintf(newfname,"%s/%s", home, OLDEBUG);
	    (void) link(filename, newfname);
	  }

	  /* Note what we just did up there: we always save the old
	     version of the debug file as OLDEBUG, so users can mail
	     copies of bug files without trashing 'em by starting up
	     the mailer.  Dumb, subtle, but easy enough to do!
 	  */

	  if ((debugfile = fopen(filename, "w")) == NULL) {
	    debug = 0;	/* otherwise 'leave' will try to log! */
	    leave(fprintf(stderr,"Could not open file %s for debug output!\n",
		  filename));
	  }
	  chown(filename, userid, groupid); /* file owned by user */

	  fprintf(debugfile, "Debug output of the ELM program.  Version %s\n\n",
		  VERSION);
	}

	if (initscreen_too)	/* don't set up unless we need to! */
	  InitScreen();

	if (debug < 2) {	/* otherwise let the system trap 'em! */
	  signal(SIGINT,  SIG_IGN);
	  signal(SIGQUIT, quit_signal);		/* Quit signal 	            */
	  signal(SIGTERM, term_signal); 	/* Terminate signal         */
	  signal(SIGILL,  ill_signal);		/* Illegal instruction      */
	  signal(SIGFPE,  fpe_signal);		/* Floating point exception */
	  signal(SIGBUS,  bus_signal);		/* Bus error  		    */
	  signal(SIGSEGV, segv_signal);		/* Segmentation Violation   */
	}
	
	signal(SIGALRM, alarm_signal);		/* Process Timer Alarm	    */
	signal(SIGPIPE, pipe_signal);		/* Illegal Pipe Operation   */

	get_term_chars();
	
	gethostname(hostname, sizeof(hostname));

#ifdef BSD
	if ((cp = getenv("USER")) == NULL)
#else
	if ((cp = getenv("LOGNAME")) == NULL)
#endif
	  if ((cp = getlogin()) == NULL)
	    cuserid(username);
	  else
	    strcpy(username, cp);
	else
	  strcpy(username, cp);

	/* now let's get the full username.. */

	if ((pass = getpwnam(username)) == NULL) {
	  error("Couldn't read password entry??");
	  strcpy(full_username, username);
	}
	else {
	  /* fix for this section from Don Joslyn of Nova University */
	  for (i=0,j=0; pass->pw_gecos[i] != '\0' && pass->pw_gecos[i] != ',';
	       i++)
	      if (pass->pw_gecos[i] == '&') {
	        full_username[j] = '\0';
	        strcat(full_username, expand_logname());
	        j = strlen(full_username);
	      }
	      else
	        full_username[j++] = pass->pw_gecos[i];
	  full_username[j] = '\0'; 
	}

	if ((cp = getenv("EDITOR")) == NULL)
	  strcpy(editor,default_editor);
	else
	  strcpy(editor, cp);
	strcpy(alternative_editor, editor);	/* this one can't be changed! */

	if (! mail_only) {
	  mailbox[0] = '\0';
	  strcpy(prefixchars, "> "); 	/* default message prefix */
	  sprintf(calendar_file, "%s/%s", home, dflt_calendar_file);
	}

	local_signature[0] = remote_signature[0] = '\0';	/* NULL! */

	read_rc_file();		/* reading the .elmrc next... */

	/** now try to expand the specified filename... **/

	if (strlen(infile) > 0) {
	  (void) expand_filename(infile);
	  if ((errno = can_access(infile, READ_ACCESS))) {
	    dprint2(1,"Error: given file %s as mailbox - unreadable (%s)!\n", 
		     infile, error_name(errno));
	    fprintf(stderr,"Can't open mailbox '%s' for reading!\n", infile);
	    exit(1);
	  }
	}

	/** check to see if the user has defined a LINES or COLUMNS
	    value different to that in the termcap entry (for
	    windowing systems, of course!) **/

	ScreenSize(&LINES, &COLUMNS);

	if ((cp = getenv("LINES")) != NULL && isdigit(*cp)) {
	  sscanf(cp, "%d", &LINES);
	  LINES -= 1;	/* kludge for HP Window system? ... */
	}

	if ((cp = getenv("COLUMNS")) != NULL && isdigit(*cp))
	  sscanf(cp, "%d", &COLUMNS);

	/** fix the shell if needed **/

	if (shell[0] != '/') {
	   sprintf(buffer, "/bin/%s", shell);
	   strcpy(shell, buffer);
	}

	if (! mail_only) {
	  mailbox_defined = (mailbox[0] != '\0'); 

	  /* get the cursor control keys... */

	  cursor_control = FALSE;

	  if ((cp = return_value_of("ku")) != NULL)
	   if (strlen(cp) == 2) {
	    strcpy(up, cp);
	    if ((cp = return_value_of("kd")) == NULL)
	      cursor_control = FALSE;
	    else if (strlen(cp) != 2)
	      cursor_control = FALSE;
	    else {
	      strcpy(down, cp);
	      cursor_control = TRUE;
	      transmit_functions(ON);
	    }
	  }

	  strcpy(start_highlight, "->");
	  end_highlight[0] = '\0';

	  if (!arrow_cursor) {	/* try to use inverse bar instead */
	    if ((cp = return_value_of("so")) != NULL) {
	      strcpy(start_highlight, cp);
	      if ((cp = return_value_of("se")) == NULL)
	        strcpy(start_highlight, "->");
	      else {
	        strcpy(end_highlight, cp);
	        has_highlighting = TRUE;
	      }
	    }
	  }
	}

	/** allocate the first KLICK headers... **/

	if ((header_table = malloc(KLICK*sizeof(struct header_rec))) == NULL) {
	   fprintf(stderr,"\n\r\n\rCouldn't allocate initial headers!\n\r\n");
	   leave();
	}
	max_headers = KLICK;		/* we have those preallocated */

	/** now cruise along... **/

	if (! mail_only) {
	  if (mini_menu)
	    headers_per_page = LINES - 13;
	  else
	    headers_per_page = LINES -  8;	/* 5 more headers! */

	  newmbox(1,FALSE, TRUE);	/* read in the mailbox! */
	}

	dprint0(2,"\n-- end of initialization phase --\n");

	dprint3(2,"\thostname = %s\n\tusername = %s\n\tfull_username = \"%s\"\n",
	         hostname, username, full_username);

	dprint3(2,"\thome = %s\n\teditor = %s\n\tmailbox = %s\n",
		 home, editor, mailbox);

	dprint3(2,"\tinfile = %s\n\tfolder-dir = %s\n\tprintout = \"%s\"\n",
		 infile, folders, printout);
	
	dprint3(2,"\tsavefile = %s\n\tprefix = \"%s\"\n\tshell = %s\n",
		savefile, prefixchars, shell);
	
	if (signature)
	  dprint2(2,"\tlocal-signature = %s\n\tremote-signature = %s\n",
	  	  local_signature, remote_signature);

	dprint0(1,"-- beginning execution phase --\n\n");
}

get_term_chars()
{
	/** This routine sucks out the special terminal characters
	    ERASE and KILL for use in the input routine.  The meaning 
            of the characters are (dare I say it?) fairly obvious... **/

#ifdef BSD
	struct sgttyb term_buffer;

# define TCGETA	TIOCGETP

#else 
	struct termio term_buffer;
#endif

	if (ioctl(STANDARD_INPUT, TCGETA, &term_buffer) == -1) {
	  dprint1(1,"Error: %s encountered on ioctl call (get_term_chars)\n", 
		   error_name(errno));
	  /* set to defaults for terminal driver */
	  backspace = BACKSPACE;
	  kill_line = ctrl('U');
	}
	else {
#ifdef BSD
	  backspace = term_buffer.sg_erase;
	  kill_line = term_buffer.sg_kill;
#else
	  backspace = term_buffer.c_cc[VERASE];
	  kill_line = term_buffer.c_cc[VKILL];
#endif
	}
}

char *expand_logname()
{
	/** Return logname in a nice format (for expanding "&" in the
	    /etc/passwd file) **/

	static char buffer[SLEN];
	register int i;

	if (strlen(username) == 0)
	  buffer[0] = '\0';
	else {
	  buffer[0] = toupper(username[0]);

	  for (i=1; username[i] != '\0'; i++)
	    buffer[i] = tolower(username[i]);

	  buffer[i] = '\0';
	}

	return( (char *) buffer);	
}
