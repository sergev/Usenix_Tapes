/**		file.c		**/

/** File I/O routines, mostly the save to file command...

    (C) Copyright 1986, Dave Taylor
**/

#include "headers.h"
#include <ctype.h>
#include <errno.h>

#ifdef BSD
#undef tolower
#endif

extern int errno;

char *error_name(), *error_description(), *strcpy(), *getenv();
unsigned long sleep();

int
save(redraw)
int *redraw;
{
	/** Save all tagged messages + current in a file.  If no messages
	    are tagged, save the current message instead!  This routine
	    will return ZERO if the operation failed.
	    'redraw' is set to TRUE iff we use the '?' and mess up
	    the screen.  Pretty reasonable, eh?
	**/

	register int tagged = 0, i, oldstat, appending = 0;
	register int iterations = 0, continue_looping;
	char filename[SLEN], address[LONG_SLEN], buffer[SLEN];
	FILE *save_file;

	oldstat = header_table[current-1].status;	/* remember */

	for (i=0; i < message_count; i++)
	  if (ison(header_table[i].status, TAGGED))
	    tagged++;

	if (tagged == 0) {
	  tagged = 1;
	  setit(header_table[current-1].status, TAGGED);
	}

	dprint2(4,"%d message%s tagged for saving (save)\n", tagged,
		plural(tagged));

	do {

	  continue_looping = 0;	/* clear the flag, ho hum... */

	  if (iterations++) {
	    printf("File message%s in: ", plural(tagged));
	    fflush(stdout);
	  }
	  else
	    PutLine1(LINES-2, 0, "File message%s in: ", plural(tagged));

	  if (save_by_name) {
	    /** build default filename to save to **/
	    get_return(address);
	    get_return_name(address, buffer, TRUE);
	    sprintf(filename, "=%s", buffer);
	  }
	  else
	    filename[0] = '\0';

	  if (iterations > 1) {
	    optionally_enter(filename, -1, -1, FALSE);
	  } 
	  else {
	    if (tagged > 1)
	      optionally_enter(filename, LINES-2, 19, FALSE);
	    else	
	      optionally_enter(filename, LINES-2, 18, FALSE);
	  }
  
	  if (iterations == 1)
	    MoveCursor(LINES-1,0);

	  if (strlen(filename) == 0) {  /** <return> means 'cancel', right? **/
	    header_table[current-1].status = oldstat;	/* BACK! */
	    return(0);
	  }
	 
	  if (strcmp(filename,"?") == 0) {
	    *redraw = TRUE;	/* set the flag so we know what to do later */
	    list_folders();
	    continue_looping++;
	  }
	} while (continue_looping);

	if (! expand_filename(filename)) {
	  dprint1(2,"Error: Failed on expansion of filename %s (save)\n", 
		   filename);
	  header_table[current-1].status = oldstat;	/* BACK! */
	  return(0);	/* failed expanding name! */
	}

	if (access(filename,ACCESS_EXISTS)) 	/* already there!! */
	  appending = 1;
	  
#ifdef BSD4.1
	if ((errno = ((can_open(filename, "a") & ~0x0200) >>8))) {
#else
	if ((errno = can_open(filename, "a"))) {
#endif
	  error1("Wrong permissions to save message to file %s!", filename);
	  dprint2(2,"Error: access permission on file %s denied (%s)! (save)\n",
		  filename, error_name(errno));
	  header_table[current-1].status = oldstat;	/* BACK! */
	  return(0);
	}

	dprint1(4,"Saving mail to file '%s'...\n", filename);

	if ((save_file = fopen(filename,"a")) == NULL) {
	  dprint1(2, "Error: couldn't append to specified file %s (save)\n", 
		   filename);
	  error1("Couldn't append to file %s!", filename);
	  header_table[current-1].status = oldstat;	/* BACK! */
	  return(0); 
	}

	for (i=0; i < message_count; i++) 	/* save each tagged msg */
	  if (header_table[i].status & TAGGED)
	    save_message(i, filename, save_file, (tagged > 1), appending++);

	fclose(save_file);

	chown(filename, userid, groupid);	/* owned by user */
	if (tagged > 1)
	  error1("Message%s saved", plural(tagged));
	return(1);
}

int
save_message(number, filename, fd, pause, appending)
int number, pause, appending;
char *filename;
FILE *fd;
{
	/** Save an actual message to a file.  This is called by 
	    "save()" only!  The parameters are the message number,
	    and the name and file descriptor of the file to save to.
	    If 'pause' is true, a sleep(2) will be done after the
	    saved message appears on the screen...
	    'appending' is only true if the file already exists 
	**/

	register int save_current;
	
	dprint1(4, "\tSaving message %d to file...\n", number);

	save_current = current;
	current = number+1;
	copy_message("", fd, FALSE, FALSE);
	current = save_current;

	if (resolve_mode)
	  setit(header_table[number].status, DELETED); /* deleted, but ...   */
	clearit(header_table[number].status, TAGGED);  /* not tagged anymore */
	clearit(header_table[number].status, NEW);     /* it's not new now!  */

	if (! appending)	/* don't ask */
	  error2("Message %d appended to file %s", number+1, filename);
	else
	  error2("Message %d saved to file %s", number+1, filename);

	show_new_status(number);	/* update screen, if needed */

	if (pause) sleep(2);
}

int
expand_filename(filename)
char *filename;
{
	/** Expands '~' and '=' to specified file names, also will try to 
	    expand shell variables if encountered.. '+' and '%' are synonymous 
	    with '=' (folder dir)... **/

	char buffer[SLEN], varname[SLEN], env_value[SLEN], *ptr;
	register int i = 1, index = 0;
	char *getenv();

	ptr = filename;
	while (*ptr == ' ') ptr++;	/* leading spaces GONE! */
	strcpy(filename, ptr);

	/** New stuff - make sure no illegal char as last **/

	if (lastch(filename) == '\n' || lastch(filename) == '\r')
	  lastch(filename) = '\0';
	  
	if (filename[0] == '~') {
	  sprintf(buffer, "%s%s%s", home, 
		(filename[1] != '/' && lastch(folders) != '/')? "/" : "",
	  	(char *) filename + 1);
	  strcpy(filename, buffer);
	}
	else if (filename[0] == '=' || filename[0] == '+' || 
	 	 filename[0] == '%') {
	  if (strlen(folders) == 0) {
	    dprint2(3,"Error: maildir not defined - can't expand '%c' (%s)\n",
		     filename[0], "expand_filename");
	    error1("MAILDIR not defined.  Can't expand '%c'", filename[0]);
	    return(0);
	  }
	  sprintf(buffer, "%s%s%s", folders, 
		(filename[1] != '/' && lastch(folders) != '/')? "/" : "",
		(char *) filename + 1);
	  strcpy(filename, buffer);
	}
	else if (filename[0] == '$') {	/* env variable! */
	  while (isalnum(filename[i]))
	    varname[index++] = filename[i++];
	  varname[index] = '\0';

	  env_value[0] = '\0';			/* null string for strlen! */
	  if (getenv(varname) != NULL)
	    strcpy(env_value, getenv(varname));

	  if (strlen(env_value) == 0) {
	    dprint2(3,"Error: Can't expand environment variable $%s (%s)\n",
		    varname, "expand_filename");
	    error1("Don't know what the value of $%s is!", varname);
	    return(0);
	  }

	  sprintf(buffer, "%s%s%s", env_value, 
		  (filename[i] != '/' && lastch(env_value) != '/')? "/" : "",
		  (char *) filename + i);
	  strcpy(filename, buffer);
	}
	  
	return(1);
}
