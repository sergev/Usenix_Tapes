/**			syscall.c		**/

/** These routines are used for user-level system calls, including the
    '!' command and the '|' commands...

    (C) Copyright 1986 Dave Taylor
**/

#include "headers.h"

#include <signal.h>

#ifdef BSD
#  include <sys/wait.h>
#endif

char *argv_zero();	
void  _exit();

int
subshell()
{
	/** spawn a subshell with either the specified command
	    returns non-zero if screen rewrite needed
	**/

	char command[SLEN];
	int  ret;

	PutLine0(LINES-3,COLUMNS-40,"(use the shell name for a shell)");
	PutLine0(LINES-2,0,"Shell Command: ");
	command[0] = '\0';
	(void) optionally_enter(command, LINES-2, 15, FALSE);
	if (strlen(command) == 0) {
	  MoveCursor(LINES-2,0);	CleartoEOLN();
	  return(0);
	}

	MoveCursor(LINES,0); 	CleartoEOLN();
	Raw(OFF);
	if (cursor_control)  transmit_functions(OFF);
	
	ret = system_call(command, USER_SHELL);

	PutLine0(LINES, 0, "\n\nPress <return> to return to ELM: ");

	Raw(ON);
	(void) getchar();
	if (cursor_control)  transmit_functions(ON);

	if (ret != 0) error1("Return code was %d", ret);
	return(1);
}

system_call(string, shell_type)
char *string;
int   shell_type;
{
	/** execute 'string', setting uid to userid... **/
	/** if shell-type is "SH" /bin/sh is used regardless of the 
	    users shell setting.  Otherwise, "USER_SHELL" is sent **/

	int stat = 0, pid, w;
#ifdef BSD
	union wait status;
#else
	int status;
#endif
	register int (*istat)(), (*qstat)();
	
	dprint2(2,"System Call: %s\n\t%s\n", shell_type == SH? "/bin/sh" : shell,
		string);

#ifdef NO_VM		/* machine without virtual memory! */
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif
	  setgid(groupid);	/* and group id		    */
	  setuid(userid);	/* back to the normal user! */

	  if (strlen(shell) > 0 && shell_type == USER_SHELL) {
	    execl(shell, argv_zero(shell), "-c", string, (char *) 0);
	  }
	  else 
	    execl("/bin/sh", "sh", "-c", string, (char *) 0);
	  _exit(127);
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pid && w != -1)
		;

#ifdef BSD
	if (status.w_retcode != 0) stat = status.w_retcode;
#else
	if (w == -1) stat = status;
#endif
	
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);

	return(stat);
}

int
do_pipe()
{
	/** pipe the tagged messages to the specified sequence.. **/

	char command[SLEN], buffer[LONG_SLEN], message_list[SLEN];
	register int  ret, tagged = 0, i;

	message_list[0] = '\0';	/* NULL string to start... */

	for (i=0; i < message_count; i++)
	  if (ison(header_table[i].status, TAGGED)) {
	    sprintf(message_list,"%s %d", message_list, 
		    header_table[i].index_number);
	    tagged++;
	  }

	if (tagged > 1)
	  PutLine0(LINES-2,0,"Pipe tagged msgs to: ");
	else if (tagged) 
	  PutLine0(LINES-2,0,"Pipe tagged msg to : ");
	else {
	  PutLine0(LINES-2,0,"Pipe current msg to: ");
	  sprintf(message_list,"%d", header_table[current-1].index_number);
	}

	command[0] = '\0';

	(void) optionally_enter(command, LINES-2, 21, FALSE);
	if (strlen(command) == 0) {
	  MoveCursor(LINES-2,0);	CleartoEOLN();
	  return(0);
	}

	MoveCursor(LINES,0); 	CleartoEOLN();
	Raw(OFF);

	if (cursor_control)  transmit_functions(OFF);
	
	sprintf(buffer, "%s -f %s -h %s | %s",
		readmsg,
		infile,
		message_list,
		command);
	
	ret = system_call(buffer, USER_SHELL);

	PutLine0(LINES, 0, "\n\nPress <return> to return to ELM: ");
	Raw(ON);
	(void) getchar();
	if (cursor_control)  transmit_functions(ON);

	if (ret != 0) error1("Return code was %d", ret);
	return(1);
}

printmsg()
{
	/** Print current message or tagged messages using 'printout' 
	    variable.  Error message iff printout not defined! **/

	char buffer[LONG_SLEN], filename[SLEN], printbuffer[LONG_SLEN];
	char message_list[SLEN];
	register int  retcode, tagged = 0, i;

	if (strlen(printout) == 0) {
	  error("Don't know how to print - option \"printmail\" undefined!");
	  return;
	}
	
	message_list[0] = '\0';	/* reset to null... */

	for (i=0; i < message_count; i++) 
	  if (header_table[i].status & TAGGED) {
	    sprintf(message_list, "%s %d", message_list, 
		    header_table[i].index_number);
	    tagged++;
	  }

	if (! tagged)
	  sprintf(message_list," %d", header_table[current-1].index_number);

	sprintf(filename,"%s%d", temp_print, getpid());

	if (in_string(printout, "%s"))
	  sprintf(printbuffer, printout, filename);
	else
	  sprintf(printbuffer, "%s %s", printout, filename);

	sprintf(buffer,"(%s -p -f %s%s > %s; %s 2>&1) > /dev/null",
		readmsg, infile, message_list, 
		filename,
		printbuffer);
	
	dprint0(2,"Printing system call...\n");

  	Centerline(LINES, "queueing...");

	if ((retcode = system_call(buffer, SH)) == 0) {
	  sprintf(buffer, "Message%s queued up to print", plural(tagged));
	  Centerline(LINES, buffer);
	}
	else
	  error1("Printout failed with return code %d", retcode);

	unlink(filename);	/* remove da temp file! */
}

list_folders()
{
	/** list the folders in the users FOLDERHOME directory.  This is
	    simply a call to "ls -C"
	**/

	char buffer[SLEN];

	CleartoEOS();	/* don't leave any junk on the bottom of the screen */
	sprintf(buffer, "cd %s;ls -C", folders);
	printf("\n\rContents of your folder directory:\n\r\n\r");
	system_call(buffer, SH); 
	printf("\n\r");
}

