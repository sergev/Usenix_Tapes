/**		utils.c		**/

/** Utility routines for ELM 

    All routines herein: (C) Copyright 1985 Dave Taylor
**/

#include "headers.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

#ifdef BSD
#undef tolower
#endif

#include <signal.h>

extern int errno;

char *error_name();
void   exit();

show_mailfile_stats()
{
	/** when we're about to die, let's try to dump lots of good stuff
	    to the debug file... **/

	struct stat buffer;

	if (debug == 0) return;		/* Damn!  Can't do it! */

	if (fstat(fileno(mailfile), &buffer) == 0) {
	  dprint1(1,"\nDump of stats for mailfile %s;\n", infile);

	  dprint3(1, "\tinode: %d, mode: %o, uid: %d, ",
			buffer.st_ino, buffer.st_mode, buffer.st_uid);
	  dprint2(1,"gid: %d, size: %d\n\n", buffer.st_gid, buffer.st_size);

	  dprint1(1,"\toffset into file = %l\n", ftell(mailfile));
	}
	else
	  dprint2(1,"\nfstat on mailfile '%s' failed with error %s!!\n\n",
			infile, error_name(errno));
}
	
emergency_exit()
{
	/** used in dramatic cases when we must leave without altering
	    ANYTHING about the system... **/

	dprint0(1,
     "\nERROR: Something dreadful is happening!  Taking emergency exit!!\n\n");
	dprint0(1,"  possibly leaving behind the following files;\n");
	dprint2(1,"     The mailbox tempfile : %s%s\n", temp_mbox, username);
	dprint2(1,"     The mailbox lock file: %s%s.lock\n", mailhome, username);
	dprint2(1,"     The composition file : %s%d\n", temp_file, getpid());
	dprint2(1,"     The header comp file : %s%d\n", temp_file, getpid()+1);
	dprint2(1,"     The readmsg data file: %s/%s\n", home, readmsg_file);

	Raw(OFF);
	if (cursor_control)  transmit_functions(OFF);
	if (hp_terminal)     softkeys_off();

	if (cursor_control)
	  MoveCursor(LINES, 0);

	PutLine0(LINES,0, "\nEmergency Exit taken!  All temp files intact!\n\n");

	exit(1);
}

/*ARGSUSED*/
/*VARARGS0*/

leave(val)
int val;	/* not used, placeholder for signal catching! */
{
	char buffer[SLEN];

	dprint0(2,"\nLeaving mailer normally (leave)\n");

	Raw(OFF);
	if (cursor_control)  transmit_functions(OFF);
	if (hp_terminal)     softkeys_off();

	sprintf(buffer,"%s%d",temp_file, getpid());  /* editor buffer */
	(void) unlink(buffer);

	sprintf(buffer,"%s%d",temp_file, getpid()+1);  /* editor buffer */
	(void) unlink(buffer);

	sprintf(buffer,"%s%s",temp_mbox, username);  /* temp mailbox */
	(void) unlink(buffer);

	sprintf(buffer,"%s/%s", home, readmsg_file);  /* readmsg temp */
	(void) unlink(buffer);

	sprintf(buffer,"%s%s.lock",mailhome, username); /* lock file */
	(void) unlink(buffer);

	if (! mail_only) {
	  MoveCursor(LINES,0);
	  Writechar('\n');
	}

	exit(0);
}

silently_exit()
{
	/** This is the same as 'leave', but it doesn't remove any non-pid
	    files.  It's used when we notice that we're trying to create a
	    temp mail file and one already exists!!
	**/
	char buffer[SLEN];

	dprint0(2,"\nLeaving mailer quietly (silently_exit)\n");

	Raw(OFF);
	if (cursor_control)  transmit_functions(OFF);
	if (hp_terminal)     softkeys_off();

	sprintf(buffer,"%s%d",temp_file, getpid());  /* editor buffer */
	(void) unlink(buffer);

	sprintf(buffer,"%s%d",temp_file, getpid()+1);  /* editor buffer */
	(void) unlink(buffer);

	if (! mail_only) {
	  MoveCursor(LINES,0);
	  Writechar('\n');
	}

	exit(0);
}

/*ARGSUSED0*/

leave_locked(val)
int val;	/* not used, placeholder for signal catching! */
{
	/** same as leave routine, but don't disturb lock file **/

	char buffer[SLEN];

        dprint0(3,
	    "\nLeaving mailer due to presence of lock file (leave_locked)\n");

	Raw(OFF);
	if (cursor_control)  transmit_functions(OFF);
	if (hp_terminal)     softkeys_off();

	sprintf(buffer,"%s%d",temp_file, getpid());  /* editor buffer */
	(void) unlink(buffer);

	sprintf(buffer,"%s%d",temp_file, getpid()+1);  /* editor buffer */
	(void) unlink(buffer);

	sprintf(buffer,"%s%s",temp_mbox, username);  /* temp mailbox */
	(void) unlink(buffer);

	MoveCursor(LINES,0);
	Writechar('\n');

	exit(0);
}

int
get_page(msg_pointer)
int msg_pointer;
{
	/** Ensure that 'current' is on the displayed page,
	    returning non-zero iff the page changed! **/

	register int first_on_page, last_on_page;

	dprint1(6,"* get_page(%d) returns...", msg_pointer);

	first_on_page = (header_page * headers_per_page) + 1;

	last_on_page = first_on_page + headers_per_page - 1;

	dprint2(8,"[first-on-page=%d, last-on-page=%d]",
		first_on_page, last_on_page);

	if (selected)	/* but what is it on the SCREEN??? */
	  msg_pointer = compute_visible(msg_pointer-1);

	if (selected && msg_pointer > selected) {
	  dprint0(6,"FALSE - too far!\n");
	  return(FALSE);	/* too far - page can't change! */
	}

	if (msg_pointer > last_on_page) {
	  header_page = (int) (msg_pointer-(selected? 0:1)) / headers_per_page;
	  dprint3(6,"TRUE (%d > %d  New hp=%d)!\n",
		msg_pointer, last_on_page, header_page);
	  return(1);
	}
	else if (msg_pointer < first_on_page) {
	  header_page = (int) (msg_pointer-1) / headers_per_page;
	  dprint3(6,"TRUE (%d < %d   New hp=%d)!\n",
		msg_pointer, first_on_page, header_page);
	  return(1);
	}
	else {
	  dprint2(6,"FALSE [first=%d last=%d]\n",
		  first_on_page, last_on_page);
	  return(0);
	}
}

char *nameof(filename)
char *filename;
{
	/** checks to see if 'filename' has any common prefixes, if
	    so it returns a string that is the same filename, but 
	    with '=' as the folder directory, or '~' as the home
	    directory..
	**/

	static char buffer[STRING];
	register int i = 0, index = 0;

	if (strncmp(filename, folders, strlen(folders)) == 0) {
	  buffer[i++] = '=';
	  index = strlen(folders);
	}
	else if (strncmp(filename, home, strlen(home)) == 0) {
	  buffer[i++] = '~';
	  index = strlen(home);
	}
	else index = 0;

	while (filename[index] != '\0')
	  buffer[i++] = filename[index++];
	buffer[i] = '\0';
	
	return( (char *) buffer);
}
