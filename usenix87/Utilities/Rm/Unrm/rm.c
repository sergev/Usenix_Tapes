/**			rm.c			**/

/** This program replaces 'rm' (it's assumed that this program is BEFORE 
    /bin/rm in the path) and will ferret away copies of the files being 
    removed to the directory /tmp/rm/<login>.  Files that are copied into 
    /tmp/rm/* can be recalled by 'unrm filename'.  Every so often (probably 
    midnight every night) a daemon should clear out the old files in the
    /tmp/rm directory...

    (C) Copyright 1986, Dave Taylor, Hewlett-Packard Company
**/

#include <stdio.h>
#include <errno.h>

#define  real_rm	"/bin/rm"
#define  RM_DIR		"/tmp/rm"

#define  ACCESS_MODE	04 & 02
#define  DIR_ACCESS	04 & 01

#define  SLEN		80

#define  USERS_NAME	"LOGNAME"		/* "USER" in BSD */

extern int errno;

char *basename(), *getlogin(), *getenv();

main(argc, argv)
int argc;
char **argv;
{
	extern int optind;	/* for getopt */
	char   buffer[SLEN], login_name[SLEN], dirname[SLEN], *cp;
	int    c, oldumask; 

	while ((c = getopt(argc, argv, "rfi")) != EOF) {
	  switch (c) {
	    case 'r' : 
	    case 'f' : 
	    case 'i' : break;	/* just accept 'em... */
	    case '?' : exit(fprintf(stderr,"Usage: rm [-rfi] files\n"));
	  }
	}

	if (strlen(argv[optind]) == 0)
	  exit(0);

	/* is the top level /tmp directory available??? */

	if (access(RM_DIR, DIR_ACCESS)) {
	  sprintf(buffer,"mkdir %s; chmod 777 %s", RM_DIR, RM_DIR);
	  if (system(buffer) != 0) {
	    printf("'%s' failed!!\n", buffer);
	    exit(1);
	  }
	}

	/* now get the users login name... */

	if ((cp = getenv(USERS_NAME)) == NULL)
	  strcpy(login_name, getlogin());
	else
	  strcpy(login_name, cp);

	/* let's see if THAT directory is hangin' around... */

	sprintf(dirname, "%s/%s", RM_DIR, login_name);

	if (access(dirname, DIR_ACCESS)) {
	  sprintf(buffer,"mkdir %s; chmod 700 %s", dirname, dirname);
	  if (system(buffer) != 0) {
	    printf("'%s' failed!!\n", buffer);
	    exit(1);
	  }
	}

	oldumask = umask(077);
	while (strlen(argv[optind]) > 0) {
	  if (access(basename(argv[optind]), ACCESS_MODE) == 0)
	    save_copy_of(dirname, argv[optind]);
	  optind++;
	}
	(void) umask(oldumask);

	execv(real_rm, argv);
	
	fprintf(stderr,"rm: error %d exec'ing!\n", errno);
}

char *basename(string)
char *string;
{
	/** returns the basename of the file specified in string **/

	static   char *buff;

	buff = string + strlen(string); /* start at last char */
	
	while (*buff != '/' && buff > string) buff--;

	return( (char *) (*buff == '/'? ++buff : buff));
}

save_copy_of(dirname, filename)
char *dirname, *filename;
{
	/** Try to link filename to dirname, if that fails, copy it
	    bit by bit... **/

	char newfname[80];

	sprintf(newfname,"%s/%s", dirname, basename(filename));

	(void) unlink(newfname);	/* blow it away if already there! */

	if (link(filename, newfname) != 0) {
	  FILE *infile, *outfile;	
	  int   c;
	  
	  if ((infile = fopen(filename, "r")) == NULL)
	    exit(fprintf(stderr, "rm: can't read file '%s' to save a copy!\n", 
		 filename));

	  if ((outfile = fopen(newfname, "w")) == NULL)
	    exit(fprintf(stderr, "rm: can't write to file '%s'!\n",
		 newfname));

	  while ((c = getc(infile)) != EOF)
	    putc(c, outfile);
	  
	  fclose(infile);
	  fclose(outfile);
	}
}
