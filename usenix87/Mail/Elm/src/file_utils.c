/**			file_utils.c			**/

/** File oriented utility routines for ELM 

    (C) Copyright 1986 Dave Taylor
**/

#include "headers.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

#ifdef BSD
# undef tolower
#endif

#include <signal.h>
#include <errno.h>

#ifdef BSD
# include <sys/wait.h>
#endif

extern int errno;		/* system error number */

char *error_name(), *error_description(), *strcpy(), *getlogin();

long
bytes(name)
char *name;
{
	/** return the number of bytes in the specified file.  This
	    is to check to see if new mail has arrived....  **/

	int ok = 1;
	extern int errno;	/* system error number! */
	struct stat buffer;

	if (stat(name, &buffer) != 0)
	  if (errno != 2) {
	    dprint2(1,"Error: errno %s on fstat of file %s (bytes)\n", 
		     error_name(errno), name);
	    Write_to_screen("\n\rError attempting fstat on file %s!\n\r",
		     1, name);
	    Write_to_screen("** %s - %s **\n\r", 2, error_name(errno),
		  error_description(errno));
	    emergency_exit();
	  }
	  else
	    ok = 0;
	
	return(ok ? (long) buffer.st_size : 0L);
}

int
can_access(file, mode)
char *file; 
int   mode;
{
	/** returns ZERO iff user can access file or "errno" otherwise **/

	int the_stat = 0, pid, w; 
	void _exit(), exit();
#ifdef BSD
	union wait status;
#else
	int status;
#endif
	register int (*istat)(), (*qstat)();
	
#ifdef NO_VM		/* machine without virtual memory!! */
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif
	  setgid(groupid);
	  setuid(userid);		/** back to normal userid **/

	  errno = 0;

	  if (access(file, mode) == 0) 
	    _exit(0);
	  else 
	    _exit(errno != 0? errno : 1);	/* never return zero! */
	  _exit(127);
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pid && w != -1)
		;

#ifdef BSD
	the_stat = status.w_retcode;
#else
	the_stat = status;
#endif

	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);

	return(the_stat);
}

int
can_open(file, mode)
char *file, *mode;
{
	/** Returns 0 iff user can open the file.  This is not
	    the same as can_access - it's used for when the file might
	    not exist... **/

	FILE *fd;
	int the_stat = 0, pid, w; 
	void _exit(), exit();
#ifdef BSD
	union wait status;
#else
	int status;
#endif
	register int (*istat)(), (*qstat)();
	
#ifdef NO_VM		/* machine without virtual memory!! */
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif
	  setgid(groupid);
	  setuid(userid);		/** back to normal userid **/
	  errno = 0;
	  if ((fd = fopen(file, mode)) == NULL)
	    _exit(errno);
	  else {
	    fclose(fd);		/* don't just LEAVE it! */
	    _exit(0);
	  }
	  _exit(127);
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pid && w != -1)
		;

#ifdef BSD
	the_stat = status.w_retcode;
#else
	the_stat = status;
#endif
	
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);

	return(the_stat);
}

int
copy(from, to)
char *from, *to;
{
	/** this routine copies a specified file to the destination
	    specified.  Non-zero return code indicates that something
	    dreadful happened! **/

	FILE *from_file, *to_file;
	char buffer[VERY_LONG_STRING];
	
	if ((from_file = fopen(from, "r")) == NULL) {
	  dprint1(1,"Error: could not open %s for reading (copy)\n", from);
	  error1("could not open file %s", from);
	  return(1);
	}

	if ((to_file = fopen(to, "w")) == NULL) {
	  dprint1(1,"Error: could not open %s for writing (copy)\n", to);
	  error1("could not open file %s", to);
	  return(1);
	}

	while (fgets(buffer, VERY_LONG_STRING, from_file) != NULL)
	  fputs(buffer, to_file);

	fclose(from_file);
	fclose(to_file);

	return(0);
}

int
append(fd, filename)
FILE *fd;
char *filename;
{
	/** This routine appends the specified file to the already
	    open file descriptor.. Returns non-zero if fails.  **/

	FILE *my_fd;
	char buffer[VERY_LONG_STRING];
	
	if ((my_fd = fopen(filename, "r")) == NULL) {
	  dprint1(1,"Error: could not open %s for reading (append)\n", filename);
	  return(1);
	}

	while (fgets(buffer, VERY_LONG_STRING, my_fd) != NULL)
	  fputs(buffer, fd);

	fclose(my_fd);

	return(0);
}

check_mailfile_size()
{
	/** Check to ensure we have mail.  Only used with the '-z'
	    starting option. **/

	char filename[SLEN], *getlogin();
	struct stat buffer;

	strcpy(username,getlogin());
	if (strlen(username) == 0)
	  cuserid(username);

	sprintf(filename,"%s%s", mailhome, username);

	if (stat(filename, &buffer) == -1) {
	  printf(" You have no mail.\n");
	  exit(0);
	}
	else if (buffer.st_size < 2) { 		/* maybe one byte??? */
	  printf("You have no mail to read.\n");
	  exit(0);
	}
}

create_readmsg_file()
{
	/** Creates the file ".current" in the users home directory
	    for use with the "readmsg" program.
	**/

	FILE *fd;
	char buffer[SLEN];

	sprintf(buffer,"%s/%s", home, readmsg_file);

	if ((fd = fopen (buffer, "w")) == NULL) {
	  dprint3(1,"Error: couldn't create file %s - error %s (%s)\n",
		 buffer, error_name(errno), "create_readmsg_file");
	  return;	/* no error to user */
	}

	fprintf(fd, "%d\n", header_table[current-1].index_number);
	fclose(fd);
}
