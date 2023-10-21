/**			autoreply.c			**/

/** This is the front-end for the autoreply system, and performs two 
    functions: it either adds the user to the list of people using the
    autoreply function (starting the daemon if no-one else) or removes
    a user from the list of people.

    Usage:  autoreply filename
	    autoreply "off"
	or  autoreply		[to find current status]
    
    (C) 1986, Dave Taylor
**/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "defs.h"

static char ident[] = { WHAT_STRING };

#define  tempdir	"/tmp/arep"		/* file prefix          */
#define  autoreply_file	"/etc/autoreply.data"   /* autoreply data file  */

extern   int errno;				/* system error code    */
char     username[NLEN];			/* login name of user   */

main(argc, argv)
int    argc;
char *argv[];
{
	char filename[SLEN];

	if (argc > 2) {
	  printf("Usage: %s <filename>\tto start autoreply,\n", argv[0]);
	  printf("       %s off\t\tto turn off autoreply\n", argv[0]);
	  printf("   or  %s    \t\tto check current status\n", argv[0]);
	  exit(1);
	}

	(void) cuserid(username);

	if (argc == 1 || strcmp(argv[1], "off") == 0) 
	  remove_user((argc == 1));
	else {
	  strcpy(filename, argv[1]);
	  if (access(filename,READ_ACCESS) != 0) {
	    printf("Error: Can't read file '%s'\n", filename);
	    exit(1);
	  }
	  
	  if (filename[0] != '/') /* prefix home directory */
	    sprintf(filename,"%s/%s", getenv("HOME"), argv[1]);

	  add_user(filename);
	}

	exit(0);
}

remove_user(stat_only)
int stat_only;
{
	/** Remove the user from the list of currently active autoreply 
	    people.  If 'stat_only' is set, then just list the name of
	    the file being used to autoreply with, if any. **/

	FILE *temp, *repfile;
	char  tempfile[SLEN], user[SLEN], filename[SLEN];
	int   c, copied = 0, found = 0;
	long  filesize, bytes();

	if (! stat_only) {
	  sprintf(tempfile, "%s.%06d", tempdir, getpid());

	  if ((temp = fopen(tempfile, "w")) == NULL) {
	    printf("Error: couldn't open tempfile '%s'.  Not removed\n",
		    tempfile);
	    exit(1);
	  }
	}

	if ((repfile = fopen(autoreply_file, "r")) == NULL) {
	  if (stat_only) {
	    printf("You're not currently autoreplying to mail.\n");
	    exit(0);
	  }
	  printf("No-one is autoreplying to their mail!\n");
	  exit(0);
	}

	/** copy out of real replyfile... **/

	while (fscanf(repfile, "%s %s %ld", user, filename, &filesize) != EOF) 

	  if (strcmp(user, username) != 0) {
	    if (! stat_only) {
	      copied++;
	      fprintf(tempfile, "%s %s %ld\n", user, filename, filesize);
	    }
	  }
	  else {
	    if (stat_only) {
	      printf("You're currently autoreplying to mail with the file %s\n",		      filename); 
	      exit(0);
	    }
	    found++;
	  }

	fclose(temp);
	fclose(repfile);

	if (! found) {
	  printf("You're not currently autoreplying to mail%s\n",
		  stat_only? "." : "!");
	  if (! stat_only)
	    unlink(tempfile);
	  exit(! stat_only);
	}

	/** now copy tempfile back into replyfile **/

	if (copied == 0) {	/* removed the only person! */
	  unlink(autoreply_file);
	}
	else {			/* save everyone else   */
	  
	  if ((temp = fopen(tempfile,"r")) == NULL) {
	    printf("Error: couldn't reopen tempfile '%s'.  Not removed.\n",
		    tempfile);
	    unlink(tempfile);
	    exit(1);
	  }

	  if ((repfile = fopen(autoreply_file, "w")) == NULL) {
	    printf(
          "Error: couldn't reopen autoreply file for writing!  Not removed.\n");
	    unlink(tempfile);
	    exit(1);
	  }

	  while ((c = getc(temp)) != EOF)
	    putc(c, repfile);

	  fclose(temp);
	  fclose(repfile);
	
	}
	unlink(tempfile);

	if (found > 1)
	  printf("Warning: your username appeared %d times!!   Removed all\n", 
		  found);
	else
	  printf("You've been removed from the autoreply table.\n");
}

add_user(filename)
char *filename;
{
	/** add the user to the autoreply file... **/

	FILE *repfile;
	char  mailfile[SLEN];
	long  bytes();

	if ((repfile = fopen(autoreply_file, "a")) == NULL) {
	  printf("Error: couldn't open the autoreply file!  Not added\n");
	  exit(1);
	}
	
	sprintf(mailfile,"%s/%s", mailhome, username);

	fprintf(repfile,"%s %s %ld\n", username, filename, bytes(mailfile));

	fclose(repfile);

	printf("You've been added to the autoreply system.\n");
}


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
	  if (errno != 2)
	   exit(fprintf(stderr,"Error %d attempting fstat on %s", errno, name));
	  else
	    ok = 0;
	
	return(ok ? buffer.st_size : 0L);
}
