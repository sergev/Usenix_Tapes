/**			read_rc.c			**/

/** (C) Copyright 1985, Dave Taylor			**/

/** This file contains programs to allow the user to have a .elmrc file
    in their home directory containing any of the following: 

	fullname= <username string>
	maildir = <directory>
	mailbox = <file>
	editor  = <editor>
	savemail= <savefile>
	calendar= <calendar file name>
	shell   = <shell>
	print   = <print command>
	weedout = <list of headers to weed out>
	prefix  = <copied message prefix string>
	pager   = <command to use for displaying messages>

--
	signature = <.signature file for all outbound mail>
OR:
	localsignature = <.signature file for local mail>
	remotesignature = <.signature file for non-local mail>
--

	bounceback= <hop count threshold, or zero to disable>
	timeout = <seconds for main menu timeout or zero to disable>
	userlevel = <0=amateur, 1=okay, 2 or greater = expert!>

	sortby  = <sent, received, from, size, subject>

	alternatives = <list of addresses that forward to us>

    and/or the logical arguments:
	
	autocopy    [on|off]
	copy        [on|off]	
	resolve     [on|off]
	weed        [on|off]
	noheader    [on|off]
	titles      [on|off]
	editout     [on|off]
	savebyname  [on|off]
	movepage    [on|off]
	pointnew    [on|off]
	hpkeypad    [on|off]
	hpsoftkeys  [on|off]
	alwaysleave [on|off]
	alwaysdel   [on|off]
	arrow	    [on|off]
	menus	    [on|off]
	forms	    [on|off]
	warnings    [on|off]
	names	    [on|off]
	ask	    [on|off]
	keepempty   [on|off]


    Lines starting with '#' are considered comments and are not checked
    any further!

    Modified 10/85 to know about "Environment" variables..
    Modified 12/85 for the 'prefix' option
    Modified  2/86 for the new 3.3 flags
    Modified  8/86 (was I supposed to be keeping this up to date?)
**/

#include <stdio.h>
#include <ctype.h>

#ifdef BSD
#undef tolower
#endif

#include "headers.h"

char *shift_lower(), *strtok(), *getenv(), *strcpy();
void   exit();

#define NOTWEEDOUT	0
#define WEEDOUT		1
#define ALTERNATIVES	2

read_rc_file()
{
	/** this routine does all the actual work of reading in the
	    .rc file... **/

	FILE *file;
	char buffer[SLEN], filename[SLEN];
	char word1[SLEN], word2[SLEN];
	register int  i, errors = 0, last = NOTWEEDOUT, lineno = 0;

	sprintf(filename,"%s/%s", home, elmrcfile);

	default_weedlist();

	alternative_addresses = NULL; 	/* none yet! */

	if ((file = fopen(filename, "r")) == NULL) {
	  dprint0(2,"Warning: User has no \".elmrc\" file (read_rc_file)\n");
	  return;	/* we're done! */
	}

	while (fgets(buffer, SLEN, file) != NULL) {
	  lineno++;
	  no_ret(buffer);	 	/* remove return */
	  if (buffer[0] == '#') { 	/* comment       */
	    last = NOTWEEDOUT;
	    continue;
	  }
	  if (strlen(buffer) < 2) {	/* empty line    */
	    last = NOTWEEDOUT;
	    continue;
	  }

	  breakup(buffer, word1, word2);

	  strcpy(word1, shift_lower(word1));	/* to lower case */

	  if (word2[0] == 0 && (last != WEEDOUT || last != ALTERNATIVES)) {
	    dprint1(2,"Error: Bad .elmrc entry in users file;\n-> \"%s\"\n",
		     buffer);
	    fprintf(stderr, 
		    "Line %d of your \".elmrc\" is badly formed:\n> %s\n",
		    lineno, buffer);
	    errors++;
	    continue;
	  }
	
	  if (equal(word1,"maildir") || equal(word1,"folders")) {
	    expand_env(folders, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "fullname") || equal(word1,"username") ||
		   equal(word1, "name")) {
	    strcpy(full_username, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "prefix")) {
	    for (i=0; i < strlen(word2); i++)
	      prefixchars[i] = (word2[i] == '_' ? ' ' : word2[i]);
	    prefixchars[i] = '\0';
	   
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "shell")) {
	    expand_env(shell, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "sort") || equal(word1, "sortby")) {
	    strcpy(word2, shift_lower(word2));
	    if (equal(word2, "sent"))
	       sortby = SENT_DATE;
	    else if (equal(word2, "received") || equal(word2,"recieved"))
	       sortby = RECEIVED_DATE;
	    else if (equal(word2, "from") || equal(word2, "sender"))
	       sortby = SENDER;
	    else if (equal(word2, "size") || equal(word2, "lines"))
	      sortby = SIZE;
	    else if (equal(word2, "subject"))
	      sortby = SUBJECT;
	    else if (equal(word2, "reverse-sent") || equal(word2,"rev-sent"))
	       sortby = - SENT_DATE;
	    else if (strncmp(word2, "reverse-rec",11) == 0 || 
		     strncmp(word2,"rev-rec",7) == 0)
	       sortby = - RECEIVED_DATE;
	    else if (equal(word2, "reverse-from") || equal(word2, "rev-from")
	          || equal(word2,"reverse-sender")|| equal(word2,"rev-sender"))
	       sortby = - SENDER;
	    else if (equal(word2, "reverse-size") || equal(word2, "rev-size")
	          || equal(word2, "reverse-lines")|| equal(word2,"rev-lines"))
	      sortby = - SIZE;
	    else if (equal(word2, "reverse-subject") || 
		     equal(word2, "rev-subject"))
	      sortby = - SUBJECT;
	    else {
	     if (! errors)
	       printf("Error reading '.elmrc' file;\n");
	     printf("Line %d: Don't know what sort key '%s' specifies!\n", 
		    lineno, word2);
	     errors++;
	     continue;
	   }
	  }
	  else if (equal(word1, "mailbox")) {
	    expand_env(mailbox, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "editor") || equal(word1,"mailedit")) {
	    expand_env(editor, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "savemail") || equal(word1, "saveto")) {
	    expand_env(savefile, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "calendar")) {
	    expand_env(calendar_file, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "print") || equal(word1, "printmail")) {
	    expand_env(printout, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "pager") || equal(word1, "page")) {
	    expand_env(pager, word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "signature")) {
	    if (equal(shift_lower(word2), "on") ||
		equal(shift_lower(word2), "off")) {
	      fprintf(stderr, 
	  "\"signature\" used in obsolete way in .elmrc file - ignored!\n\r");
	      fprintf(stderr,
   "\t(signature should specify the filename to use rather than on/off)\n\r");
	    }
	    else {
	      expand_env(local_signature, word2);
	      strcpy(remote_signature, local_signature);
	      signature = TRUE;
	    }
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "localsignature")) {
	    expand_env(local_signature, word2);
	    signature = TRUE;
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "remotesignature")) {
	    expand_env(remote_signature, word2);
	    signature = TRUE;
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "autocopy")) {
	    auto_copy = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "copy") || equal(word1, "auto_cc")) {
	    auto_cc = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "names")) {
	    names_only = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "resolve")) {
	    resolve_mode = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "weed")) {
	    filter = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "noheader")) {
	    noheader = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "titles")) {
	    title_messages = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "editout")) {
	    edit_outbound = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "savebyname") || equal(word1, "savename")) {
	    save_by_name = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "movepage") || equal(word1, "page") ||
		   equal(word1, "movewhenpaged")) {
	    move_when_paged = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "pointnew") || equal(word1, "pointtonew")) {
	    point_to_new = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "keypad") || equal(word1, "hpkeypad")) {
	    hp_terminal = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "softkeys") || equal(word1, "hpsoftkeys")) {
	    if (hp_softkeys = is_it_on(word2))
	      hp_terminal = TRUE;	/* must be set also! */
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "arrow")) {
	    arrow_cursor = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (strncmp(word1, "form", 4) == 0) {
	    allow_forms = (is_it_on(word2)? MAYBE : NO);
	    last = NOTWEEDOUT;
	  }
	  else if (strncmp(word1, "menu", 4) == 0) {
	    mini_menu = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (strncmp(word1, "warning", 7) == 0) {
	    warnings = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "alwaysleave") || equal(word1, "leave")) {
	    always_leave = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "alwaysdelete") || equal(word1, "delete")) {
	    always_del = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "ask") || equal(word1, "question")) {
	    question_me = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "keep") || equal(word1, "keepempty")) {
	    keep_empty_files = is_it_on(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "bounce") || equal(word1, "bounceback")) {
	    bounceback = atoi(word2);
	    if (bounceback > MAX_HOPS) {
	      fprintf(stderr,
	"Warning: bounceback is set to greater than %d (max-hops) - Ignored.\n",
		       MAX_HOPS);
	      bounceback = 0;
	    }
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "userlevel")) {
	    user_level = atoi(word2);
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "timeout")) {
	    timeout = atoi(word2);
	    if (timeout < 10) {
	      fprintf(stderr,
	   "Warning: timeout is set to less than 10 seconds - Ignored.\n");
	      timeout = 0;
	    }
	    last = NOTWEEDOUT;
	  }
	  else if (equal(word1, "weedout")) {
	    weedout(word2);
	    last = WEEDOUT;
	  }
	  else if (equal(word1, "alternatives")) {
	    alternatives(word2);
	    last = ALTERNATIVES;
	  }
	  else if (last == WEEDOUT)	/* could be multiple line weedout */
	    weedout(buffer);
	  else if (last == ALTERNATIVES)	/* multi-line addresses   */
	    alternatives(buffer);
	  else {
	    fprintf(stderr, 
		   "Line %d is undefined in your \".elmrc\" file:\n> %s\n", 
		   lineno, buffer);
	    errors++;
	  }
	}

	if (debug > 10) 	/** only do this if we REALLY want debug! **/
	  dump_rc_results();

	if (errors) 
	  exit(errors);
}
	
weedout(string)
char *string;
{
	/** This routine is called with a list of headers to weed out.   **/

	char *strptr, *header;
	register int i;

	strptr = string;

	while ((header = strtok(strptr, "\t ,\"'")) != NULL) {
	  if (strlen(header) > 0) {
	    if (weedcount > MAX_IN_WEEDLIST) {
	      fprintf(stderr, "Too many weed headers!  Leaving...\n");
	      exit(1);
	    }
	    if ((weedlist[weedcount] = (char *) pmalloc(strlen(header) + 1)) 
		== NULL) {
	      fprintf(stderr,
		      "Too many weed headers - out of memory!  Leaving...\n");
	      exit(1);
	    }

	    for (i=0; i< strlen(header); i++)
	      if (header[i] == '_') header[i] = ' ';

	    strcpy(weedlist[weedcount], header);
	    weedcount++;
	  }
	  strptr = NULL;
	}
}

alternatives(string)
char *string;
{
	/** This routine is called with a list of alternative addresses
	    that you may receive mail from (forwarded) **/

	char *strptr, *address;
	struct addr_rec *current_record, *previous_record;

	previous_record = alternative_addresses;	/* start 'er up! */
	/* move to the END of the alternative addresses list */

	if (previous_record != NULL)
	  while (previous_record->next != NULL)
	    previous_record = previous_record->next;

	strptr = (char *) string;

	while ((address = strtok(strptr, "\t ,\"'")) != NULL) {
	  if (previous_record == NULL) {
	    previous_record = (struct addr_rec *) pmalloc(sizeof 
		*alternative_addresses);

	    strcpy(previous_record->address, address);
	    previous_record->next = NULL;
	    alternative_addresses = previous_record;
	  }
	  else {
	    current_record = (struct addr_rec *) pmalloc(sizeof 
		*alternative_addresses);
	  
	    strcpy(current_record->address, address);
	    current_record->next = NULL;
	    previous_record->next = current_record;
	    previous_record = current_record;
	  }
	  strptr = (char *) NULL;
	}
}

default_weedlist()
{
	/** Install the default headers to weed out!  Many gracious 
	    thanks to John Lebovitz for this dynamic method of 
	    allocation!
	**/

	static char *default_list[] = { ">From", "In-Reply-To:",
		       "References:", "Newsgroups:", "Received:",
		       "Apparently-To:", "Message-Id:", "Content-Type:",
		       "From", "X-Mailer:", "*end-of-defaults*", NULL
		     };

	for (weedcount = 0; default_list[weedcount] != NULL; weedcount++) {
	  if ((weedlist[weedcount] = (char *)
	      pmalloc(strlen(default_list[weedcount]) + 1)) == NULL) {
	    fprintf(stderr, 
	       "\n\rNot enough memory for default weedlist.  Leaving.\n\r");
	    leave(1);
	  }
	  strcpy(weedlist[weedcount], default_list[weedcount]);
	}
}

int
matches_weedlist(buffer)
char *buffer;
{
	/** returns true iff the first 'n' characters of 'buffer' 
	    match an entry of the weedlist **/
	
	register int i;

	for (i=0;i < weedcount; i++)
	  if (strncmp(buffer, weedlist[i], strlen(weedlist[i])) == 0) 
	    return(1);

	return(0);
}

breakup(buffer, word1, word2)
char *buffer, *word1, *word2;
{
	/** This routine breaks buffer down into word1, word2 where 
	    word1 is alpha characters only, and there is an equal
	    sign delimiting the two...
		alpha = beta
	    For lines with more than one 'rhs', word2 is set to the
	    entire string...
	**/

	register int i;
	
	for (i=0;i<strlen(buffer) && isalpha(buffer[i]); i++)
	  word1[i] = buffer[i];

	word1[i++] = '\0';	/* that's the first word! */

	/** spaces before equal sign? **/

	while (buffer[i] == ' ' || buffer[i] == '\t') i++;

	if (buffer[i] == '=') i++;

	/** spaces after equal sign? **/

	while (buffer[i] == ' ' || buffer[i] == '\t') i++;

	if (i < strlen(buffer))
	  strcpy(word2, (char *) (buffer + i));
	else
	  word2[0] = '\0';
}

expand_env(dest, buffer)
char *dest, *buffer;
{
	/** expand possible metacharacters in buffer and then copy
	    to dest... 
	    This routine knows about "~" being the home directory,
	    and "$xxx" being an environment variable.
	**/

	char  *word, *string, next_word[SLEN];
	
	if (buffer[0] == '/') {
	  dest[0] = '/';
	  dest[1] = '\0';
	}
	else
	  dest[0] = '\0';

	string = (char *) buffer;

	while ((word = strtok(string, "/")) != NULL) {
	  if (word[0] == '$') {
	    next_word[0] = '\0';
	    if (getenv((char *) (word + 1)) != NULL)
	    strcpy(next_word, getenv((char *) (word + 1)));
	    if (strlen(next_word) == 0)
	      leave(fprintf(stderr, 
		    "\n\rCan't expand environment variable '%s'\n\r",
		    word));
	  }
	  else if (word[0] == '~' && word[1] == '\0')
	    strcpy(next_word, home);
	  else
	    strcpy(next_word, word);

	  sprintf(dest, "%s%s%s", dest, 
		 (strlen(dest) > 0 && lastch(dest) != '/' ? "/":""),
		 next_word);

	  string = (char *) NULL;
	}
}

#define on_off(s)	(s == 1? "ON" : "OFF")
dump_rc_results()
{

	register int i;

	fprintf(debugfile, "\n\n\n\n");
	fprintf(debugfile, "folders = %s\n", folders);
	fprintf(debugfile, "mailbox = %s\n", mailbox);
	fprintf(debugfile, "editor = %s\n", editor);
	fprintf(debugfile, "printout = %s\n", printout);
	fprintf(debugfile, "savefile = %s\n", savefile);
	fprintf(debugfile, "calendar_file = %s\n", calendar_file);
	fprintf(debugfile, "prefixchars = %s\n", prefixchars);
	fprintf(debugfile, "shell = %s\n", shell);
	fprintf(debugfile, "pager = %s\n", pager);
	fprintf(debugfile, "\n");
	fprintf(debugfile, "mini_menu = %s\n", on_off(mini_menu));
	fprintf(debugfile, "mbox_specified = %s\n", on_off(mbox_specified));
	fprintf(debugfile, "check_first = %s\n", on_off(check_first));
	fprintf(debugfile, "auto_copy = %s\n", on_off(auto_copy));
	fprintf(debugfile, "filter = %s\n", on_off(filter));
	fprintf(debugfile, "resolve_mode = %s\n", on_off(resolve_mode));
	fprintf(debugfile, "auto_cc = %s\n", on_off(auto_cc));
	fprintf(debugfile, "noheader = %s\n", on_off(noheader));
	fprintf(debugfile, "title_messages = %s\n", on_off(title_messages));
	fprintf(debugfile, "hp_terminal = %s\n", on_off(hp_terminal));
	fprintf(debugfile, "hp_softkeys = %s\n", on_off(hp_softkeys));
	fprintf(debugfile, "save_by_name = %s\n", on_off(save_by_name));
	fprintf(debugfile, "move_when_paged = %s\n", on_off(move_when_paged));
	fprintf(debugfile, "point_to_new = %s\n", on_off(point_to_new));
	fprintf(debugfile, "bounceback = %s\n", on_off(bounceback));
	fprintf(debugfile, "signature = %s\n", on_off(signature));
	fprintf(debugfile, "always_leave = %s\n", on_off(always_leave));
	fprintf(debugfile, "always_del = %s\n", on_off(always_del));
	fprintf(debugfile, "arrow_cursor = %s\n", on_off(arrow_cursor));
	fprintf(debugfile, "names = %s\n", on_off(names_only));
	fprintf(debugfile, "warnings = %s\n", on_off(warnings));
	fprintf(debugfile, "question_me = %s\n", on_off(question_me));
	fprintf(debugfile, "keep_empty_files = %s\n", 
			   on_off(keep_empty_files));

	fprintf(debugfile, "\n** userlevel = %s **\n\n", user_level);

	fprintf(debugfile, "Weeding out the following headers;\n");
	for (i=0; i < weedcount; i++) 
	  fprintf(debugfile, "\t\"%s\"\n", weedlist[i]);
	
	fprintf(debugfile, "\n\n");
}

is_it_on(word)
char *word;
{
	/** Returns TRUE if the specified word is either 'ON', 'YES'
	    or 'TRUE', and FALSE otherwise.   We explicitly translate
	    to lowercase here to ensure that we have the fastest
	    routine possible - we really DON'T want to have this take
	    a long time or our startup will be major pain each time.
	**/

	static char mybuffer[NLEN];
	register int i, j;

	for (i=0, j=0; word[i] != '\0'; i++)
	  mybuffer[j++] = isupper(word[i]) ? tolower(word[i]) : word[i];
	mybuffer[j] = '\0';

	return(  (strncmp(mybuffer, "on",   2) == 0) ||
		 (strncmp(mybuffer, "yes",  3) == 0) ||
		 (strncmp(mybuffer, "true", 4) == 0)
	      );
}
	

	


