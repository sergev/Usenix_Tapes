/**			save_opts.c			**/

/** This file contains the routine needed to allow the users to change the
    Elm parameters and then save the configuration in a ".elmrc" file in
    their home directory.  With any luck this will allow them never to have
    to actually EDIT the file!!

    (C) Copyright 1986, Dave Taylor
**/

#include "headers.h"
#include <errno.h>

#undef onoff
#define   onoff(n)	(n == 1? "ON":"OFF")

#define absolute(x)		((x) < 0? -(x) : (x))

extern  int errno;

char *error_name(), *nameof(), *sort_name();
long  ftell();

#include "save_opts.h"

FILE *elminfo;		/* informational file as needed... */

save_options()
{
	/** Save the options currently specified to a file.  This is a
	    fairly complex routine since it tries to put in meaningful
	    comments and such as it goes along.  The comments are
	    extracted from the file ELMRC_INFO as defined in the sysdefs.h
	    file.  THAT file has the format;

		varname
		  <comment>
		  <comment>
		<blank line>

	    and each comment is written ABOVE the variable to be added.  This
	    program also tries to make 'pretty' stuff like the alternatives
	    and such.
	**/

	FILE *newelmrc; 
	char  oldfname[SLEN], newfname[SLEN];

	sprintf(newfname, "%s/%s", home, elmrcfile);
	sprintf(oldfname, "%s/.old%s", home, elmrcfile);

	/** first off, let's see if they already HAVE a .elmrc file **/

	if (access(newfname, ACCESS_EXISTS) != -1) {
	  /** YES!  Copy it to the file ".old.elmrc".. **/
	  (void) unlink(oldfname);
	  (void) link(newfname, oldfname);
          (void) unlink(newfname);
	  (void) chown(oldfname, userid, groupid);
	}

	/** now let's open the datafile if we can... **/

	if ((elminfo = fopen(ELMRC_INFO, "r")) == NULL) 
	  error1("Warning: saving without comments - can't get to %s", 
		  ELMRC_INFO);

	/** next, open the new .elmrc file... **/

	if ((newelmrc = fopen(newfname, "w")) == NULL) {
	   error2("Can't save configuration: can't write to %s [%s]",
		   newfname, error_name(errno));
	   return;
	}
	
	save_user_options(elminfo, newelmrc);

	error1("Options saved in file %s", newfname);
}

save_user_options(elminfo_fd, newelmrc)
FILE *elminfo_fd, *newelmrc;
{
	/** save the information in the file.  If elminfo_fd == NULL don't look
	    for comments!
	**/

	if (elminfo_fd != NULL) 
	  build_offset_table(elminfo_fd);

	fprintf(newelmrc, 	
	          "#\n# .elmrc - options file for the Elm mail system\n#\n");

	if (strlen(full_username) > 0)
	  fprintf(newelmrc, "# Saved automatically by Elm %s for %s\n#\n\n",
		  VERSION, full_username);
	else
	  fprintf(newelmrc, "# Saved automatically by Elm %s\n#\n\n", VERSION);

	save_option_string(CALENDAR, calendar_file, newelmrc, FALSE);
	save_option_string(EDITOR, editor, newelmrc, FALSE);
	save_option_string(FULLNAME, full_username, newelmrc, FALSE);
	save_option_string(MAILBOX, nameof(mailbox), newelmrc, FALSE);
	save_option_string(MAILDIR, folders, newelmrc, FALSE);
	save_option_string(PAGER, pager, newelmrc, FALSE);
	save_option_string(PREFIX, prefixchars, newelmrc, TRUE);
	save_option_string(PRINT, printout, newelmrc, FALSE);
	save_option_string(SAVEMAIL, savefile, newelmrc, FALSE);
	save_option_string(SHELL, shell, newelmrc, FALSE);

	save_option_string(LOCALSIGNATURE, local_signature, newelmrc, FALSE);
	save_option_string(REMOTESIGNATURE, remote_signature, newelmrc, FALSE);

	save_option_sort(SORTBY, sortby, newelmrc);

	save_option_on_off(ALWAYSDELETE, always_del, newelmrc);
	save_option_on_off(ALWAYSLEAVE, always_leave, newelmrc);
	save_option_on_off(ARROW, arrow_cursor, newelmrc);
	save_option_on_off(AUTOCOPY, auto_copy, newelmrc);

	save_option_number(BOUNCEBACK, bounceback, newelmrc);

	save_option_on_off(COPY, auto_cc, newelmrc);
/**	save_option_on_off(EDITOUT, edit_outbound, newelmrc);		     **/
	save_option_on_off(FORMS, allow_forms, newelmrc);
	save_option_on_off(KEYPAD, hp_terminal, newelmrc);
	save_option_on_off(MENU, mini_menu, newelmrc);
	save_option_on_off(MOVEPAGE, move_when_paged, newelmrc);
	save_option_on_off(NAMES, names_only, newelmrc);
	save_option_on_off(NOHEADER, noheader, newelmrc);
	save_option_on_off(POINTNEW, point_to_new, newelmrc);
	save_option_on_off(RESOLVE, resolve_mode, newelmrc);
	save_option_on_off(SAVENAME, save_by_name, newelmrc);
	save_option_on_off(SOFTKEYS, hp_softkeys, newelmrc);

	save_option_number(TIMEOUT, timeout, newelmrc);

	save_option_on_off(TITLES, title_messages, newelmrc);

	save_option_number(USERLEVEL, user_level, newelmrc);

	save_option_on_off(WARNINGS, warnings, newelmrc);
	save_option_on_off(WEED, filter, newelmrc);

	save_option_weedlist(WEEDOUT, weedlist, newelmrc);
	save_option_alternatives(ALTERNATIVES, alternative_addresses, newelmrc);

	fflush(elminfo_fd);	/* make sure we're clear... */
	fclose(elminfo_fd);
}

save_option_string(index, value, fd, underscores)
int index, underscores;
char *value;
FILE *fd;
{
	/** Save a string option to the file... only subtlety is when we
	    save strings with spaces in 'em - translate to underscores!
	**/

	register int i;
	char     buffer[SLEN];
	
	if (strlen(value) == 0) return;		/* why bother? */

	add_comment(index, fd);
	
	strcpy(buffer, value);

	if (underscores)
	  for (i=0; i < strlen(buffer); i++)
	    if (buffer[i] == SPACE) buffer[i] = '_';

	fprintf(fd, "%s = %s\n\n", save_info[index].name, buffer);
}
	   
save_option_sort(index, value, fd)
int index;
char *value;
FILE *fd;
{
	/** save the current sorting option to a file **/

	add_comment(index, fd);

	fprintf(fd, "%s = %s\n\n", save_info[index].name,
		sort_name(SHORT));
}

save_option_number(index, value, fd)
int index, value;
FILE *fd;
{
	/** Save a binary option to the file - boy is THIS easy!! **/

	add_comment(index, fd);
	
	fprintf(fd, "%s = %d\n\n", save_info[index].name, value);
}

save_option_on_off(index, value, fd)
int index, value;
FILE *fd;
{
	/** Save a binary option to the file - boy is THIS easy!! **/

	add_comment(index, fd);
	
	fprintf(fd, "%s = %s\n\n", save_info[index].name, onoff(value));
}

save_option_weedlist(index, list, fd)
int index;
char *list[];
FILE *fd;
{
	/** save a list of weedout headers to the file **/

	int length_so_far = 0, i;

	add_comment(index, fd);

	length_so_far = strlen(save_info[index].name) + 4;

	fprintf(fd, "%s = ", save_info[index].name);

	/** first off, skip till we get past the default list **/

	for (i = 0; i < weedcount; i++) 
	  if (strcmp(weedlist[i],"*end-of-defaults*") == 0)
	    break;

	while (i < weedcount) {
	  if (strlen(weedlist[i]) + length_so_far > 78) {
	    fprintf(fd, "\n\t");
	    length_so_far = 8;
	  }
	  fprintf(fd, "\"%s\" ", weedlist[i]);
	  length_so_far += (strlen(weedlist[i]) + 4);
	  i++;
	}
	fprintf(fd, "\n\n");
}

save_option_alternatives(index, list, fd)
int index;
struct addr_rec *list;
FILE *fd;
{
	/** save a list of options to the file **/
	int length_so_far = 0;
	struct addr_rec     *alternate;

	if (list == NULL) return;	/* nothing to do! */

	add_comment(index, fd);

	alternate = list;	/* don't LOSE the top!! */

	length_so_far = strlen(save_info[index].name) + 4;

	fprintf(fd, "%s = ", save_info[index].name);

	while (alternate != NULL) {
	  if (strlen(alternate->address) + length_so_far > 78) {
	    fprintf(fd, "\n\t");
	    length_so_far = 8;
	  }
	  fprintf(fd, "%s  ", alternate->address);
	  length_so_far += (strlen(alternate->address) + 3);
	  alternate = alternate->next;
	}
	fprintf(fd, "\n\n");
}

add_comment(index, fd)
int index;
FILE *fd;
{	
	/** get to and add the comment to the file **/
	char buffer[SLEN];

	/** first off, add the comment from the comment file, if available **/

	if (save_info[index].offset > 0L) {
	  if (fseek(elminfo, save_info[index].offset, 0)) {
	    dprint2(1,"** error %s seeking to %ld in elm-info file!\n",
		     error_name(errno), save_info[index].offset);
	  }
	  else while (fgets(buffer, SLEN, elminfo) != NULL) {
	    if (buffer[0] != '#') 
	       break;
	    else
	       fprintf(fd, "%s", buffer);
	  }
	}
}

build_offset_table(elminfo_fd)
FILE *elminfo_fd;
{
	/** read in the info file and build the table of offsets.
	    This is a rather laborious puppy, but at least we can
	    do a binary search through the array for each element and
	    then we have it all at once!
	**/

	char line_buffer[SLEN];
	
	while (fgets(line_buffer, SLEN, elminfo_fd) != NULL) {
	  if (strlen(line_buffer) > 1)
	    if (line_buffer[0] != '#' && !whitespace(line_buffer[0])) {
	       no_ret(line_buffer);
	       if (find_and_store_loc(line_buffer, ftell(elminfo_fd))) {
	         dprint1(1, "** Couldn't find and store \"%s\" **\n", 
			 line_buffer);
	       }
	    }
	}
}

find_and_store_loc(name, offset)
char *name;
long  offset;
{
	/** given the name and offset, find it in the table and store it **/

	register int first = 0, last, middle, compare;

	last = NUMBER_OF_SAVEABLE_OPTIONS;

	while (first <= last) {

	  middle = (first+last) / 2;

	  if ((compare = strcmp(name, save_info[middle].name)) < 0) /* a < b */
	    last = middle - 1;
	  else if (compare == 0) {				    /* a = b */
	    save_info[middle].offset = offset;
	    return(0);
	  }
	  else  /* greater */					    /* a > b */
	    first = middle + 1; 
	}

	return(-1);
}
