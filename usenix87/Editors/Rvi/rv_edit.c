#include "rv.h"
#include <ctype.h>

char *nextfile;  /* Ptr to name of next file to edit */

extern 	char editlist[256]; /* List of files to edit */

void
edit(name)
/*
 * Edit named file
 * If null, edit next file in editlist[]
 */
char *name;
{
	register char *s;
	register INT c;
	char buf[256];
	extern alarmring();

	xmit_curline();

	if (nextfile == NULL)
		nextfile = editlist;

	if (name != NULL && *name != '\0') {
		strncpy(editlist, name, 255);
		nextfile = editlist;
	}

	s = nextfile;
	while (isspace(*s) && *s != '\0')
		++s;
	name = s;
	while (!isspace(*s) && *s != '\0')
		++s;
	if (*s != '\0') {
		*s++ = '\0';
		while (isspace(*s) && s != '\0')
			++s;
	}
	nextfile = s;

	if (name == NULL || *name == '\0')
		name = file.fi_name;

	if (name == NULL || *name == '\0' || strcmp(name, "/dev/null") == 0) {
		botprint(TRUE, "Missing file name");
		hitcr_continue();
		refresh();
		fetch_window(1, TRUE);
		move_abs_cursor(1, 0);
		return;
	}

	s = name+strlen(name)-1;
	if (*(s-1) == '.' && *s == 'f')
		set_fortran = TRUE;
	clear();
	botprint(FALSE, "\"%s\" %s", name, set_fortran ? "[fortran mode]" : "");
	hitcr_continue();
	refresh();
	xmit_sync();
	xmit_ed("e %s\n", name);
	(void) recv_sync(FALSE);
	alarm(RV_TIMEOUT);
	fgets(buf, 255, file.fi_fpin);
	if (buf[0] == '?' && (buf[1] == '\n' || buf[1] == ' ')) {
		/*
		 * Ed is complaining about something, but we don't
		 * know what it is.  It could be trying to edit a
		 * directory, or the old file may have been modified
		 * without saving the changes.
		 */
		/* Get err message from ed, if possible */
		if (file.fi_sysv && (!file.fi_cray || buf[1] != ' '))
			fgets(buf, 255, file.fi_fpin);
		alarm(0);
		if (!file.fi_modified && file.fi_numlines > 1) {
			if (file.fi_sysv)
				botprint(TRUE, "%s", buf);
			botprint(FALSE, "Are you sure?");
			refresh();
			while ((c = getch()) != 'y' && c != 'Y'
					&& c != 'n' && c != 'N')
				;
			hitcr_continue();
			if (c == 'n' || c == 'N') {
				if (file.fi_name[0] == '\0')
					strcpy(file.fi_name, ".");
				xmit_ed("f %s\n", file.fi_name);
				redraw_screen((struct li_line *)0);
				botprint(FALSE, "Continue editing \"%s\"",
					file.fi_name);
				return;
			}
		}
		alarm(RV_TIMEOUT);
		xmit_ed("e %s\n", name);
		fflush(file.fi_fpout);
		fgets(buf, 255, file.fi_fpin);
	}

	xmit_ed("f %s\n", name);
	if (name != file.fi_name)
		strncpy(file.fi_name, name, 126);
	if (buf[0] == '?') { /* If error */
		if (file.fi_sysv) {  /* Get err msg */
			if (!file.fi_cray || buf[1] != ' ')
				fgets(buf, 255, file.fi_fpin);
		} else
			strcpy(buf, "File not found");
		alarm(0);
		refresh();
		hitcr_continue();
		fetch_window(1, TRUE);
		botprint(TRUE, "%s", buf);
	} else {
		alarm(0);
		fetch_window(1, TRUE);
		file.fi_modified = FALSE;
		move_abs_cursor(1, 0);
		botprint(FALSE, "\"%s\"%s %d lines, %d characters", name,
			set_fortran ? " [fortran mode]" : "",
			file.fi_numlines, atoi(buf));
	} 
	move_abs_cursor(1, 0);
}
