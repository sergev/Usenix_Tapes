/**			connect_to.c			**/

/** This contains the routine(s) needed to have the Elm mailer figure
    out what machines the current machine can talk to.   This can be
    done in one of two ways - either the program can read the L.sys
    file, or (if it fails or "UUNAME" define is present) will invoke
    uuname to a file, then read the file in!

    (C) Copyright Dave Taylor, 1986
**/

#include "headers.h"

char *strcpy();

struct lsys_rec *pmalloc(); 

get_connections()
{

	/** get the direct connections that this machine has, by hook
	    or by crook (so to speak) 
	**/

#ifndef USE_UUNAME
	FILE *lsysfile;
	char  buffer[SLEN], sysname[NLEN];
	struct lsys_rec *system_record, *previous_record;
	int    loc_on_line;

	previous_record = NULL;
	if ((lsysfile = fopen(Lsys,"r")) == NULL) {
	  dprint1(1, "Warning: Can't open L.sys file %s (read_lsys)\n", Lsys);
#endif

	  if (read_uuname() == -1) {
	    error("Warning: couldn't figure out system connections...");
	    talk_to_sys = NULL;
	  }

#ifndef USE_UUNAME
	  /** ELSE: already read in uuname() output if we're here!! **/
	  return;
	}

	while (fgets(buffer, SLEN, lsysfile) != NULL) {
	  sscanf(buffer,"%s", sysname);

	  if (previous_record == NULL) {
	    dprint1(2, "L.sys\tdirect connection to %s, ", sysname);
	    loc_on_line = 30 + strlen(sysname);  
	    previous_record = pmalloc(sizeof *talk_to_sys);

	    strcpy(previous_record->name, sysname);
	    previous_record->next = NULL;
	    talk_to_sys = previous_record;
	  }
	  else if (! talk_to(sysname) && sysname[0] != '#') {
	    if (loc_on_line + strlen(sysname) > 80) {
	      dprint0(2, "\n\t");
	      loc_on_line = 8;
	    }
	    dprint1(2, "%s, ", sysname);
	    loc_on_line += (strlen(sysname) + 2);
	    system_record = pmalloc(sizeof *talk_to_sys);
	  
	    strcpy(system_record->name, sysname);
	    system_record->next = NULL;
	    previous_record->next = system_record;
	    previous_record = system_record;
	  }
	}

	fclose(lsysfile);

	if (loc_on_line != 8)
	  dprint0(2, "\n");

	dprint0(2, "\n");			/* for a nice format! Yeah! */
#endif
}

int
read_uuname()
{
	/** This routine trys to use the uuname routine to get the names of
	    all the machines that this machine connects to...it returns
	    -1 on failure.
	**/

	FILE *fd;
	char  buffer[SLEN], filename[SLEN];
	struct lsys_rec *system_record, *previous_record;
	int   loc_on_line;

	sprintf(filename, "%s%d", temp_uuname, getpid());
	sprintf(buffer,"%s > %s", uuname, filename);

	if (system_call(buffer, SH) != 0) {
	  dprint0(1, "Can't get uuname info - system call failed!\n");
	  unlink(filename);	/* insurance */
	  return(-1);
	}
	
	if ((fd = fopen(filename, "r")) == NULL) {
	  dprint1(1, "Can't get uuname info - can't open file %s for reading\n",
		   filename);
	  unlink(filename);	/* insurance */
	  return(-1);
	}
	
	previous_record = NULL;

	while (fgets(buffer, SLEN, fd) != NULL) {
	  no_ret(buffer);
	  if (previous_record == NULL) {
	    dprint1(2, "uuname\tdirect connection to %s, ", buffer);
	    loc_on_line = 30 + strlen(buffer);
	    previous_record = pmalloc(sizeof *talk_to_sys);

	    strcpy(previous_record->name, buffer);
	    previous_record->next = NULL;
	    talk_to_sys = previous_record;
	  }
	  else {	/* don't have to check uniqueness - uuname does that! */
	    if (loc_on_line + strlen(buffer) > 80) {
	      dprint0(2, "\n\t");
	      loc_on_line = 8;
	    }
	    dprint1(2, "%s, ", buffer);
	    loc_on_line += (strlen(buffer) + 2);
	    system_record = pmalloc(sizeof *talk_to_sys);
	  
	    strcpy(system_record->name, buffer);
	    system_record->next = NULL;
	    previous_record->next = system_record;
	    previous_record = system_record;
	  }
	}

	fclose(fd);

	(void) unlink(filename);		/* kill da temp file!! */

	dprint0(2, "\n");			/* for a nice format! Yeah! */

	return(0);				/* it all went okay... */
}
