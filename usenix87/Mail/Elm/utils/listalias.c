/**			listalias.c			**/

/** Program that lists all the available aliases.  This one uses the pipe 
    command, feeding the stuff to egrep then sort, or just sort.

    (C) Copyright 1986, Dave Taylor
**/

#include <stdio.h>
#include <fcntl.h>

#include "defs.h"

#define alias_hash	".alias_hash"
#define alias_data	".alias_data"

#ifdef BSD
  FILE *popen();
#endif

char *getenv();

main(argc, argv)
int argc;
char *argv[];
{
	FILE *datafile, *fd_pipe;
	struct alias_rec hash_record;
	int hashfile, count = 0;
	char buffer[LONG_SLEN], fd_hash[SLEN], 
	     fd_data[SLEN], *home;

	if (argc > 2) {
	  printf("Usage: listalias <optional-regular-expression>\n");
	  exit(1);
	}

	home = getenv("HOME");

	sprintf(fd_hash, "%s/%s", home, alias_hash);
	sprintf(fd_data, "%s/%s", home, alias_data);

	if (argc > 1)
	  sprintf(buffer, "egrep \"%s\" | sort", argv[1]);
	else
	  sprintf(buffer, "sort");

	if ((fd_pipe = popen(buffer, "w")) == NULL) {
	  if (argc > 1) 
	    printf("cannot open pipe to egrep program for expressions!\n");
	  fd_pipe = stdout;
	}

	do {

	  if ((hashfile = open(fd_hash, O_RDONLY)) > 0) {
	    if ((datafile = fopen(fd_data, "r")) == NULL) {
	      fprintf("Opened %s hash file, but couldn't open data file!\n",
		       count? "system" : "user");
	      goto next_file;
	    }
	
	    /** Otherwise let us continue... **/

	    while (read(hashfile, &hash_record, sizeof (hash_record)) != 0) {
	      if (strlen(hash_record.name) > 0) {
	        fseek(datafile, hash_record.byte, 0L);
	        fgets(buffer, LONG_SLEN, datafile);
	        fprintf(fd_pipe, "%-15s  %s", hash_record.name, buffer);
	      }
	    }
	  }

next_file: strcpy(fd_hash, system_hash_file);
	   strcpy(fd_data, system_data_file);

	} while (++count < 2);

	pclose(fd_pipe);

	exit(0);
}
